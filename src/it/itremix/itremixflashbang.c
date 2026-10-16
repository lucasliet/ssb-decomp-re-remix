/**
 * itremixflashbang.c - Peppy's Flashbang (Remix item id 0x4A), tossed by PeppyDSP.
 * Port of Item/Flashbang.asm (flashbang_* routines and item_info_array).
 *
 * Remix fields that the vanilla ITStruct does not name (0x01C4 to 0x01D4, 0x0354)
 * live in ITRemixFlashbangVars inside item_vars. The notes file lists the REMIX-TODO
 * markers, the vanilla patches the port replaces and the uncertain offsets.
 */

#ifdef PORT

#include "it/itremix/itremixflashbang.h"
#include "it/itremix/itremix.h"
#include "it/itremix/itremixscreenflash.h"
#include <math.h>
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftremix/ftremixfields.h>
#include <ef/efmanager.h>
#include <gm/gmtypes.h>
#include <it/item.h>
#include <it/itfighter/itlinkbomb.h>
#include <it/itmain.h>
#include <it/itmanager.h>
#include <it/itmap.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <mp/mpprocess.h>
#include <sys/utils.h>
#include "ft/ftremix/ftremix.h"

extern void *func_800269C0_275C0(u16 id);

#define ITREMIX_FLASHBANG_KIND 0x4A
#define ITREMIX_FLASHBANG_ATTRIBUTES_OFFSET 0x40
#define ITREMIX_FLASHBANG_ATTACK_EVENTS_OFFSET 0x88
#define ITREMIX_FLASHBANG_RENDER_KIND 0x2E
#define ITREMIX_FLASHBANG_OWNER_POWER_N64 0x0B20
#define ITREMIX_FLASHBANG_FGM 0x33
#define ITREMIX_FLASHBANG_EXPLOSION_FGM 0x2B7
#define ITREMIX_FLASHBANG_EXPLOSION_SETUP_FGM 1
#define ITREMIX_FLASHBANG_DAMAGE_TYPE 0x0C /* Damage.id.FLASHBANG: sixth add_damage_type in Damage.asm, after SHADOW..BURY */

#define ITREMIX_FLASHBANG_DURATION 250
#define ITREMIX_FLASHBANG_GRAVITY 2.3F
#define ITREMIX_FLASHBANG_MAX_SPEED 50.0F
#define ITREMIX_FLASHBANG_BOUNCE 0.5F
#define ITREMIX_FLASHBANG_ANGLE 0.40F
#define ITREMIX_FLASHBANG_ROTATION 0.003F
#define ITREMIX_FLASHBANG_ROTATION_BASE 0.017578125F /* 0x3C900000 */
#define ITREMIX_FLASHBANG_MAX_SPEED_DECAY 1.0F
#define ITREMIX_FLASHBANG_GROUND_DRAG 0.9375F /* 0x3F700000; the asm comment says 0.875 */
#define ITREMIX_FLASHBANG_GROUND_STOP_SPEED 2.0F
#define ITREMIX_FLASHBANG_RESTING_Y_SPEED 5.0F
#define ITREMIX_FLASHBANG_REFRESH_SPEED 35.0F
#define ITREMIX_FLASHBANG_REFRESH_TIME 16
#define ITREMIX_FLASHBANG_SMOKE_DURATION 50
#define ITREMIX_FLASHBANG_SMOKE_INTERVAL 8
#define ITREMIX_FLASHBANG_EXPLOSION_END_FRAME 1
#define ITREMIX_FLASHBANG_EXPLOSION_EVENT_LAST 3
#define ITREMIX_FLASHBANG_EXPLOSION_EVENT_COUNT 4
#define ITREMIX_FLASHBANG_EXPLODE_DAMAGE 7
#define ITREMIX_FLASHBANG_EXPLODE_SIZE 350.0F /* 0x43AF0000 */
#define ITREMIX_FLASHBANG_EXPLODE_KNOCKBACK_SCALE 70
#define ITREMIX_FLASHBANG_EXPLODE_KNOCKBACK_BASE 50
#define ITREMIX_FLASHBANG_INITIAL_DAMAGE 1
#define ITREMIX_FLASHBANG_INITIAL_KNOCKBACK_BASE 1
#define ITREMIX_FLASHBANG_DAMAGE_FUSE_TIME 4
#define ITREMIX_FLASHBANG_HURT_MIN_DURATION 12
#define ITREMIX_FLASHBANG_HURT_SPEED_SCALE 0.5F
#define ITREMIX_FLASHBANG_HURT_SPEED_BASE 5.0F
#define ITREMIX_FLASHBANG_HIT_KNOCKBACK_SCALE 4.0F
#define ITREMIX_FLASHBANG_HIT_KNOCKBACK_BASE 10.0F
#define ITREMIX_FLASHBANG_COLLISION_MASK 0x0C21
#define ITREMIX_FLASHBANG_HITBOX_ON 1
#define ITREMIX_FLASHBANG_NO_COMBO_PORT 4

enum itRemixFlashbangStatus
{
    nITRemixFlashbangStatusAerial,
    nITRemixFlashbangStatusResting,
    nITRemixFlashbangStatusExplosion,
    nITRemixFlashbangStatusCount
};

/* Remix flashbang fields beyond the vanilla ITStruct union. Offsets are the Remix asm ones. */
typedef struct ITRemixFlashbangVars
{
    FTStruct *fp;                   /* 0x01C4: owner fighter struct */
    void **p_file;                  /* owner file 6 (p_file_special1): explosion attack events are read from it */
    s32 unk_0x354;                  /* 0x0354: written as 4 at spawn; meaning unknown */
    f32 max_speed;                  /* 0x01C8: speed cap; set to the knockback when hit */
    f32 rotation_direction;         /* 0x01CC */
    s32 hitbox_refresh_timer;       /* 0x01D0 */
    sb32 is_hit_collide;            /* 0x01D4 */
} ITRemixFlashbangVars;

_Static_assert(sizeof(ITRemixFlashbangVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixFlashbangVars must fit in ITStruct.item_vars");

static sb32 itRemixFlashbangProcUpdate(GObj *item_gobj);
static sb32 itRemixFlashbangProcMap(GObj *item_gobj);
static sb32 itRemixFlashbangProcMapResting(GObj *item_gobj);
static sb32 itRemixFlashbangProcExploding(GObj *item_gobj);
static sb32 itRemixFlashbangProcHurtbox(GObj *item_gobj);
static sb32 itRemixFlashbangProcDamage(GObj *item_gobj);
static sb32 itRemixFlashbangProcDead(GObj *item_gobj);
static sb32 itRemixFlashbangDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags);
static void itRemixFlashbangExplosionAttackEvent(GObj *item_gobj);
static void itRemixFlashbangSetAir(GObj *item_gobj);
static void itRemixFlashbangSetResting(GObj *item_gobj);
static void itRemixFlashbangExplode(GObj *item_gobj);
static void itRemixFlashbangSetExplosion(GObj *item_gobj);
static void itRemixFlashbangSetActive(FTStruct *fp, GObj *item_gobj);

static ITStatusDesc sITRemixFlashbangStatusDescs[nITRemixFlashbangStatusCount] =
{
    /* Aerial: flashbang_main_, flashbang_collision_, hurtbox and hitbox collisions, hop, no setoff, reflector, hitbox */
    {
        itRemixFlashbangProcUpdate,
        itRemixFlashbangProcMap,
        itRemixFlashbangProcHurtbox,
        itRemixFlashbangProcHurtbox,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixFlashbangProcDamage
    },

    /* Resting: as aerial, except the collision is flashbang_resting_collision_ */
    {
        itRemixFlashbangProcUpdate,
        itRemixFlashbangProcMapResting,
        itRemixFlashbangProcHurtbox,
        itRemixFlashbangProcHurtbox,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixFlashbangProcDamage
    },

    /* Exploding: flashbang_exploding_main_ only */
    {
        itRemixFlashbangProcExploding,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/* Same as the aerial status, so a freshly spawned flashbang has the procs the Remix item_info_array gives it. */
static ITDesc sITRemixFlashbangItemDesc =
{
    ITREMIX_FLASHBANG_KIND,
    NULL,                                   /* p_file: set from the owner fighter in itRemixFlashbangMakeItem */
    ITREMIX_FLASHBANG_ATTRIBUTES_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixFlashbangProcUpdate,
    itRemixFlashbangProcMap,
    itRemixFlashbangProcHurtbox,
    itRemixFlashbangProcHurtbox,
    itMainCommonProcHop,
    NULL,
    itMainCommonProcReflector,
    itRemixFlashbangProcDamage
};

/* Returns the Remix flashbang fields overlaid on the item's union storage. */
static ITRemixFlashbangVars *itRemixFlashbangGetVars(ITStruct *ip)
{
    return (ITRemixFlashbangVars*)&ip->item_vars;
}

/* Current speed magnitude; the asm sqrt(x*x + y*y). */
static f32 itRemixFlashbangGetAbsSpeed(ITStruct *ip)
{
    f32 x = ip->physics.vel_air.x;
    f32 y = ip->physics.vel_air.y;

    return sqrtf((x * x) + (y * y));
}

/* Ground drag; the hitbox turns off once the flashbang has nearly stopped. */
static void itRemixFlashbangDragGround(ITStruct *ip)
{
    ip->physics.vel_air.x *= ITREMIX_FLASHBANG_GROUND_DRAG;
    if (fabsf(ip->physics.vel_air.x) < ITREMIX_FLASHBANG_GROUND_STOP_SPEED)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->attack_coll.attack_state = nGMAttackStateOff;
    }
}

/* Air max speed decays by one per frame down to MAX_SPEED, then gravity is applied. */
static void itRemixFlashbangDecaySpeedAir(ITStruct *ip, ITRemixFlashbangVars *vars)
{
    f32 decayed_max_speed = vars->max_speed - ITREMIX_FLASHBANG_MAX_SPEED_DECAY;

    if (decayed_max_speed > ITREMIX_FLASHBANG_MAX_SPEED)
    {
        vars->max_speed = decayed_max_speed;
    }
    else
    {
        vars->max_speed = ITREMIX_FLASHBANG_MAX_SPEED;
    }
    itMainApplyGravityClampTVel(ip, ITREMIX_FLASHBANG_GRAVITY, vars->max_speed);
}

/* Facing sign of the x speed; the rotation direction keeps its last value while x is zero. */
static void itRemixFlashbangUpdateRotationDirection(ITStruct *ip, ITRemixFlashbangVars *vars)
{
    f32 x = ip->physics.vel_air.x;

    if (x == 0.0F)
    {
        return;
    }
    vars->rotation_direction = (x < 0.0F) ? -1.0F : 1.0F;
}

/* Spins the first joint around its x axis. The spin stops when the flashbang has no speed. */
static void itRemixFlashbangUpdateRotation(GObj *item_gobj, f32 abs_speed, ITRemixFlashbangVars *vars)
{
    DObj *joint = DObjGetStruct(item_gobj);
    f32 rotation_speed = (ITREMIX_FLASHBANG_ROTATION_BASE + (ITREMIX_FLASHBANG_ROTATION * abs_speed)) * vars->rotation_direction;

    if (abs_speed == 0.0F)
    {
        rotation_speed = 0.0F;
    }
    joint->rotate.vec.f.x -= rotation_speed;
}

/* Clears the hit records so the flashbang can hit the same targets again. */
static void itRemixFlashbangResetHitRecords(ITStruct *ip)
{
    s32 i;

    for (i = 0; i < GMATTACKREC_NUM_MAX; i++)
    {
        ip->attack_coll.attack_records[i].victim_gobj = NULL;
    }
}

/* Counts the refresh timer down and clears the hit records when it ends, or on every frame while the flashbang is fast. */
static void itRemixFlashbangUpdateHitRecords(ITStruct *ip, ITRemixFlashbangVars *vars, f32 abs_speed)
{
    if (vars->hitbox_refresh_timer != 0)
    {
        vars->hitbox_refresh_timer--;
        if (vars->hitbox_refresh_timer != 0)
        {
            return;
        }
        itRemixFlashbangResetHitRecords(ip);
    }
    if (abs_speed >= ITREMIX_FLASHBANG_REFRESH_SPEED)
    {
        itRemixFlashbangResetHitRecords(ip);
    }
}

/* Puffs smoke at the flashbang's pin, rotated by the first joint's current angle. */
static void itRemixFlashbangSpawnSmoke(GObj *item_gobj)
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
    efManagerFlameRandgcMakeEffect(&smoke_pos);
}

/* flashbang_main_ */
static sb32 itRemixFlashbangProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixFlashbangVars *vars = itRemixFlashbangGetVars(ip);
    f32 abs_speed;

    if (ip->ga == nMPKineticsGround)
    {
        itRemixFlashbangDragGround(ip);
    }
    else
    {
        itRemixFlashbangDecaySpeedAir(ip, vars);
    }

    if (ip->lifetime == 0)
    {
        itRemixFlashbangExplode(item_gobj);
    }
    else
    {
        ip->lifetime--;
        abs_speed = itRemixFlashbangGetAbsSpeed(ip);
        itRemixFlashbangUpdateRotationDirection(ip, vars);
        itRemixFlashbangUpdateRotation(item_gobj, abs_speed, vars);
        itRemixFlashbangUpdateHitRecords(ip, vars, abs_speed);
        if ((ip->lifetime < ITREMIX_FLASHBANG_SMOKE_DURATION) && ((ip->lifetime % ITREMIX_FLASHBANG_SMOKE_INTERVAL) == 0))
        {
            itRemixFlashbangSpawnSmoke(item_gobj);
        }
    }

    vars->is_hit_collide = FALSE;
    return FALSE;
}

/* flashbang_collision_: map collision, bounce, and resting on the floor once it is slow. */
static sb32 itRemixFlashbangProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    sb32 is_collided;

    is_collided = mpProcessUpdateMain(&ip->coll_data, itRemixFlashbangDetectCollision, item_gobj, ITREMIX_FLASHBANG_COLLISION_MASK);
    itMapCheckCollideAllRebound(item_gobj, ITREMIX_FLASHBANG_COLLISION_MASK, ITREMIX_FLASHBANG_BOUNCE, NULL);

    if (!is_collided)
    {
        return FALSE;
    }
    if (!(ip->coll_data.mask_stat & MAP_FLAG_FLOOR))
    {
        return FALSE;
    }
    if (fabsf(ip->physics.vel_air.y) < ITREMIX_FLASHBANG_RESTING_Y_SPEED)
    {
        itRemixFlashbangSetResting(item_gobj);
    }
    return FALSE;
}

/* flashbang_resting_collision_ */
static sb32 itRemixFlashbangProcMapResting(GObj *item_gobj)
{
    itMapCheckLRWallProcNoFloor(item_gobj, itRemixFlashbangSetAir);
    return FALSE;
}

/* flashbang_exploding_main_ */
static sb32 itRemixFlashbangProcExploding(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixFlashbangExplosionAttackEvent(item_gobj);
    ip->multi++;
    if (ip->multi != ITREMIX_FLASHBANG_EXPLOSION_END_FRAME)
    {
        return FALSE;
    }
    itRemixFlashbangSetActive(itRemixFlashbangGetVars(ip)->fp, NULL);
    return TRUE;
}

/* Shortens the fuse by about 1.5 frames per unit of speed, never below HURT_MIN_DURATION. */
static void itRemixFlashbangShortenFuseOnHurt(ITStruct *ip, f32 abs_speed)
{
    /* lrintf: the asm's cvt.w.s rounds to nearest under the default FCSR mode, not toward zero. */
    s32 whole_speed = lrintf(abs_speed);
    s32 duration_loss = whole_speed + (whole_speed >> 1);

    ip->lifetime -= duration_loss;
    if (ip->lifetime < ITREMIX_FLASHBANG_HURT_MIN_DURATION)
    {
        ip->lifetime = ITREMIX_FLASHBANG_HURT_MIN_DURATION;
    }
}

/* flashbang_hurtbox_collision_: a hurtbox hit returns the flashbang to the aerial state and bounces it up. */
static sb32 itRemixFlashbangProcHurtbox(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixFlashbangVars *vars = itRemixFlashbangGetVars(ip);
    f32 abs_speed;

    itRemixFlashbangSetAir(item_gobj);
    ip->attack_coll.attack_state = nGMAttackStateOff;

    abs_speed = itRemixFlashbangGetAbsSpeed(ip);
    if (!vars->is_hit_collide)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->physics.vel_air.y = (abs_speed * ITREMIX_FLASHBANG_HURT_SPEED_SCALE) + ITREMIX_FLASHBANG_HURT_SPEED_BASE;
    }
    vars->is_hit_collide = FALSE;
    vars->hitbox_refresh_timer = 0;
    itRemixFlashbangShortenFuseOnHurt(ip, abs_speed);
    return FALSE;
}

/* The attacker becomes the owner. Combo ownership uses its port, or NO_COMBO_PORT when it is not a fighter. */
static void itRemixFlashbangSetOwnerFromAttacker(ITStruct *ip)
{
    GObj *attacker_gobj = ip->damage_gobj;

    ip->owner_gobj = attacker_gobj;
    ip->player = ITREMIX_FLASHBANG_NO_COMBO_PORT;
    if ((attacker_gobj != NULL) && (attacker_gobj->id == nGCCommonKindFighter))
    {
        ip->player = ftGetStruct(attacker_gobj)->player;
    }
}

/* Knockback is damage * 4 + 10. The flashbang leaves along the knockback angle, mirrored away from the hit. */
static void itRemixFlashbangLaunchFromDamage(ITStruct *ip, ITRemixFlashbangVars *vars)
{
    f32 knockback = (ip->damage_queue * ITREMIX_FLASHBANG_HIT_KNOCKBACK_SCALE) + ITREMIX_FLASHBANG_HIT_KNOCKBACK_BASE;
    f32 knockback_angle;

    vars->max_speed = knockback;
    knockback_angle = ftCommonDamageGetKnockbackAngle(ip->damage_angle, ip->ga, knockback);
    ip->lr = -ip->damage_lr;
    ip->physics.vel_air.x = (knockback * __cosf(knockback_angle)) * (f32)ip->lr;
    ip->physics.vel_air.y = knockback * __sinf(knockback_angle);
}

/* flashbang_hitbox_collision_: a hitbox launches the flashbang and makes the attacker its owner. */
static sb32 itRemixFlashbangProcDamage(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixFlashbangVars *vars = itRemixFlashbangGetVars(ip);

    itRemixFlashbangSetAir(item_gobj);
    itRemixFlashbangSetOwnerFromAttacker(ip);
    itRemixFlashbangLaunchFromDamage(ip, vars);

    vars->hitbox_refresh_timer = ITREMIX_FLASHBANG_REFRESH_TIME;
    vars->is_hit_collide = TRUE;
    /* The asm keeps a fuse below DAMAGE_FUSE_TIME and otherwise sets it to DAMAGE_FUSE_TIME. */
    if (ip->lifetime >= ITREMIX_FLASHBANG_DAMAGE_FUSE_TIME)
    {
        ip->lifetime = ITREMIX_FLASHBANG_DAMAGE_FUSE_TIME;
    }
    return FALSE;
}

/* Blast zone (ITStruct proc_dead, Remix slot 0x0398): frees the owner's slot so another flashbang can be thrown. */
static sb32 itRemixFlashbangProcDead(GObj *item_gobj)
{
    itRemixFlashbangSetActive(itRemixFlashbangGetVars(itGetStruct(item_gobj))->fp, NULL);
    return TRUE;
}

/* flashbang_detect_collision_: vanilla itMapTestAllCollisionFlag (0x801737B8) without its floor-edge and FGM block. */
static sb32 itRemixFlashbangDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags)
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

/* Applies the explosion attack event whose timer matches the current frame, read from the owner's file (vanilla itLinkBombExplodeUpdateAttackEvent). */
static void itRemixFlashbangExplosionAttackEvent(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixFlashbangVars *vars = itRemixFlashbangGetVars(ip);
    ITAttackEvent *events = (ITAttackEvent*)((uintptr_t)*vars->p_file + ITREMIX_FLASHBANG_ATTACK_EVENTS_OFFSET);
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

    ip->event_id++;
    if (ip->event_id == ITREMIX_FLASHBANG_EXPLOSION_EVENT_COUNT)
    {
        ip->event_id = ITREMIX_FLASHBANG_EXPLOSION_EVENT_LAST;
    }
}

/* flashbang_begin_main_ */
static void itRemixFlashbangSetAir(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMapSetAir(ip);
    ip->attack_coll.attack_state = ITREMIX_FLASHBANG_HITBOX_ON;
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixFlashbangStatusDescs, nITRemixFlashbangStatusAerial);
}

/* flashbang_begin_resting_ */
static void itRemixFlashbangSetResting(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->times_landed = 0;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    itMapSetGround(ip);
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixFlashbangStatusDescs, nITRemixFlashbangStatusResting);
}

/* flashbang_explosion_: stops the flashbang, then starts the explosion state and its sound. */
static void itRemixFlashbangExplode(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    itMainClearOwnerStats(item_gobj);

    /* Flashbang.asm _check_flash: the screen flash is drawn in place of the explosion gfx. */
    itRemixScreenFlashFlashbang();
    efManagerQuakeMakeEffect(1);
    /* asm 0x0054 of the first joint (Flashbang.asm 618-624) is the DObj flags byte. */
    DObjGetStruct(item_gobj)->flags = 2;
    ip->attack_coll.fgm_id = ITREMIX_FLASHBANG_EXPLOSION_SETUP_FGM;
    itMainRefreshAttackColl(item_gobj);
    itRemixFlashbangSetExplosion(item_gobj);
    func_800269C0_275C0(ITREMIX_FLASHBANG_FGM);
    /* REMIX-TODO(GFX): asm calls Item.BlueShell.hit_gfx_ with the item object; the shared routine has no port yet. */
}

/* flashbang_begin_explosion_ */
static void itRemixFlashbangSetExplosion(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->event_id = 0;
    ip->multi = 0;
    ip->attack_coll.throw_mul = 1.0F;
    ip->attack_coll.knockback_scale = ITREMIX_FLASHBANG_EXPLODE_KNOCKBACK_SCALE;
    ip->attack_coll.knockback_base = ITREMIX_FLASHBANG_EXPLODE_KNOCKBACK_BASE;
    itRemixFlashbangExplosionAttackEvent(item_gobj);
    itMainSetStatus(item_gobj, sITRemixFlashbangStatusDescs, nITRemixFlashbangStatusExplosion);

    ip->attack_coll.fgm_id = ITREMIX_FLASHBANG_EXPLOSION_FGM;
    ip->attack_coll.element = ITREMIX_FLASHBANG_DAMAGE_TYPE;
    ip->attack_coll.size = ITREMIX_FLASHBANG_EXPLODE_SIZE;
    ip->attack_coll.damage = ITREMIX_FLASHBANG_EXPLODE_DAMAGE;
}

/* The owner's passive slot (N64 0x0ADC) holds the flashbang in flight. Peppy's DSP reads it, and it is NULL when none is in flight. */
static void itRemixFlashbangSetActive(FTStruct *fp, GObj *item_gobj)
{
    *(GObj**)&fp->passive_vars = item_gobj;
}

/* Sets the joints' render routines, as the vanilla item does for its DObjs. */
static void itRemixFlashbangSetupRender(GObj *item_gobj)
{
    DObj *joint = DObjGetStruct(item_gobj);

    gcAddXObjForDObjFixed(joint, ITREMIX_FLASHBANG_RENDER_KIND, 0);
    gcAddXObjForDObjFixed(DObjGetChild(joint), ITREMIX_FLASHBANG_RENDER_KIND, 0);
}

/* Fills the flashbang's fields from its owner and enables its hitbox and hurtbox. */
static void itRemixFlashbangSetupState(ITStruct *ip, GObj *parent_gobj, FTStruct *fp)
{
    ITRemixFlashbangVars *vars = itRemixFlashbangGetVars(ip);

    ip->owner_gobj = parent_gobj;
    ip->team = fp->team;
    ip->player = fp->player;
    /* asm 0x02CE bit 0x80 (stage_setting_): decomp names it is_allow_pickup by the bitfield order; see the notes. */
    ip->is_allow_pickup = TRUE;
    /* asm 0x0158 bit 0x10 (Flashbang.asm 127-128): the bitfield the decomp names can_rehit_shield. */
    ip->attack_coll.can_rehit_shield = TRUE;
    /* asm 0x0100 (Flashbang.asm 184) is the word named coll_data.cliff_id; the asm clears it. */
    ip->coll_data.cliff_id = 0;

    vars->fp = fp;
    vars->p_file = ftRemixPowerData(fp)->p_file_special1;
    vars->unk_0x354 = 4;
    vars->max_speed = ITREMIX_FLASHBANG_MAX_SPEED;
    vars->rotation_direction = 0.0F;
    vars->hitbox_refresh_timer = 0;
    vars->is_hit_collide = FALSE;

    ip->attack_coll.attack_state = ITREMIX_FLASHBANG_HITBOX_ON;
    ip->attack_coll.damage = ITREMIX_FLASHBANG_INITIAL_DAMAGE;
    ip->attack_coll.knockback_scale = 0;
    ip->attack_coll.knockback_weight = 0;
    ip->attack_coll.knockback_base = ITREMIX_FLASHBANG_INITIAL_KNOCKBACK_BASE;
    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->lifetime = ITREMIX_FLASHBANG_DURATION;
    ip->proc_dead = itRemixFlashbangProcDead;
}

/* Launch velocity: the owner's power along ANGLE, mirrored by its facing. The asm's y uses sin(ANGLE). */
static void itRemixFlashbangSetupLaunch(ITStruct *ip, FTStruct *fp)
{
    f32 speed = (f32)*ftRemixFighterWord(fp, ITREMIX_FLASHBANG_OWNER_POWER_N64);
    f32 direction = (f32)fp->lr;
    f32 cos_angle = __cosf(ITREMIX_FLASHBANG_ANGLE);
    f32 sin_angle = __sinf(ITREMIX_FLASHBANG_ANGLE);

    ip->physics.vel_air.x = (speed * cos_angle) * direction;
    ip->physics.vel_air.y = speed * sin_angle;
    ip->physics.vel_air.z = 0.0F;
}

/**
 * Spawns a flashbang thrown by parent_gobj from pos. vel is passed through to
 * the item manager, and the launch velocity overwrites it. Remix's stage routine
 * always passes flags = 0 to the spawn, so `flags` is unused. Returns NULL when
 * the item could not be created.
 */
GObj *itRemixFlashbangMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc item_desc = sITRemixFlashbangItemDesc;
    GObj *item_gobj;

    (void)flags;
    item_desc.p_file = ftRemixPowerData(fp)->p_file_special1;
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, FALSE);
    if (item_gobj == NULL)
    {
        return NULL;
    }

    itRemixFlashbangSetupRender(item_gobj);
    itRemixFlashbangSetupState(itGetStruct(item_gobj), parent_gobj, fp);
    itRemixFlashbangSetupLaunch(itGetStruct(item_gobj), fp);
    itRemixFlashbangSetActive(fp, item_gobj);
    mpCommonRunItemCollisionDefault(item_gobj, fp->coll_data.p_translate, &fp->coll_data);
    return item_gobj;
}

const ITRemixItemDesc dITRemixFlashbangItemDesc =
{
    ITREMIX_FLASHBANG_KIND,         /* Item.Flashbang.id */
    itRemixFlashbangMakeItem,       /* SPAWN_ITEM */
    FALSE,                          /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                           /* PICKUP_ITEM_INIT */
    NULL,                           /* PICKUP_ITEM_MAIN */
    NULL,                           /* DROP_ITEM */
    NULL,                           /* THROW_ITEM */
    NULL                            /* PLAYER_COLLISION */
};

#endif
