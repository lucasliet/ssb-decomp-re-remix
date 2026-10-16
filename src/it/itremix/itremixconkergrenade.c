/**
 * itremixconkergrenade.c - Conker's grenade (vanilla item id 0x15), thrown by
 * ConkerDSP. Port of the grenade_* routines in Conker/ConkerSpecial.asm.
 *
 * Remix item fields that the vanilla ITStruct does not name (0x01C4 to 0x01D4,
 * 0x0354) live in ITRemixConkerGrenadeVars inside item_vars. The notes file lists
 * the vanilla patches, REMIX-TODOs and uncertain offsets.
 */

#ifdef PORT

#include "it/itremix/itremixconkergrenade.h"
#include <math.h>
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <gm/gmtypes.h>
#include <it/item.h>
#include "ft/ftremix/ftremixhooks.h"
#include <it/itfighter/itlinkbomb.h>
#include <it/itmain.h>
#include <it/itmanager.h>
#include <it/itmap.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <mp/mpprocess.h>
#include <ef/efmanager.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include "ft/ftremix/ftremix.h"

extern void *func_800269C0_275C0(u16 id);

#define ITREMIX_CONKER_GRENADE_KIND 0x15
#define ITREMIX_CONKER_GRENADE_ATTRIBUTES_OFFSET 0x40
#define ITREMIX_CONKER_GRENADE_ATTACK_EVENTS_OFFSET 0x88
#define ITREMIX_CONKER_GRENADE_RENDER_KIND 0x2E

#define ITREMIX_CONKER_GRENADE_DURATION 150
#define ITREMIX_CONKER_GRENADE_GRAVITY 1.5F
#define ITREMIX_CONKER_GRENADE_MAX_SPEED 60.0F
#define ITREMIX_CONKER_GRENADE_BOUNCE 0.55F
#define ITREMIX_CONKER_GRENADE_ANGLE 0.85F
#define ITREMIX_CONKER_GRENADE_ROTATION 0.003F
#define ITREMIX_CONKER_GRENADE_ROTATION_BASE 0.017578125F
#define ITREMIX_CONKER_GRENADE_MAX_SPEED_DECAY 1.0F
#define ITREMIX_CONKER_GRENADE_GROUND_DRAG 0.9375F
#define ITREMIX_CONKER_GRENADE_GROUND_STOP_SPEED 2.0F
#define ITREMIX_CONKER_GRENADE_RESTING_Y_SPEED 5.0F
#define ITREMIX_CONKER_GRENADE_REFRESH_SPEED 35.0F
#define ITREMIX_CONKER_GRENADE_REFRESH_TIME 16
#define ITREMIX_CONKER_GRENADE_DAMAGE_SPEED_SCALE 0.0703125F
#define ITREMIX_CONKER_GRENADE_HIT_KNOCKBACK_SCALE 4.0F
#define ITREMIX_CONKER_GRENADE_HIT_KNOCKBACK_BASE 10.0F
#define ITREMIX_CONKER_GRENADE_HIT_MIN_DURATION 20
#define ITREMIX_CONKER_GRENADE_HURT_MIN_DURATION 12
#define ITREMIX_CONKER_GRENADE_HURT_SPEED_SCALE 0.5F
#define ITREMIX_CONKER_GRENADE_HURT_SPEED_BASE 5.0F
#define ITREMIX_CONKER_GRENADE_SMOKE_DURATION 50
#define ITREMIX_CONKER_GRENADE_SMOKE_INTERVAL 8
#define ITREMIX_CONKER_GRENADE_EXPLOSION_END_FRAME 6
#define ITREMIX_CONKER_GRENADE_EXPLOSION_KNOCKBACK_SCALE 70
#define ITREMIX_CONKER_GRENADE_EXPLOSION_KNOCKBACK_BASE 50
#define ITREMIX_CONKER_GRENADE_EXPLOSION_FGM 1
#define ITREMIX_CONKER_GRENADE_EXPLOSION_EVENT_LAST 3
#define ITREMIX_CONKER_GRENADE_EXPLOSION_EVENT_COUNT 4
#define ITREMIX_CONKER_GRENADE_COLLISION_MASK 0x0C21
#define ITREMIX_CONKER_GRENADE_NO_COMBO_PORT 4
#define ITREMIX_CONKER_GRENADE_HITBOX_ON 1
#define ITREMIX_CONKER_GRENADE_HITBOX_OFF 0
#define ITREMIX_CONKER_GRENADE_HURTBOX_NORMAL 1
#define ITREMIX_CONKER_GRENADE_HURTBOX_OFF 0
#define ITREMIX_CONKER_GRENADE_HIT_ELEMENT_FIRE nGMHitElementFire

enum itRemixConkerGrenadeStatus
{
    nITRemixConkerGrenadeStatusAerial,
    nITRemixConkerGrenadeStatusResting,
    nITRemixConkerGrenadeStatusExplosion,
    nITRemixConkerGrenadeStatusEnumCount
};

/* Remix item fields beyond the vanilla ITStruct union. Offsets are the Remix asm ones. */
typedef struct ITRemixConkerGrenadeVars
{
    void **p_file;                  /* Conker file 7 (ftRemixPowerData(fp)->p_file_special2); explosion attack events are read from it */
    FTStruct *fp;                   /* 0x01C4: owner fighter struct; its active grenade slot is cleared when the grenade ends */
    s32 unk_0x354;                  /* 0x0354: written as 4 at spawn; meaning unknown */
    f32 max_speed;                  /* 0x01C8: speed cap; set to the knockback when hit */
    f32 rotation_direction;         /* 0x01CC */
    s32 hitbox_refresh_timer;       /* 0x01D0 */
    sb32 is_hit_collide;            /* 0x01D4 */
} ITRemixConkerGrenadeVars;

_Static_assert(sizeof(ITRemixConkerGrenadeVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixConkerGrenadeVars must fit in ITStruct.item_vars");

static sb32 itRemixConkerGrenadeProcUpdate(GObj *item_gobj);
static sb32 itRemixConkerGrenadeProcMap(GObj *item_gobj);
static sb32 itRemixConkerGrenadeProcMapResting(GObj *item_gobj);
static sb32 itRemixConkerGrenadeProcExploding(GObj *item_gobj);
static sb32 itRemixConkerGrenadeProcHurtbox(GObj *item_gobj);
static sb32 itRemixConkerGrenadeProcDamage(GObj *item_gobj);
static sb32 itRemixConkerGrenadeProcDead(GObj *item_gobj);
static sb32 itRemixConkerGrenadeDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags);
static void itRemixConkerGrenadeExplosionAttackEvent(GObj *item_gobj);
static void itRemixConkerGrenadeSetAir(GObj *item_gobj);
static void itRemixConkerGrenadeSetResting(GObj *item_gobj);
static void itRemixConkerGrenadeExplode(GObj *item_gobj);
static void itRemixConkerGrenadeSetExplosion(GObj *item_gobj);

static ITStatusDesc sITRemixConkerGrenadeStatusDescs[nITRemixConkerGrenadeStatusEnumCount] =
{
    /* Aerial: grenade_main_, grenade_collision_, grenade_hurtbox_collision_ x2, hop, no setoff, reflector, hitbox */
    {
        itRemixConkerGrenadeProcUpdate,
        itRemixConkerGrenadeProcMap,
        itRemixConkerGrenadeProcHurtbox,
        itRemixConkerGrenadeProcHurtbox,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixConkerGrenadeProcDamage
    },

    /* Resting: as aerial, except the collision is grenade_resting_collision_ */
    {
        itRemixConkerGrenadeProcUpdate,
        itRemixConkerGrenadeProcMapResting,
        itRemixConkerGrenadeProcHurtbox,
        itRemixConkerGrenadeProcHurtbox,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixConkerGrenadeProcDamage
    },

    /* Exploding: grenade_exploding_main_ only */
    {
        itRemixConkerGrenadeProcExploding,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/* Same as the aerial status, so a freshly spawned grenade has the same procs the Remix item table gives it. */
static ITDesc sITRemixConkerGrenadeItemDesc =
{
    ITREMIX_CONKER_GRENADE_KIND,
    NULL,                                   /* p_file: set from the owner fighter in itRemixConkerGrenadeMakeItem */
    ITREMIX_CONKER_GRENADE_ATTRIBUTES_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixConkerGrenadeProcUpdate,
    itRemixConkerGrenadeProcMap,
    itRemixConkerGrenadeProcHurtbox,
    itRemixConkerGrenadeProcHurtbox,
    itMainCommonProcHop,
    NULL,
    itMainCommonProcReflector,
    itRemixConkerGrenadeProcDamage
};

/* Returns the Remix grenade fields overlaid on the item's union storage. */
static ITRemixConkerGrenadeVars *itRemixConkerGrenadeGetVars(ITStruct *ip)
{
    return (ITRemixConkerGrenadeVars*)&ip->item_vars;
}

/* Current speed magnitude; the asm sqrt(x*x + y*y). */
static f32 itRemixConkerGrenadeGetAbsSpeed(ITStruct *ip)
{
    f32 x = ip->physics.vel_air.x;
    f32 y = ip->physics.vel_air.y;

    return sqrtf((x * x) + (y * y));
}

/* Ground drag, and the hitbox is off once the grenade is nearly stopped. */
static void itRemixConkerGrenadeDragGround(ITStruct *ip)
{
    ip->physics.vel_air.x *= ITREMIX_CONKER_GRENADE_GROUND_DRAG;
    if (fabsf(ip->physics.vel_air.x) < ITREMIX_CONKER_GRENADE_GROUND_STOP_SPEED)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->attack_coll.attack_state = ITREMIX_CONKER_GRENADE_HITBOX_OFF;
    }
}

/* Air speed decays by one per frame down to MAX_SPEED, then gravity is applied. */
static void itRemixConkerGrenadeDecaySpeedAir(ITStruct *ip, ITRemixConkerGrenadeVars *vars)
{
    f32 decayed_max_speed = vars->max_speed - ITREMIX_CONKER_GRENADE_MAX_SPEED_DECAY;

    if (decayed_max_speed > ITREMIX_CONKER_GRENADE_MAX_SPEED)
    {
        vars->max_speed = decayed_max_speed;
    }
    else
    {
        vars->max_speed = ITREMIX_CONKER_GRENADE_MAX_SPEED;
    }
    itMainApplyGravityClampTVel(ip, ITREMIX_CONKER_GRENADE_GRAVITY, vars->max_speed);
}

/* Facing sign of the x speed; the rotation direction keeps its last value while x is zero. */
static void itRemixConkerGrenadeUpdateRotationDirection(ITStruct *ip, ITRemixConkerGrenadeVars *vars)
{
    f32 x = ip->physics.vel_air.x;

    if (x == 0.0F)
    {
        return;
    }
    vars->rotation_direction = (x < 0.0F) ? -1.0F : 1.0F;
}

/* Spins the first joint around its x axis. */
static void itRemixConkerGrenadeUpdateRotation(GObj *item_gobj, f32 abs_speed, ITRemixConkerGrenadeVars *vars)
{
    DObj *joint = DObjGetStruct(item_gobj);
    f32 rotation_speed = (ITREMIX_CONKER_GRENADE_ROTATION_BASE + (ITREMIX_CONKER_GRENADE_ROTATION * abs_speed)) * vars->rotation_direction;

    if (abs_speed == 0.0F)
    {
        rotation_speed = 0.0F;
    }
    joint->rotate.vec.f.x -= rotation_speed;
}

/* Clears the four hit-object records so the grenade can hit the same targets again. */
static void itRemixConkerGrenadeResetHitRecords(ITStruct *ip)
{
    s32 i;

    for (i = 0; i < GMATTACKREC_NUM_MAX; i++)
    {
        ip->attack_coll.attack_records[i].victim_gobj = NULL;
    }
}

/* Counts the hit refresh timer; resets the records when it ends or when the grenade is fast. */
static void itRemixConkerGrenadeUpdateHitRecords(ITStruct *ip, ITRemixConkerGrenadeVars *vars, f32 abs_speed)
{
    if (vars->hitbox_refresh_timer != 0)
    {
        vars->hitbox_refresh_timer--;
        if (vars->hitbox_refresh_timer == 0)
        {
            itRemixConkerGrenadeResetHitRecords(ip);
        }
        return;
    }
    if (abs_speed >= ITREMIX_CONKER_GRENADE_REFRESH_SPEED)
    {
        itRemixConkerGrenadeResetHitRecords(ip);
    }
}

/* Damage and base knockback scale with the current speed. */
static void itRemixConkerGrenadeUpdateDamage(ITStruct *ip, f32 abs_speed)
{
    s32 damage = (s32)(abs_speed * ITREMIX_CONKER_GRENADE_DAMAGE_SPEED_SCALE) + 1;

    ip->attack_coll.damage = damage;
    ip->attack_coll.knockback_base = (damage * 8) + ITREMIX_CONKER_GRENADE_HIT_KNOCKBACK_BASE;
}

/* Puffs smoke at the grenade's pin, rotated by its current angle. */
static void itRemixConkerGrenadeSpawnSmoke(GObj *item_gobj, ITStruct *ip)
{
    DObj *joint = DObjGetStruct(item_gobj);
    DObj *pin_joint = DObjGetChild(joint);
    f32 theta = -joint->rotate.vec.f.x;
    f32 cos_theta = __cosf(theta);
    f32 sin_theta = __sinf(theta);
    Vec3f smoke_pos;

    smoke_pos.x = joint->translate.vec.f.x + ((pin_joint->translate.vec.f.x * cos_theta) + (pin_joint->translate.vec.f.y * sin_theta));
    smoke_pos.y = joint->translate.vec.f.y + ((pin_joint->translate.vec.f.y * cos_theta) - (pin_joint->translate.vec.f.x * sin_theta));
    smoke_pos.z = joint->translate.vec.f.z;
    efManagerFlameLRMakeEffect(&smoke_pos, ip->lr);
}

/* grenade_main_ */
static sb32 itRemixConkerGrenadeProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixConkerGrenadeVars *vars = itRemixConkerGrenadeGetVars(ip);
    f32 abs_speed;

    if (ip->ga == nMPKineticsGround)
    {
        itRemixConkerGrenadeDragGround(ip);
    }
    else
    {
        itRemixConkerGrenadeDecaySpeedAir(ip, vars);
    }

    if (ip->lifetime == 0)
    {
        itRemixConkerGrenadeExplode(item_gobj);
    }
    else
    {
        ip->lifetime--;
        abs_speed = itRemixConkerGrenadeGetAbsSpeed(ip);
        itRemixConkerGrenadeUpdateRotationDirection(ip, vars);
        itRemixConkerGrenadeUpdateRotation(item_gobj, abs_speed, vars);
        itRemixConkerGrenadeUpdateHitRecords(ip, vars, abs_speed);
        itRemixConkerGrenadeUpdateDamage(ip, abs_speed);
        if ((ip->lifetime < ITREMIX_CONKER_GRENADE_SMOKE_DURATION) && ((ip->lifetime % ITREMIX_CONKER_GRENADE_SMOKE_INTERVAL) == 0))
        {
            itRemixConkerGrenadeSpawnSmoke(item_gobj, ip);
        }
    }

    vars->is_hit_collide = FALSE;
    return FALSE;
}

/* grenade_collision_ */
static sb32 itRemixConkerGrenadeProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    sb32 is_collided;

    is_collided = mpProcessUpdateMain(&ip->coll_data, itRemixConkerGrenadeDetectCollision, item_gobj, ITREMIX_CONKER_GRENADE_COLLISION_MASK);
    itMapCheckCollideAllRebound(item_gobj, ITREMIX_CONKER_GRENADE_COLLISION_MASK, ITREMIX_CONKER_GRENADE_BOUNCE, NULL);

    if (!is_collided)
    {
        return FALSE;
    }
    if (!(ip->coll_data.mask_stat & MAP_FLAG_FLOOR))
    {
        return FALSE;
    }
    if (fabsf(ip->physics.vel_air.y) < ITREMIX_CONKER_GRENADE_RESTING_Y_SPEED)
    {
        itRemixConkerGrenadeSetResting(item_gobj);
    }
    return FALSE;
}

/* grenade_resting_collision_ */
static sb32 itRemixConkerGrenadeProcMapResting(GObj *item_gobj)
{
    itMapCheckLRWallProcNoFloor(item_gobj, itRemixConkerGrenadeSetAir);
    return FALSE;
}

/* grenade_exploding_main_ */
static sb32 itRemixConkerGrenadeProcExploding(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixConkerGrenadeExplosionAttackEvent(item_gobj);
    ip->multi++;
    if (ip->multi != ITREMIX_CONKER_GRENADE_EXPLOSION_END_FRAME)
    {
        return FALSE;
    }
    ftRemixConkerDSPSetActiveGrenade(itRemixConkerGrenadeGetVars(ip)->fp, NULL);
    return TRUE;
}

/* grenade_hurtbox_collision_ */
static sb32 itRemixConkerGrenadeProcHurtbox(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixConkerGrenadeVars *vars = itRemixConkerGrenadeGetVars(ip);
    f32 abs_speed;
    s32 whole_speed;
    s32 duration_loss;

    itRemixConkerGrenadeSetAir(item_gobj);

    abs_speed = itRemixConkerGrenadeGetAbsSpeed(ip);
    if (!vars->is_hit_collide)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->physics.vel_air.y = (abs_speed * ITREMIX_CONKER_GRENADE_HURT_SPEED_SCALE) + ITREMIX_CONKER_GRENADE_HURT_SPEED_BASE;
    }
    vars->is_hit_collide = FALSE;
    vars->hitbox_refresh_timer = 0;

    whole_speed = (s32)abs_speed;
    duration_loss = whole_speed + (whole_speed >> 1);
    ip->lifetime -= duration_loss;
    if (ip->lifetime < ITREMIX_CONKER_GRENADE_HURT_MIN_DURATION)
    {
        ip->lifetime = ITREMIX_CONKER_GRENADE_HURT_MIN_DURATION;
    }
    return FALSE;
}

/* grenade_hitbox_collision_ */
static sb32 itRemixConkerGrenadeProcDamage(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixConkerGrenadeVars *vars = itRemixConkerGrenadeGetVars(ip);
    GObj *attacker_gobj = ip->damage_gobj;
    s32 damage = ip->damage_queue;
    f32 knockback;
    f32 knockback_angle;

    itRemixConkerGrenadeSetAir(item_gobj);

    ip->owner_gobj = attacker_gobj;
    ip->player = ITREMIX_CONKER_GRENADE_NO_COMBO_PORT;
    if ((attacker_gobj != NULL) && (attacker_gobj->id == nGCCommonKindFighter))
    {
        ip->player = ftGetStruct(attacker_gobj)->player;
    }

    knockback = (damage * ITREMIX_CONKER_GRENADE_HIT_KNOCKBACK_SCALE) + ITREMIX_CONKER_GRENADE_HIT_KNOCKBACK_BASE;
    vars->max_speed = knockback;

    knockback_angle = ftCommonDamageGetKnockbackAngle(ip->damage_angle, ip->ga, knockback);
    ip->lr = -ip->damage_lr;
    ip->physics.vel_air.x = (knockback * __cosf(knockback_angle)) * (f32)ip->lr;
    ip->physics.vel_air.y = knockback * __sinf(knockback_angle);

    vars->hitbox_refresh_timer = ITREMIX_CONKER_GRENADE_REFRESH_TIME;
    vars->is_hit_collide = TRUE;

    ip->lifetime -= damage * 2;
    if (ip->lifetime < ITREMIX_CONKER_GRENADE_HIT_MIN_DURATION)
    {
        ip->lifetime = ITREMIX_CONKER_GRENADE_HIT_MIN_DURATION;
    }
    return FALSE;
}

/* grenade_blast_zone_ */
static sb32 itRemixConkerGrenadeProcDead(GObj *item_gobj)
{
    ftRemixConkerDSPSetActiveGrenade(itRemixConkerGrenadeGetVars(itGetStruct(item_gobj))->fp, NULL);
    /* Remix stores this in the blast wall slot (ITStruct 0x398), which runs before the vanilla destroy. */
    return TRUE;
}

/* grenade_detect_collision_: the vanilla itMapProcAllCheckCollisionFlag without its floor-edge and FGM block. */
static sb32 itRemixConkerGrenadeDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags)
{
    if (mpProcessCheckTestLWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunLWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestRWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunRWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestCeilCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunCeilCollisionAdjNew(coll_data);
        if (coll_data->mask_stat & MAP_FLAG_CEIL)
        {
            mpProcessRunCeilEdgeAdjust(coll_data);
        }
    }
    if (mpProcessRunFloorCollisionAdjNewNULL(coll_data) != FALSE)
    {
        mpProcessSetLandingFloor(coll_data);
    }
    return (coll_data->mask_curr & coll_flags) != 0;
}

/* grenade_explosion_hitboxes_: vanilla itLinkBombExplodeUpdateAttackEvent, reading the grenade's file. */
static void itRemixConkerGrenadeExplosionAttackEvent(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixConkerGrenadeVars *vars = itRemixConkerGrenadeGetVars(ip);
    ITAttackEvent *events = (ITAttackEvent*)((uintptr_t)*vars->p_file + ITREMIX_CONKER_GRENADE_ATTACK_EVENTS_OFFSET);
    ITAttackEvent *event = &events[ip->event_id];

    if (ip->multi != event->timer)
    {
        return;
    }

    ip->attack_coll.angle = BITFIELD_SEXT10(event->angle);
    ip->attack_coll.damage = event->damage;
    ip->attack_coll.size = event->size;

    ip->attack_coll.can_rehit_item = TRUE;
    ip->attack_coll.can_hop = FALSE;
    ip->attack_coll.can_reflect = FALSE;
    ip->attack_coll.can_setoff = FALSE;
    ip->attack_coll.element = ITREMIX_CONKER_GRENADE_HIT_ELEMENT_FIRE;

    ip->event_id++;
    if (ip->event_id == ITREMIX_CONKER_GRENADE_EXPLOSION_EVENT_COUNT)
    {
        ip->event_id = ITREMIX_CONKER_GRENADE_EXPLOSION_EVENT_LAST;
    }
}

/* grenade_begin_main_ */
static void itRemixConkerGrenadeSetAir(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMapSetAir(ip);
    ip->attack_coll.attack_state = ITREMIX_CONKER_GRENADE_HITBOX_ON;
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixConkerGrenadeStatusDescs, nITRemixConkerGrenadeStatusAerial);
}

/* grenade_begin_resting_ */
static void itRemixConkerGrenadeSetResting(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->times_landed = 0;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    itMapSetGround(ip);
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixConkerGrenadeStatusDescs, nITRemixConkerGrenadeStatusResting);
}

/* grenade_explosion_ */
static void itRemixConkerGrenadeExplode(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    ip->damage_coll.hitstatus = ITREMIX_CONKER_GRENADE_HURTBOX_OFF;
    itMainClearOwnerStats(item_gobj);

    efManagerSparkleWhiteMultiExplodeMakeEffect(&joint->translate.vec.f);
    efManagerQuakeMakeEffect(1);
    /* REMIX-TODO(DObj): asm 0x0054 of the first joint is set to 2 here; the DObj field is not named yet. */

    ip->attack_coll.fgm_id = ITREMIX_CONKER_GRENADE_EXPLOSION_FGM;
    itMainRefreshAttackColl(item_gobj);
    itRemixConkerGrenadeSetExplosion(item_gobj);
    func_800269C0_275C0(ITREMIX_CONKER_GRENADE_EXPLOSION_FGM);
}

/* grenade_begin_explosion_ */
static void itRemixConkerGrenadeSetExplosion(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->event_id = 0;
    ip->multi = 0;
    ip->attack_coll.throw_mul = 1.0F;
    ip->attack_coll.knockback_scale = ITREMIX_CONKER_GRENADE_EXPLOSION_KNOCKBACK_SCALE;
    ip->attack_coll.knockback_base = ITREMIX_CONKER_GRENADE_EXPLOSION_KNOCKBACK_BASE;
    itRemixConkerGrenadeExplosionAttackEvent(item_gobj);
    itMainSetStatus(item_gobj, sITRemixConkerGrenadeStatusDescs, nITRemixConkerGrenadeStatusExplosion);
}

/* Sets the joints' render routines, as the vanilla item does for its DObjs. */
static void itRemixConkerGrenadeSetupRender(GObj *item_gobj)
{
    DObj *joint = DObjGetStruct(item_gobj);

    gcAddXObjForDObjFixed(joint, ITREMIX_CONKER_GRENADE_RENDER_KIND, 0);
    gcAddXObjForDObjFixed(DObjGetChild(joint), ITREMIX_CONKER_GRENADE_RENDER_KIND, 0);
}

/* The launch velocity is power along the throw angle, mirrored by the fighter's facing. */
static void itRemixConkerGrenadeSetupLaunch(ITStruct *ip, FTStruct *fp, s32 power)
{
    f32 speed = (f32)power;
    f32 direction = (f32)fp->lr;
    f32 cos_angle = __cosf(ITREMIX_CONKER_GRENADE_ANGLE);
    f32 sin_angle = __sinf(ITREMIX_CONKER_GRENADE_ANGLE);

    ip->physics.vel_air.x = (speed * cos_angle) * direction;
    ip->physics.vel_air.y = speed * sin_angle;
    ip->physics.vel_air.z = 0.0F;
}

/* Fills the grenade's fields from its owner and enables its hitbox and hurtbox. */
static void itRemixConkerGrenadeSetupState(ITStruct *ip, GObj *parent_gobj, FTStruct *fp)
{
    ITRemixConkerGrenadeVars *vars = itRemixConkerGrenadeGetVars(ip);

    ip->owner_gobj = parent_gobj;
    ip->team = fp->team;
    ip->player = fp->player;
    vars->fp = fp;
    vars->p_file = ftRemixPowerData(fp)->p_file_special2;

    ip->attack_coll.can_rehit_shield = TRUE;
    ip->attack_coll.attack_state = ITREMIX_CONKER_GRENADE_HITBOX_ON;
    ip->damage_coll.hitstatus = ITREMIX_CONKER_GRENADE_HURTBOX_NORMAL;
    ip->is_damage_all = TRUE;
    ip->lifetime = ITREMIX_CONKER_GRENADE_DURATION;
    vars->unk_0x354 = 4;

    vars->max_speed = ITREMIX_CONKER_GRENADE_MAX_SPEED;
    vars->rotation_direction = 0.0F;
    vars->hitbox_refresh_timer = 0;
    vars->is_hit_collide = FALSE;
    ip->proc_dead = itRemixConkerGrenadeProcDead;
    ip->coll_data.cliff_id = 0;
}

/**
 * Spawns a grenade thrown by parent_gobj, at pos, with the fighter's grenade power.
 * Returns NULL when the item could not be created.
 */
GObj *itRemixConkerGrenadeMakeItem(GObj *parent_gobj, Vec3f *pos, s32 power)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc item_desc = sITRemixConkerGrenadeItemDesc;
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *item_gobj;

    item_desc.p_file = ftRemixPowerData(fp)->p_file_special2;
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, &vel, FALSE);
    if (item_gobj == NULL)
    {
        return NULL;
    }

    itRemixConkerGrenadeSetupRender(item_gobj);
    itRemixConkerGrenadeSetupLaunch(itGetStruct(item_gobj), fp, power);
    itRemixConkerGrenadeSetupState(itGetStruct(item_gobj), parent_gobj, fp);
    ftRemixConkerDSPSetActiveGrenade(fp, item_gobj);
    mpCommonRunItemCollisionDefault(item_gobj, fp->coll_data.p_translate, &fp->coll_data);
    return item_gobj;
}

#endif
