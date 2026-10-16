/**
 * itremixgordo.c - Smash Remix Gordo (Item.Gordo.id 0x48), the spiked ball
 * Dedede's down special throws. Port of items/Gordo.asm.
 *
 * Remix item fields that the vanilla ITStruct does not name (0x01C4 to 0x01D4,
 * 0x033E, 0x0354, 0x0398) live in ITRemixGordoVars inside item_vars. The notes
 * file lists the vanilla patches, REMIX-TODOs and uncertain offsets.
 */

#ifdef PORT

#include "it/itremix/itremixgordo.h"
#include "it/itremix/itremixwaddledee.h"
#include <math.h>
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <gm/gmcollision.h>
#include <it/item.h>
#include <it/itcommon/itmsbomb.h>
#include <it/itfighter/itlinkbomb.h>
#include <it/itmain.h>
#include <it/itmanager.h>
#include <it/itmap.h>
#include <mp/mpcommon.h>
#include "ft/ftremix/ftremix.h"

extern void *func_800269C0_275C0(u16 id);

#define ITREMIX_GORDO_KIND 0x48
#define ITREMIX_GORDO_ATTRIBUTES_OFFSET 0x120

#define ITREMIX_GORDO_DURATION 220
#define ITREMIX_GORDO_GRAVITY 2.0F
#define ITREMIX_GORDO_MAX_SPEED 48.0F
#define ITREMIX_GORDO_BOUNCE 0.95F
#define ITREMIX_GORDO_ANGLE 0.872665F
#define ITREMIX_GORDO_ROTATION 0.001F
#define ITREMIX_GORDO_DAMAGE 18
#define ITREMIX_GORDO_SPIN_BASE 0.017578125F
#define ITREMIX_GORDO_STICK_ANGLE_SCALE -0.0078125F
#define ITREMIX_GORDO_THROW_LIFT 200.0F
#define ITREMIX_GORDO_GROUND_DRAG 0.9375F
#define ITREMIX_GORDO_GROUND_STOP_SPEED 2.0F
#define ITREMIX_GORDO_LANDING_SPEED_MAX 32.0F
#define ITREMIX_GORDO_REFRESH_SPEED 35.0F
#define ITREMIX_GORDO_REFRESH_TIME 16
#define ITREMIX_GORDO_HURT_SPEED_SCALE 0.5F
#define ITREMIX_GORDO_HURT_SPEED_BASE 5.0F
#define ITREMIX_GORDO_HURT_MIN_DURATION 12
#define ITREMIX_GORDO_KNOCKBACK_SCALE 4.0F
#define ITREMIX_GORDO_KNOCKBACK_BASE 10.0F
#define ITREMIX_GORDO_HIT_MIN_DURATION 20
#define ITREMIX_GORDO_HAND_OFFSET_X 128.0F
#define ITREMIX_GORDO_STICK_COLL_SIZE 30.0F
#define ITREMIX_GORDO_FGM_SURFACE_STICK 0x107
#define ITREMIX_GORDO_SURFACE_STICK_DAMAGE 5
#define ITREMIX_GORDO_COLLISION_MASK 0x0C21
#define ITREMIX_GORDO_MASK_STAT_GROUND 0x0800
#define ITREMIX_GORDO_FAKE_KIND 0x1C
#define ITREMIX_GORDO_THROW_DESTROY 0xFFFF /* ITStruct.multi (N64 0x033E) read as -1 by lh */
#define ITREMIX_GORDO_NO_COMBO_PORT 4
#define ITREMIX_GORDO_HITBOX_FRESH 1
#define ITREMIX_GORDO_HITBOX_OFF 0
#define ITREMIX_GORDO_FIGHTER_JOINT_LEFT_HAND 10 /* Remix asm offset 0x0910 = FTStruct.joints[10] */

enum itRemixGordoStatus
{
    nITRemixGordoStatusNull,
    nITRemixGordoStatusAerial,
    nITRemixGordoStatusResting,
    nITRemixGordoStatusEnumCount
};

/* Remix item special struct fields beyond the vanilla ITStruct union. Offsets are the Remix asm ones. */
typedef struct ITRemixGordoVars
{
    u32 minion_reserved;                        /* ITRemixWaddleDeeVars prefix: the shared minion routines read player_gobj */
    s32 unk_0x354;                              /* 0x0354: written as 4 at spawn and throw; meaning unknown */
    GObj *player_gobj;                          /* 0x01C4: fighter that last owned Gordo */
    f32 max_speed;                              /* 0x01C8: speed cap; set to the knockback when hit */
    f32 rotation_direction;                     /* 0x01CC */
    s32 hitbox_refresh_timer;                   /* 0x01D0 */
    sb32 is_hit_collide;                        /* 0x01D4 */
} ITRemixGordoVars;

_Static_assert(offsetof(ITRemixGordoVars, player_gobj) == offsetof(ITRemixWaddleDeeVars, player_gobj), "Gordo must share the Waddle Dee minion layout");

_Static_assert(sizeof(ITRemixGordoVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixGordoVars must fit in ITStruct.item_vars");

static sb32 itRemixGordoNullProcUpdate(GObj *item_gobj);
static sb32 itRemixGordoAerialProcUpdate(GObj *item_gobj);
static sb32 itRemixGordoAerialProcCollision(GObj *item_gobj);
static sb32 itRemixGordoAerialHurtboxCollision(GObj *item_gobj);
static sb32 itRemixGordoAerialProcDamage(GObj *item_gobj);
static sb32 itRemixGordoRestingProcUpdate(GObj *item_gobj);
static sb32 itRemixGordoRestingProcMap(GObj *item_gobj);
static sb32 itRemixGordoRestingHurtboxCollision(GObj *item_gobj);
static void itRemixGordoThrowInitial(GObj *item_gobj, GObj *player_gobj);
static void itRemixGordoPinToHand(GObj *item_gobj);
static void itRemixGordoBeginThrow(GObj *item_gobj);
static void itRemixGordoBeginResting(GObj *item_gobj);
static void itRemixGordoStickToSurface(GObj *item_gobj);

static ITDesc sITRemixGordoItemDesc =
{
    ITREMIX_GORDO_KIND,
    NULL,                                   /* p_file: set from the owner fighter in itRemixGordoMakeItem */
    ITREMIX_GORDO_ATTRIBUTES_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixGordoNullProcUpdate,  /* 0x14: item_info_array runs into item_state_table, so the desc starts in state 0 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static ITStatusDesc sITRemixGordoStatusDescs[nITRemixGordoStatusEnumCount] =
{
    {
        itRemixGordoNullProcUpdate,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        itRemixGordoAerialProcUpdate,
        itRemixGordoAerialProcCollision,
        itRemixGordoAerialHurtboxCollision,
        itRemixGordoAerialHurtboxCollision,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixGordoAerialProcDamage
    },
    {
        itRemixGordoRestingProcUpdate,
        itRemixGordoRestingProcMap,
        itRemixGordoRestingHurtboxCollision,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/** Returns the Remix variable block of an item, stored in its item_vars union. */
static ITRemixGordoVars* itRemixGordoGetVars(GObj *item_gobj)
{
    return (ITRemixGordoVars*)&itGetStruct(item_gobj)->item_vars;
}

/** Returns the magnitude of the item's velocity (Gordo.asm sqrt of x^2 + y^2). */
static f32 itRemixGordoGetSpeed(ITStruct *ip)
{
    f32 x = ip->physics.vel_air.x;
    f32 y = ip->physics.vel_air.y;

    return sqrtf((x * x) + (y * y));
}

/** Returns the combo port of the attacking fighter, or ITREMIX_GORDO_NO_COMBO_PORT when none. */
static u8 itRemixGordoGetAttackerPort(ITStruct *ip)
{
    if ((ip->damage_gobj == NULL) || (ip->damage_gobj->id != nGCCommonKindFighter))
    {
        return ITREMIX_GORDO_NO_COMBO_PORT;
    }
    return ftGetStruct(ip->damage_gobj)->player;
}

/** Counts the hitbox refresh timer down; returns TRUE only on the frame it reaches 0. */
static sb32 itRemixGordoTickHitboxRefresh(ITRemixGordoVars *vars)
{
    if (vars->hitbox_refresh_timer == 0)
    {
        return FALSE;
    }
    vars->hitbox_refresh_timer--;
    return (vars->hitbox_refresh_timer == 0);
}

/** Clears the is_hit_collide flag; every exit of the aerial and resting updates does this in the asm. */
static void itRemixGordoClearHitFlag(GObj *item_gobj)
{
    itRemixGordoGetVars(item_gobj)->is_hit_collide = FALSE;
}

/** Gordo.gordo_null_main_ (state 0): pins Gordo to the owner's hand, throws it, or destroys it. */
static sb32 itRemixGordoNullProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);

    if (ip->multi == ITREMIX_GORDO_THROW_DESTROY)
    {
        ip->kind = ITREMIX_GORDO_FAKE_KIND;
        return TRUE;
    }
    if (ip->multi == 0)
    {
        itRemixGordoPinToHand(item_gobj);
    }
    else
    {
        itRemixGordoThrowInitial(item_gobj, ip->owner_gobj);
        itRemixWaddleDeeRemoveMinionCheck(item_gobj);
    }
    ip->lifetime++;
    return FALSE;
}

/** Gordo.gordo_null_main_ (pin branch): moves Gordo to the owner's left hand. */
static void itRemixGordoPinToHand(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);
    DObj *joint = item_gobj->obj;
    FTStruct *fp = ftGetStruct(ip->owner_gobj);
    Vec3f *hand_pos = &joint->translate.vec.f;

    hand_pos->x = ITREMIX_GORDO_HAND_OFFSET_X;
    hand_pos->y = 0.0F;
    hand_pos->z = 0.0F;
    gmCollisionGetFighterPartsWorldPosition(fp->joints[ITREMIX_GORDO_FIGHTER_JOINT_LEFT_HAND], hand_pos);
    joint->translate.vec.f.z = 0.0F;
    joint->rotate.vec.f.z = 0.0F;
    vars->rotation_direction = 0.0F;
}

/** Gordo.gordo_throw_initial_: applies the thrown speed and direction, then starts the aerial state. */
static void itRemixGordoThrowInitial(GObj *item_gobj, GObj *player_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);
    FTStruct *fp = ftGetStruct(player_gobj);
    DObj *joint = item_gobj->obj;
    f32 direction = (f32)fp->lr;
    f32 stick_x = (f32)fp->input.pl.stick_range.x;
    f32 angle = ITREMIX_GORDO_ANGLE + ((stick_x * direction) * ITREMIX_GORDO_STICK_ANGLE_SCALE);

    joint->translate.vec.f.y -= ITREMIX_GORDO_THROW_LIFT;
    joint->translate.vec.f.z = 0.0F;
    ip->attack_coll.can_rehit_shield = TRUE;
    vars->unk_0x354 = 4;
    ip->lifetime = ITREMIX_GORDO_DURATION;
    ip->physics.vel_air.x = (ITREMIX_GORDO_MAX_SPEED * cosf(angle)) * direction;
    ip->physics.vel_air.y = ITREMIX_GORDO_MAX_SPEED * sinf(angle);
    ip->physics.vel_air.z = 0.0F;
    ip->attack_coll.attack_state = ITREMIX_GORDO_HITBOX_FRESH;
    ip->lr = fp->lr;

    itRemixGordoBeginThrow(item_gobj);
    mpCommonRunItemCollisionDefault(item_gobj, fp->coll_data.p_translate, &fp->coll_data);
}

/** Gordo.begin_throw_: changes Gordo to the aerial state. */
static void itRemixGordoBeginThrow(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);

    ip->attack_coll.attack_state = ITREMIX_GORDO_HITBOX_FRESH;
    itMapSetAir(ip);
    ip->multi = 0;
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixGordoStatusDescs, nITRemixGordoStatusAerial);
    ip->damage_coll.hitstatus = 0;
}

/** Gordo.begin_resting_: changes Gordo to the grounded/resting state. */
static void itRemixGordoBeginResting(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = 0;
    ip->times_landed = 0;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    itRemixGordoStickToSurface(item_gobj);
    itLinkBombCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixGordoStatusDescs, nITRemixGordoStatusResting);
}

/** Gordo.stick_to_surface: attaches Gordo to the surface it hit and plays the stick sound. */
static void itRemixGordoStickToSurface(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = item_gobj->obj;
    f32 saved_rotate_z = joint->rotate.vec.f.z;

    ip->coll_data.map_coll.top = ITREMIX_GORDO_STICK_COLL_SIZE;
    ip->coll_data.map_coll.width = ITREMIX_GORDO_STICK_COLL_SIZE;
    ip->coll_data.map_coll.center = 0.0F;
    ip->coll_data.map_coll.bottom = -ITREMIX_GORDO_STICK_COLL_SIZE;
    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;

    itMSBombAttachedUpdateSurface(item_gobj);
    joint->rotate.vec.f.z = saved_rotate_z;
    ip->is_attach_surface = TRUE;

    /* REMIX-TODO(rumble): asm rumbles the fighter at port ip->player (not the owner) via the table at 0x800A50E8 */
    ip->attack_coll.damage = ITREMIX_GORDO_SURFACE_STICK_DAMAGE;
    func_800269C0_275C0(ITREMIX_GORDO_FGM_SURFACE_STICK);
}

/** Gordo.stage_setting_: SPAWN_ITEM. Creates a Gordo owned by parent_gobj; Remix always passes flags = 0. */
GObj* itRemixGordoMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    ITDesc item_desc = sITRemixGordoItemDesc;
    FTStruct *fp = ftGetStruct(parent_gobj);
    GObj *item_gobj;
    ITStruct *ip;
    ITRemixGordoVars *vars;

    item_desc.p_file = ftRemixPowerData(fp)->p_file_special1;
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, 0);
    if (item_gobj == NULL)
    {
        return NULL;
    }

    ip = itGetStruct(item_gobj);
    vars = itRemixGordoGetVars(item_gobj);
    ((DObj*)item_gobj->obj)->translate.vec.f.z = 0.0F;
    ip->multi = 0;
    /* REMIX-TODO(gfx): asm calls 0x80008CC0 (render routine setup, a1 = 0x2E, a2 = 0); not identified in decomp */
    ip->attack_coll.can_rehit_shield = TRUE;
    ip->lifetime = 0;
    vars->unk_0x354 = 4;
    ip->owner_gobj = parent_gobj;
    ip->player = fp->player;
    ip->team = fp->team;
    ip->handicap = fp->handicap;
    vars->player_gobj = parent_gobj;
    ip->attack_coll.attack_state = ITREMIX_GORDO_HITBOX_OFF;
    ip->damage_coll.hitstatus = 0;
    vars->max_speed = ITREMIX_GORDO_MAX_SPEED;
    vars->rotation_direction = 0.0F;
    vars->hitbox_refresh_timer = 0;
    vars->is_hit_collide = FALSE;
    ip->attack_coll.damage = ITREMIX_GORDO_DAMAGE;
    ip->proc_dead = itRemixWaddleDeeMinionBlastZone; /* Remix stores it in the blast wall slot (ITStruct 0x398) */

    return item_gobj;
}

/** Gordo.gordo_aerial_main_ speed step: ground drag when grounded, otherwise air speed cap and gravity. */
static void itRemixGordoApplyAerialSpeed(ITStruct *ip, ITRemixGordoVars *vars)
{
    if (ip->ga != 0)
    {
        vars->max_speed -= 1.0F;
        if (vars->max_speed <= ITREMIX_GORDO_MAX_SPEED)
        {
            vars->max_speed = ITREMIX_GORDO_MAX_SPEED;
        }
        itMainApplyGravityClampTVel(ip, ITREMIX_GORDO_GRAVITY, vars->max_speed);
        return;
    }

    ip->physics.vel_air.x *= ITREMIX_GORDO_GROUND_DRAG;
    if (fabsf(ip->physics.vel_air.x) < ITREMIX_GORDO_GROUND_STOP_SPEED)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->attack_coll.attack_state = ITREMIX_GORDO_HITBOX_OFF;
    }
}

/** Gordo.gordo_aerial_main_ spin step; returns TRUE when Gordo is grounded and must be destroyed. */
static sb32 itRemixGordoApplySpin(GObj *item_gobj, f32 speed)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);
    f32 spin;

    if (ip->physics.vel_air.x != 0.0F)
    {
        vars->rotation_direction = (ip->physics.vel_air.x > 0.0F) ? 1.0F : -1.0F;
    }
    spin = (ITREMIX_GORDO_SPIN_BASE + (ITREMIX_GORDO_ROTATION * speed)) * vars->rotation_direction;
    if (ip->ga == 0)
    {
        return TRUE;
    }
    ((DObj*)item_gobj->obj)->rotate.vec.f.z -= spin;
    return FALSE;
}

/** Gordo.gordo_aerial_main_ (body): returns TRUE when Gordo is destroyed this frame. */
static sb32 itRemixGordoAerialStep(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);
    f32 speed;

    itRemixGordoApplyAerialSpeed(ip, vars);
    if (ip->lifetime == 0)
    {
        itRemixWaddleDeeMinionFree(item_gobj);
        return TRUE;
    }
    ip->lifetime--;

    speed = itRemixGordoGetSpeed(ip);
    if (itRemixGordoApplySpin(item_gobj, speed))
    {
        return TRUE;
    }

    if (itRemixGordoTickHitboxRefresh(vars))
    {
        itMainClearAttackRecord(ip);
    }
    else if (vars->hitbox_refresh_timer != 0)
    {
        return FALSE;
    }
    if (speed >= ITREMIX_GORDO_REFRESH_SPEED)
    {
        itMainClearAttackRecord(ip);
    }
    return FALSE;
}

/** Gordo.gordo_aerial_main_ (state 1 main proc). */
static sb32 itRemixGordoAerialProcUpdate(GObj *item_gobj)
{
    sb32 is_destroyed = itRemixGordoAerialStep(item_gobj);

    itRemixGordoClearHitFlag(item_gobj);
    return is_destroyed;
}

/** Gordo.gordo_collision_ (state 1 map proc): bounces off the map and rests when it lands. */
static sb32 itRemixGordoAerialProcCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = item_gobj->obj;
    f32 saved_rotate_z = joint->rotate.vec.f.z;
    sb32 is_collide = itMapTestAllCollisionFlag(item_gobj, ITREMIX_GORDO_COLLISION_MASK);

    if (is_collide != FALSE)
    {
        joint->rotate.vec.f.z = saved_rotate_z;
    }
    itMapCheckCollideAllRebound(item_gobj, ITREMIX_GORDO_COLLISION_MASK, ITREMIX_GORDO_BOUNCE, NULL);
    if (is_collide == FALSE)
    {
        return FALSE;
    }
    if (((ip->coll_data.mask_stat & ITREMIX_GORDO_MASK_STAT_GROUND) != 0) &&
        (fabsf(ip->physics.vel_air.y) >= ITREMIX_GORDO_LANDING_SPEED_MAX))
    {
        return FALSE;
    }
    itRemixGordoBeginResting(item_gobj);
    return FALSE;
}

/** Gordo.gordo_hurtbox_collision_ (state 1): reduces the lifetime when Gordo is hit by a hurtbox. */
static sb32 itRemixGordoAerialHurtboxCollision(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);
    f32 speed = itRemixGordoGetSpeed(ip);
    f32 hit_speed = (speed * ITREMIX_GORDO_HURT_SPEED_SCALE) + ITREMIX_GORDO_HURT_SPEED_BASE;
    sb32 was_hit = vars->is_hit_collide;
    s32 speed_int;
    s32 duration_loss;

    vars->is_hit_collide = FALSE;
    if (was_hit == FALSE)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->physics.vel_air.y = hit_speed;
    }
    vars->hitbox_refresh_timer = ITREMIX_GORDO_REFRESH_TIME;

    speed_int = (s32)speed;
    duration_loss = speed_int + (speed_int >> 1);
    ip->lifetime -= duration_loss;
    if (ip->lifetime < ITREMIX_GORDO_HURT_MIN_DURATION)
    {
        ip->lifetime = ITREMIX_GORDO_HURT_MIN_DURATION;
    }
    return FALSE;
}

/** Gordo.gordo_hitbox_collision_ (state 1 damage proc): launches Gordo when it is hit by an attack. */
static sb32 itRemixGordoAerialProcDamage(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);
    f32 knockback;
    f32 angle;
    f32 velocity_x;
    f32 velocity_y;
    s32 direction;

    itRemixGordoBeginThrow(item_gobj);
    ip->owner_gobj = ip->damage_gobj;
    ip->player = itRemixGordoGetAttackerPort(ip);

    knockback = ((f32)ip->damage_queue * ITREMIX_GORDO_KNOCKBACK_SCALE) + ITREMIX_GORDO_KNOCKBACK_BASE;
    vars->max_speed = knockback;
    angle = ftCommonDamageGetKnockbackAngle(ip->damage_angle, ip->ga, knockback);
    velocity_x = knockback * cosf(angle);
    velocity_y = knockback * sinf(angle);

    direction = -ip->damage_lr;
    ip->lr = direction;
    ip->physics.vel_air.x = velocity_x * (f32)direction;
    ip->physics.vel_air.y = velocity_y;
    vars->hitbox_refresh_timer = ITREMIX_GORDO_REFRESH_TIME;
    vars->is_hit_collide = TRUE;

    ip->lifetime -= ip->damage_queue * 2;
    if (ip->lifetime < ITREMIX_GORDO_HIT_MIN_DURATION)
    {
        ip->lifetime = ITREMIX_GORDO_HIT_MIN_DURATION;
    }
    return FALSE;
}

/** Gordo.resting_main_ (body): returns TRUE when Gordo is destroyed this frame. */
static sb32 itRemixGordoRestingStep(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixGordoVars *vars = itRemixGordoGetVars(item_gobj);

    ip->physics.vel_air.x = 0.0F;
    if (ip->lifetime == 0)
    {
        itRemixWaddleDeeMinionFree(item_gobj);
        return TRUE;
    }
    ip->lifetime--;

    if (itRemixGordoTickHitboxRefresh(vars))
    {
        itMainClearAttackRecord(ip);
    }
    return FALSE;
}

/** Gordo.resting_main_ (state 2 main proc). */
static sb32 itRemixGordoRestingProcUpdate(GObj *item_gobj)
{
    sb32 is_destroyed = itRemixGordoRestingStep(item_gobj);

    itRemixGordoClearHitFlag(item_gobj);
    return is_destroyed;
}

/** Gordo.resting_collision_ (state 2 map proc). The generic resting collision call is disabled in the asm (it crashes). */
static sb32 itRemixGordoRestingProcMap(GObj *item_gobj)
{
    return FALSE;
}

/** Gordo.resting_hurtbox_collision_ (state 2): refreshes the hitbox timer when hit by a hurtbox. */
static sb32 itRemixGordoRestingHurtboxCollision(GObj *item_gobj)
{
    itRemixGordoGetVars(item_gobj)->hitbox_refresh_timer = ITREMIX_GORDO_REFRESH_TIME;
    return FALSE;
}

/* Descriptor for the Remix item table (Gordo.asm: no pickup, drop, throw or touch routines). */
const ITRemixItemDesc dITRemixGordoItemDesc =
{
    ITREMIX_GORDO_KIND,         /* Item.Gordo.id */
    itRemixGordoMakeItem,       /* SPAWN_ITEM */
    FALSE,                      /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                       /* PICKUP_ITEM_INIT */
    NULL,                       /* PICKUP_ITEM_MAIN */
    NULL,                       /* DROP_ITEM */
    NULL,                       /* THROW_ITEM */
    NULL                        /* PLAYER_COLLISION */
};

#endif
