#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixboomerang.h"
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixhooks.h"
#endif
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <it/itcommon/itbombhei.h>
#include <it/itmain.h>
#include <it/itmanager.h>
#include <mp/mpdef.h>
#include <mp/mpprocess.h>
#include <sc/scene.h>
#include <if/ifcommon.h>
#include "ft/ftremix/ftremix.h"

#define ITREMIX_BOOMERANG_KIND 0x44
#define ITREMIX_BOOMERANG_FILE_OFFSET 0xE0 /* Boomerang.asm FILE_OFFSET: item footer in file 0xFB */
#define ITREMIX_BOOMERANG_BASE_DAMAGE 1
#define ITREMIX_BOOMERANG_KB_GROWTH 40
#define ITREMIX_BOOMERANG_BASE_KNOCKBACK 70
#define ITREMIX_BOOMERANG_KB_ANGLE 68
#define ITREMIX_BOOMERANG_HIT_FGM 0x1F /* heavy kick FGM */
#define ITREMIX_BOOMERANG_THROW_TIMER 28
#define ITREMIX_BOOMERANG_RETURN_TIMER 40
#define ITREMIX_BOOMERANG_COLLIDE_FLAGS 0x0C21 /* all map collision types */
#define ITREMIX_BOOMERANG_SPAWN_GFX 0x2E

#define ITREMIX_BOOMERANG_TURN_ACCEL 0.0546875F   /* 0x3D600000 */
#define ITREMIX_BOOMERANG_CATCH_RANGE_X 280.0F    /* 0x438C0000 */
#define ITREMIX_BOOMERANG_CATCH_RANGE_LY -100.0F  /* 0xC2C80000 */
#define ITREMIX_BOOMERANG_CATCH_RANGE_UY 440.0F   /* 0x43DC0000 */

#define ITREMIX_BOOMERANG_THROW_HIT_MULTIPLIER -0.9375F /* 0xBF700000 */
#define ITREMIX_BOOMERANG_TURN_HIT_MULTIPLIER 0.9375F   /* 0x3F700000 */
#define ITREMIX_BOOMERANG_RETURN_HIT_MULTIPLIER 0.9375F /* 0x3F700000 */
#define ITREMIX_BOOMERANG_CLIP_HIT_MULTIPLIER -0.5F     /* 0xBF000000 */
#define ITREMIX_BOOMERANG_DAMAGE_MULTIPLIER 0.5F        /* 0x3F000000 */
#define ITREMIX_BOOMERANG_SHIELD_MULTIPLIER -0.1875F    /* 0xBE400000 */

enum ITRemixBoomerangStatus
{
    nITRemixBoomerangStatusGround,
    nITRemixBoomerangStatusAir,
    nITRemixBoomerangStatusPickup,
    nITRemixBoomerangStatusThrown,
    nITRemixBoomerangStatusTurn,
    nITRemixBoomerangStatusReturn,
    nITRemixBoomerangStatusFall,
    nITRemixBoomerangStatusCount
};

enum ITRemixBoomerangAxis
{
    nITRemixBoomerangAxisX,
    nITRemixBoomerangAxisY
};

/* Fighter action ids from Action.asm, used by the catch check. */
enum ITRemixBoomerangAction
{
    nITRemixBoomerangActionIdle = 0x0A,
    nITRemixBoomerangActionTeeterStart = 0x24,
    nITRemixBoomerangActionFallSpecial = 0x3A,
    nITRemixBoomerangActionLandingSpecial = 0x3B,
    nITRemixBoomerangActionCeilingBonk = 0x42,
    nITRemixBoomerangActionClang = 0x52,
    nITRemixBoomerangActionCliffWait = 0x55,
    nITRemixBoomerangActionItemThrowDash = 0x69,
    nITRemixBoomerangActionItemThrowAirSmashD = 0x79,
    nITRemixBoomerangActionShieldOn = 0x98,
    nITRemixBoomerangActionShieldStun = 0x9B,
    nITRemixBoomerangActionTaunt = 0xBD
};

/* Boomerang state kept in ITStruct.item_vars. The asm's N64 offsets 0x1CC..0x1D8 have no named ITStruct field. */
typedef struct ITRemixBoomerangVars
{
    s32 timer;
    s32 target_axis;
    f32 target_vel;
    f32 accel;
} ITRemixBoomerangVars;

_Static_assert(sizeof(ITRemixBoomerangVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixBoomerangVars must fit in ITStruct.item_vars");

static ITRemixBoomerangVars* itRemixBoomerangGetVars(ITStruct *ip);
static f32* itRemixBoomerangAxisVel(ITStruct *ip, s32 axis);
static f32 itRemixBoomerangAbs(f32 value);
static void itRemixBoomerangApplyRotation(GObj *item_gobj);
static void itRemixBoomerangApplyMultiplier(GObj *item_gobj, f32 multiplier);
static void itRemixBoomerangSetBaseAttack(ITStruct *ip);
static void itRemixBoomerangZeroMinorAxis(ITStruct *ip);
static void itRemixBoomerangRunWallsAndCeil(MPCollData *coll_data);
static sb32 itRemixBoomerangProcCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags);
static sb32 itRemixBoomerangIsInCatchRange(DObj *item_joint, DObj *fighter_joint, f32 size_multiplier);
static sb32 itRemixBoomerangIsCatchableAction(FTStruct *fp);
static sb32 itRemixBoomerangHandleCatch(GObj *item_gobj, GObj *fighter_gobj);
static void itRemixBoomerangTryCatchOwner(GObj *item_gobj);

static void itRemixBoomerangHoldProc(GObj *item_gobj);
static void itRemixBoomerangThrownSetStatus(GObj *item_gobj);
static void itRemixBoomerangTurnSetStatus(GObj *item_gobj);
static void itRemixBoomerangReturnSetStatus(GObj *item_gobj);
static void itRemixBoomerangFallSetStatus(GObj *item_gobj);
static sb32 itRemixBoomerangThrownProcUpdate(GObj *item_gobj);
static sb32 itRemixBoomerangThrownProcMap(GObj *item_gobj);
static sb32 itRemixBoomerangThrownProcHit(GObj *item_gobj);
static sb32 itRemixBoomerangTurnProcUpdate(GObj *item_gobj);
static sb32 itRemixBoomerangTurnProcMap(GObj *item_gobj);
static sb32 itRemixBoomerangTurnProcHit(GObj *item_gobj);
static sb32 itRemixBoomerangReturnProcUpdate(GObj *item_gobj);
static sb32 itRemixBoomerangReturnProcMap(GObj *item_gobj);
static sb32 itRemixBoomerangReturnProcHit(GObj *item_gobj);
static sb32 itRemixBoomerangReturnProcClippingHit(GObj *item_gobj);
static sb32 itRemixBoomerangFallProcUpdate(GObj *item_gobj);
static sb32 itRemixBoomerangProcDamage(GObj *item_gobj);
static sb32 itRemixBoomerangProcShield(GObj *item_gobj);
static sb32 itRemixBoomerangProcReflector(GObj *item_gobj);

static ITStatusDesc sITRemixBoomerangStatusDescs[nITRemixBoomerangStatusCount] =
{
    /* Ground wait (Remix status 0) */
    {
        NULL,
        itTomatoWaitProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Air wait (Remix status 1) */
    {
        itTomatoFallProcUpdate,
        itTomatoFallProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Pickup (Remix status 2) */
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Thrown (Remix status 3) */
    {
        itRemixBoomerangThrownProcUpdate,
        itRemixBoomerangThrownProcMap,
        itRemixBoomerangThrownProcHit,
        itRemixBoomerangProcShield,
        itMainCommonProcHop,
        NULL,
        itRemixBoomerangProcReflector,
        itRemixBoomerangProcDamage
    },

    /* Turning (Remix status 4) */
    {
        itRemixBoomerangTurnProcUpdate,
        itRemixBoomerangTurnProcMap,
        itRemixBoomerangTurnProcHit,
        itRemixBoomerangProcShield,
        itMainCommonProcHop,
        NULL,
        itRemixBoomerangProcReflector,
        itRemixBoomerangProcDamage
    },

    /* Returning (Remix status 5) */
    {
        itRemixBoomerangReturnProcUpdate,
        itRemixBoomerangReturnProcMap,
        itRemixBoomerangReturnProcHit,
        itRemixBoomerangProcShield,
        itMainCommonProcHop,
        NULL,
        itRemixBoomerangProcReflector,
        itRemixBoomerangProcDamage
    },

    /* Falling (Remix status 6) */
    {
        itRemixBoomerangFallProcUpdate,
        itTomatoDroppedProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/* Remix item_info_array. Air wait is the initial proc, as for the Bob-omb spawn it is based on. */
static ITDesc sITRemixBoomerangItemDesc =
{
    ITREMIX_BOOMERANG_KIND,                     /* Item.Boomerang.id */
    NULL,                                       /* p_file: set per owner in itRemixBoomerangMakeItem */
    ITREMIX_BOOMERANG_FILE_OFFSET,
    { nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 },
    nGMAttackStateOff,
    itTomatoFallProcUpdate,
    itTomatoFallProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

const ITRemixItemDesc dITRemixBoomerangItemDesc =
{
    ITREMIX_BOOMERANG_KIND,             /* Item.Boomerang.id */
    itRemixBoomerangMakeItem,           /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    itRemixBoomerangHoldProc,           /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,           /* PICKUP_ITEM_MAIN: pickup_boomerang jumps to 0x80145C4C */
    itTomatoDroppedSetStatus,           /* DROP_ITEM */
    itRemixBoomerangThrownSetStatus,    /* THROW_ITEM */
    NULL                                /* PLAYER_COLLISION */
};

/* Remix item state accessor: the Boomerang's own vars live in item_vars. */
static ITRemixBoomerangVars* itRemixBoomerangGetVars(ITStruct *ip)
{
    return (ITRemixBoomerangVars*)&ip->item_vars;
}

/* Returns the x or y velocity component selected by the turn axis. */
static f32* itRemixBoomerangAxisVel(ITStruct *ip, s32 axis)
{
    if (axis == nITRemixBoomerangAxisX)
    {
        return &ip->physics.vel_air.x;
    }
    return &ip->physics.vel_air.y;
}

static f32 itRemixBoomerangAbs(f32 value)
{
    return (value < 0.0F) ? -value : value;
}

/* Remix stage_setting_: spawns the Boomerang with the owner's file 8 and the Marina base attack values. */
GObj* itRemixBoomerangMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc item_desc = sITRemixBoomerangItemDesc;
    GObj *item_gobj;
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    item_desc.p_file = ftRemixPowerData(fp)->p_file_special3;
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }

    ip = itGetStruct(item_gobj);
    joint = DObjGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    ip->multi = 0;
    itMainClearOwnerStats(item_gobj);
    gcAddXObjForDObjFixed(joint, ITREMIX_BOOMERANG_SPAWN_GFX, 0);
    joint->translate.vec.f = translate;

    itRemixBoomerangSetBaseAttack(ip);
    ip->is_unused_item_bool = TRUE;
    joint->rotate.vec.f.z = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/* Remix stage_setting_ attack values: base damage, knockback and the heavy kick FGM. */
static void itRemixBoomerangSetBaseAttack(ITStruct *ip)
{
    ip->attack_coll.damage = ITREMIX_BOOMERANG_BASE_DAMAGE;
    ip->attack_coll.knockback_scale = ITREMIX_BOOMERANG_KB_GROWTH;
    ip->attack_coll.knockback_base = ITREMIX_BOOMERANG_BASE_KNOCKBACK;
    ip->attack_coll.angle = ITREMIX_BOOMERANG_KB_ANGLE;
    ip->attack_coll.fgm_id = ITREMIX_BOOMERANG_HIT_FGM;
}

/* Remix throw_initial_: starts the throw timer, enables the hitbox and enters the thrown state. */
static void itRemixBoomerangThrownSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixBoomerangGetVars(ip)->timer = ITREMIX_BOOMERANG_THROW_TIMER;
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixBoomerangStatusDescs, nITRemixBoomerangStatusThrown);
}

/* Remix turn_initial_: picks the axis to reverse (x unless x is zero) and starts the turn acceleration. */
static void itRemixBoomerangTurnSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBoomerangVars *vars = itRemixBoomerangGetVars(ip);
    f32 velocity;

    if (ip->physics.vel_air.x != 0.0F)
    {
        vars->target_axis = nITRemixBoomerangAxisX;
        velocity = ip->physics.vel_air.x;
    }
    else
    {
        vars->target_axis = nITRemixBoomerangAxisY;
        velocity = ip->physics.vel_air.y;
    }
    vars->target_vel = -velocity;
    vars->accel = vars->target_vel * ITREMIX_BOOMERANG_TURN_ACCEL;

    itMainSetStatus(item_gobj, sITRemixBoomerangStatusDescs, nITRemixBoomerangStatusTurn);
    ip->is_thrown = TRUE;
}

/* Remix return_initial_: starts the return timer and enables the bonus damage flag. */
static void itRemixBoomerangReturnSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixBoomerangGetVars(ip)->timer = ITREMIX_BOOMERANG_RETURN_TIMER;
    itMainSetStatus(item_gobj, sITRemixBoomerangStatusDescs, nITRemixBoomerangStatusReturn);
    ip->is_thrown = TRUE;
}

/* Remix falling_initial_: disables both boxes and falls. */
static void itRemixBoomerangFallSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    itMainSetStatus(item_gobj, sITRemixBoomerangStatusDescs, nITRemixBoomerangStatusFall);
}

/* Remix thrown_main_: spins, then counts the throw timer down and turns when it reaches zero. */
static sb32 itRemixBoomerangThrownProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBoomerangVars *vars = itRemixBoomerangGetVars(ip);

    if (vars->timer == ITREMIX_BOOMERANG_THROW_TIMER)
    {
        itRemixBoomerangZeroMinorAxis(ip);
    }
    itRemixBoomerangApplyRotation(item_gobj);

    vars->timer -= 1;
    if (vars->timer == 0)
    {
        itRemixBoomerangTurnSetStatus(item_gobj);
    }
    return FALSE;
}

/* Remix thrown_main_ first frame: zeroes the smaller velocity component, so the throw travels on one axis. */
static void itRemixBoomerangZeroMinorAxis(ITStruct *ip)
{
    f32 vel_x = ip->physics.vel_air.x;
    f32 vel_y = ip->physics.vel_air.y;

    if ((vel_x == 0.0F) || (vel_y == 0.0F))
    {
        return;
    }
    if (itRemixBoomerangAbs(vel_x) <= itRemixBoomerangAbs(vel_y))
    {
        ip->physics.vel_air.x = 0.0F;
    }
    else
    {
        ip->physics.vel_air.y = 0.0F;
    }
}

/* Remix throw_collision_: runs the map collision and enters the hit routine on contact. */
static sb32 itRemixBoomerangThrownProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (mpProcessUpdateMain(&ip->coll_data, itRemixBoomerangProcCollision, item_gobj, ITREMIX_BOOMERANG_COLLIDE_FLAGS) != FALSE)
    {
        itRemixBoomerangThrownProcHit(item_gobj);
    }
    return FALSE;
}

/* Remix throw_collide_: damps the velocity and returns. */
static sb32 itRemixBoomerangThrownProcHit(GObj *item_gobj)
{
    itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_THROW_HIT_MULTIPLIER);
    itRemixBoomerangReturnSetStatus(item_gobj);
    return FALSE;
}

/* Remix turn_main_: accelerates the target axis toward the reversed velocity, then returns when it passes it. */
static sb32 itRemixBoomerangTurnProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBoomerangVars *vars = itRemixBoomerangGetVars(ip);
    f32 *velocity = itRemixBoomerangAxisVel(ip, vars->target_axis);

    itRemixBoomerangApplyRotation(item_gobj);

    *velocity += vars->accel;
    if (itRemixBoomerangAbs(vars->target_vel) < itRemixBoomerangAbs(*velocity))
    {
        *velocity = vars->target_vel;
        itRemixBoomerangReturnSetStatus(item_gobj);
    }
    return FALSE;
}

/* Remix turn_collision_: runs the map collision and enters the hit routine on contact. */
static sb32 itRemixBoomerangTurnProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (mpProcessUpdateMain(&ip->coll_data, itRemixBoomerangProcCollision, item_gobj, ITREMIX_BOOMERANG_COLLIDE_FLAGS) != FALSE)
    {
        itRemixBoomerangTurnProcHit(item_gobj);
    }
    return FALSE;
}

/* Remix turn_collide_: snaps the axis to the target velocity, starts the return, then damps. */
static sb32 itRemixBoomerangTurnProcHit(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBoomerangVars *vars = itRemixBoomerangGetVars(ip);

    *itRemixBoomerangAxisVel(ip, vars->target_axis) = vars->target_vel;
    itRemixBoomerangReturnSetStatus(item_gobj);
    itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_TURN_HIT_MULTIPLIER);
    return FALSE;
}

/* Remix return_main_: spins, checks for a catch by the owner, then falls when the return timer runs out. */
static sb32 itRemixBoomerangReturnProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBoomerangVars *vars = itRemixBoomerangGetVars(ip);

    itRemixBoomerangApplyRotation(item_gobj);
    itRemixBoomerangTryCatchOwner(item_gobj);

    vars->timer -= 1;
    if (vars->timer == 0)
    {
        itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_DAMAGE_MULTIPLIER);
        itRemixBoomerangFallSetStatus(item_gobj);
    }
    return FALSE;
}

/* Remix return_collision_: runs the map collision and enters the clipping hit routine on contact. */
static sb32 itRemixBoomerangReturnProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (mpProcessUpdateMain(&ip->coll_data, itRemixBoomerangProcCollision, item_gobj, ITREMIX_BOOMERANG_COLLIDE_FLAGS) != FALSE)
    {
        itRemixBoomerangReturnProcClippingHit(item_gobj);
    }
    return FALSE;
}

/* Remix return_collide_: damps the velocity and returns. */
static sb32 itRemixBoomerangReturnProcHit(GObj *item_gobj)
{
    itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_RETURN_HIT_MULTIPLIER);
    return FALSE;
}

/* Remix return_collide_clipping_: a wall or ceiling clip reverses the velocity and falls. */
static sb32 itRemixBoomerangReturnProcClippingHit(GObj *item_gobj)
{
    itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_CLIP_HIT_MULTIPLIER);
    itRemixBoomerangFallSetStatus(item_gobj);
    return FALSE;
}

/* Remix falling_main_: runs the tomato fall update, then checks for a catch by the owner. */
static sb32 itRemixBoomerangFallProcUpdate(GObj *item_gobj)
{
    itTomatoFallProcUpdate(item_gobj);
    itRemixBoomerangTryCatchOwner(item_gobj);
    return FALSE;
}

/* Remix hurtbox_collision_: a hurtbox hit damps the velocity and falls. */
static sb32 itRemixBoomerangProcDamage(GObj *item_gobj)
{
    itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_DAMAGE_MULTIPLIER);
    itRemixBoomerangFallSetStatus(item_gobj);
    return FALSE;
}

/* Remix shield_collision_: a shield hit damps the velocity, drops the owner and falls. */
static sb32 itRemixBoomerangProcShield(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixBoomerangApplyMultiplier(item_gobj, ITREMIX_BOOMERANG_SHIELD_MULTIPLIER);
    ip->owner_gobj = NULL;
    itRemixBoomerangFallSetStatus(item_gobj);
    return FALSE;
}

/* Remix reflect_: the generic reflect routine, then a fresh throw. */
static sb32 itRemixBoomerangProcReflector(GObj *item_gobj)
{
    itMainCommonProcReflector(item_gobj);
    itRemixBoomerangThrownSetStatus(item_gobj);
    return FALSE;
}

/* Remix apply_multiplier_: scales the x and y velocity. */
static void itRemixBoomerangApplyMultiplier(GObj *item_gobj, f32 multiplier)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x = ip->physics.vel_air.x * multiplier;
    ip->physics.vel_air.y = ip->physics.vel_air.y * multiplier;
}

/* Remix apply_rotation_: adds spin_step to the joint rotation twice per frame. */
static void itRemixBoomerangApplyRotation(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);
    f32 rotation = joint->rotate.vec.f.z + ip->spin_step;

    rotation += ip->spin_step;
    joint->rotate.vec.f.z = rotation;
}

/* Remix detect_collision_: itMapProcAllCheckCollisionFlag without the ground block (edge adjust, landing FGM and spin reset). */
static sb32 itRemixBoomerangProcCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags)
{
    itRemixBoomerangRunWallsAndCeil(coll_data);
    if (mpProcessRunFloorCollisionAdjNewNULL(coll_data) != FALSE)
    {
        mpProcessSetLandingFloor(coll_data);
    }
    return (coll_data->mask_curr & coll_flags) != 0;
}

/* Remix detect_collision_ walls and ceiling part. */
static void itRemixBoomerangRunWallsAndCeil(MPCollData *coll_data)
{
    if (mpProcessCheckTestLWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunLWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestRWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunRWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestCeilCollisionAdjNew(coll_data) == FALSE)
    {
        return;
    }
    mpProcessRunCeilCollisionAdjNew(coll_data);
    if (coll_data->mask_stat & MAP_FLAG_CEIL)
    {
        mpProcessRunCeilEdgeAdjust(coll_data);
    }
}

/* Remix prepickup_: disables the hurtbox and hitbox, then enters the pickup state. */
static void itRemixBoomerangHoldProc(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itBombHeiCommonSetHitStatusNone(item_gobj);
    ip->attack_coll.attack_state = nGMAttackStateOff;
    itMainSetStatus(item_gobj, sITRemixBoomerangStatusDescs, nITRemixBoomerangStatusPickup);
}

/* Remix _check_catch: asks the owner fighter to catch the Boomerang, when the owner is a fighter. */
static void itRemixBoomerangTryCatchOwner(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    GObj *owner_gobj = ip->owner_gobj;

    if (owner_gobj == NULL)
    {
        return;
    }
    if (owner_gobj->id != nGCCommonKindFighter)
    {
        return;
    }
    itRemixBoomerangHandleCatch(item_gobj, owner_gobj);
}

/* Remix handle_item_catch_: checks the catch range, the fighter's hold state and action, then picks up the Boomerang. */
static sb32 itRemixBoomerangHandleCatch(GObj *item_gobj, GObj *fighter_gobj)
{
    DObj *item_joint = DObjGetStruct(item_gobj);
    DObj *fighter_joint = DObjGetStruct(fighter_gobj);
    FTStruct *fp = ftGetStruct(fighter_gobj);
#ifdef REGION_REMIX
    f32 size_multiplier = ftRemixSizeGetMultiplier(fp->player); /* Size.multiplier_table[fp->player]; REMIX-TODO(Size) resolved */
#else
    f32 size_multiplier = 1.0F; /* REMIX-TODO(Size): 1.0F is the unscaled value outside REGION_REMIX. */
#endif

    if (!itRemixBoomerangIsInCatchRange(item_joint, fighter_joint, size_multiplier))
    {
        return FALSE;
    }
    if ((fp->item_gobj != NULL) || (fp->catch_gobj != NULL))
    {
        return FALSE;
    }
    if (!itRemixBoomerangIsCatchableAction(fp))
    {
        return FALSE;
    }

    itRemixBoomerangHoldProc(item_gobj);
    itMainSetFighterHold(item_gobj, fighter_gobj);
    return TRUE;
}

/* Remix handle_item_catch_ range check: horizontal |dx| within range, and the item's y between the scaled lower and upper bounds. */
static sb32 itRemixBoomerangIsInCatchRange(DObj *item_joint, DObj *fighter_joint, f32 size_multiplier)
{
    f32 dx = fighter_joint->translate.vec.f.x - item_joint->translate.vec.f.x;
    f32 item_y = item_joint->translate.vec.f.y;
    f32 lower_y = (ITREMIX_BOOMERANG_CATCH_RANGE_LY * size_multiplier) + fighter_joint->translate.vec.f.y;
    f32 upper_y = (ITREMIX_BOOMERANG_CATCH_RANGE_UY * size_multiplier) + fighter_joint->translate.vec.f.y;

    if (!(itRemixBoomerangAbs(dx) <= ITREMIX_BOOMERANG_CATCH_RANGE_X))
    {
        return FALSE;
    }
    return (lower_y <= item_y) && (item_y <= upper_y);
}

/* Remix handle_item_catch_ action check: the fighter must be in an idle, landing, shield, taunt or jump-aerial state. */
static sb32 itRemixBoomerangIsCatchableAction(FTStruct *fp)
{
    s32 action = fp->status_id;

    if (action < nITRemixBoomerangActionIdle)
    {
        return FALSE;
    }
    if (action <= nITRemixBoomerangActionTeeterStart)
    {
        return TRUE;
    }
    if ((action == nITRemixBoomerangActionFallSpecial) || (action == nITRemixBoomerangActionLandingSpecial) || (action == nITRemixBoomerangActionCeilingBonk))
    {
        return TRUE;
    }
    if (action < nITRemixBoomerangActionClang)
    {
        return FALSE;
    }
    if (action <= nITRemixBoomerangActionCliffWait)
    {
        return TRUE;
    }
    if (action < nITRemixBoomerangActionItemThrowDash)
    {
        return FALSE;
    }
    if (action <= nITRemixBoomerangActionItemThrowAirSmashD)
    {
        return TRUE;
    }
    if (action < nITRemixBoomerangActionShieldOn)
    {
        return FALSE;
    }
    if (action <= nITRemixBoomerangActionShieldStun)
    {
        return TRUE;
    }
    if (action == nITRemixBoomerangActionTaunt)
    {
        return TRUE;
    }
    return fp->proc_interrupt == ftCommonJumpAerialProcInterrupt;
}

#endif
