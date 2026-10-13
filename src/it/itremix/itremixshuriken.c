#ifdef PORT

#include <it/item.h>
#include <it/itmain.h>
#include <it/itmap.h>
#include <it/itmanager.h>
#include <it/itvisuals.h>
#include <it/itcommon/ittomato.h>
#include <it/itcommon/itbombhei.h>
#include <ft/fighter.h>
#include <ft/ftdef.h>
#include <ef/efmanager.h>
#include <mp/mpprocess.h>
#include <reloc_data.h>
#include "it/itremix/itremix.h"
#include "it/itremix/itremixshuriken.h"

extern void *func_800269C0_275C0(u16 id);

#define ITREMIX_SHURIKEN_BASE_DAMAGE 2
#define ITREMIX_SHURIKEN_BKB 70
#define ITREMIX_SHURIKEN_KBG 50
#define ITREMIX_SHURIKEN_KB_ANGLE 150
#define ITREMIX_SHURIKEN_ANGLE_DOWN 270
#define ITREMIX_SHURIKEN_ANGLE_UP 90
#define ITREMIX_SHURIKEN_TIME_BETWEEN_HITS 4
#define ITREMIX_SHURIKEN_THROW_TIMER 50
#define ITREMIX_SHURIKEN_SLOW_TIMER 30
#define ITREMIX_SHURIKEN_SLOW_MULTIPLIER 0.9375F /* 0x3F700000 */
#define ITREMIX_SHURIKEN_HIT_MULTIPLIER 0.5F /* 0x3F000000 */
#define ITREMIX_SHURIKEN_FILE_OFFSET 0x90
#define ITREMIX_SHURIKEN_HIT_FGM 0x0106 /* sword hit hurt FGM */
#define ITREMIX_SHURIKEN_STAB_FGM 0x0107
#define ITREMIX_SHURIKEN_COLL_MASK 0x0C21
#define ITREMIX_SHURIKEN_SMOKE_FRAME_MASK 7

#define ITREMIX_SHURIKEN_STATUS_PREPICKUP_GROUND 0
#define ITREMIX_SHURIKEN_STATUS_PREPICKUP_AIR 1
#define ITREMIX_SHURIKEN_STATUS_PICKUP 2
#define ITREMIX_SHURIKEN_STATUS_THROWN 3
#define ITREMIX_SHURIKEN_STATUS_SLOW 4
#define ITREMIX_SHURIKEN_STATUS_FALL 5

typedef struct ITRemixShurikenVars
{
    s32 status_timer;          /* THROW_TIMER / SLOW_TIMER (Remix ITStruct 0x1CC) */
    s32 hit_refresh_timer;     /* Hitbox refresh timer (Remix ITStruct 0x1D0) */
} ITRemixShurikenVars;

_Static_assert(sizeof(ITRemixShurikenVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixShurikenVars must fit in ITStruct.item_vars");

static sb32 itRemixShurikenThrownProcUpdate(GObj *item_gobj);
static sb32 itRemixShurikenThrownProcMap(GObj *item_gobj);
static sb32 itRemixShurikenThrownProcHit(GObj *item_gobj);
static sb32 itRemixShurikenSlowProcUpdate(GObj *item_gobj);
static sb32 itRemixShurikenSlowProcMap(GObj *item_gobj);
static sb32 itRemixShurikenSlowProcHit(GObj *item_gobj);
static sb32 itRemixShurikenProcReflector(GObj *item_gobj);
static void itRemixShurikenReflectedSetStatus(GObj *item_gobj);
static void itRemixShurikenSlowSetStatus(GObj *item_gobj);
static void itRemixShurikenFallSetStatus(GObj *item_gobj);
static sb32 itRemixShurikenDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags);
static void itRemixShurikenPrepickupProc(GObj *item_gobj);

/* Shuriken.item_info_array */
static ITDesc sITRemixShurikenItemInfo =
{
    0x43,                                   /* Item.Shuriken.id */
    NULL,                                   /* p_file, set from the owner before each spawn */
    ITREMIX_SHURIKEN_FILE_OFFSET,           /* item footer offset in Marina file 8 */
    { nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 },
    nGMAttackStateOff,
    itTomatoFallProcUpdate,                 /* Maxim Tomato fall routines, as in the Remix table */
    itTomatoFallProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* Shuriken.item_state_table */
static ITStatusDesc sITRemixShurikenStatusDescs[] =
{
    /* STATE 0 - PREPICKUP - GROUNDED */
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

    /* STATE 1 - PREPICKUP - AERIAL */
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

    /* STATE 2 - PICKUP */
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

    /* STATE 3 - THROWN */
    {
        itRemixShurikenThrownProcUpdate,
        itRemixShurikenThrownProcMap,
        itRemixShurikenThrownProcHit,
        itRemixShurikenThrownProcHit,
        itMainCommonProcHop,
        itRemixShurikenThrownProcHit,
        itRemixShurikenProcReflector,
        NULL
    },

    /* STATE 4 - SLOWING */
    {
        itRemixShurikenSlowProcUpdate,
        itRemixShurikenSlowProcMap,
        itRemixShurikenSlowProcHit,
        itRemixShurikenSlowProcHit,
        itMainCommonProcHop,
        itRemixShurikenSlowProcHit,
        itRemixShurikenProcReflector,
        NULL
    },

    /* STATE 5 - FALLING */
    {
        itTomatoFallProcUpdate,
        itTomatoDroppedProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/** Views the item-specific state of the Shuriken, which lives in ITStruct.item_vars. */
static ITRemixShurikenVars* itRemixShurikenGetVars(ITStruct *ip)
{
    return (ITRemixShurikenVars*) &ip->item_vars;
}

/** Shuriken.stage_setting_ helper: sets the damage, knockback and hit sfx of a freshly spawned Shuriken. */
static void itRemixShurikenSetAttackDefaults(ITStruct *ip)
{
    ip->attack_coll.damage = ITREMIX_SHURIKEN_BASE_DAMAGE;
    ip->attack_coll.knockback_scale = ITREMIX_SHURIKEN_KBG;
    ip->attack_coll.knockback_base = ITREMIX_SHURIKEN_BKB;
    ip->attack_coll.angle = ITREMIX_SHURIKEN_KB_ANGLE;
    ip->attack_coll.fgm_id = ITREMIX_SHURIKEN_HIT_FGM;
}

/** Shuriken.stage_setting_: spawns the Shuriken owned by `parent_gobj`. */
GObj* itRemixShurikenMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc item_desc = sITRemixShurikenItemInfo;
    GObj *item_gobj;
    DObj *dobj;
    ITStruct *ip;
    Vec3f translate;

    item_desc.p_file = fp->data->p_file_special3;
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    dobj = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);
    translate = dobj->translate.vec.f;

    ip->multi = 0;
    itMainClearOwnerStats(item_gobj);
    gcAddXObjForDObjFixed(dobj, 0x2E, 0);
    dobj->translate.vec.f = translate;

    itRemixShurikenSetAttackDefaults(ip);

    ip->is_unused_item_bool = TRUE;
    dobj->rotate.vec.f.z = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/** Shuriken.throw_initial_ helper: knockback angle from the thrower's action (ItemThrowDash, or the low bits of the action id). */
static s32 itRemixShurikenGetThrowAngle(ITStruct *ip)
{
    FTStruct *fp = ftGetStruct(ip->owner_gobj);
    s32 action_id = fp->status_id;

    if (action_id == nFTCommonStatusLightThrowDash)
    {
        return ITREMIX_SHURIKEN_KB_ANGLE;
    }
    if ((action_id & 3) == 0)
    {
        return ITREMIX_SHURIKEN_ANGLE_DOWN;
    }
    if ((action_id & 2) != 0)
    {
        return ITREMIX_SHURIKEN_KB_ANGLE;
    }
    return ITREMIX_SHURIKEN_ANGLE_UP;
}

/** Shuriken.throw_initial_: THROW_ITEM. Sets the knockback angle and enters the thrown state. */
static void itRemixShurikenThrowSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixShurikenGetVars(ip)->status_timer = ITREMIX_SHURIKEN_THROW_TIMER;
    ip->attack_coll.angle = itRemixShurikenGetThrowAngle(ip);

    itMainSetStatus(item_gobj, sITRemixShurikenStatusDescs, ITREMIX_SHURIKEN_STATUS_THROWN);
}

/** Shuriken.reflected_initial_: restarts the thrown state after a reflect, keeping the knockback angle. */
static void itRemixShurikenReflectedSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixShurikenGetVars(ip)->status_timer = ITREMIX_SHURIKEN_THROW_TIMER;
    itMainSetStatus(item_gobj, sITRemixShurikenStatusDescs, ITREMIX_SHURIKEN_STATUS_THROWN);
}

/** Shuriken.slow_initial_: enters the slowing state. */
static void itRemixShurikenSlowSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixShurikenGetVars(ip)->status_timer = ITREMIX_SHURIKEN_SLOW_TIMER;
    itMainSetStatus(item_gobj, sITRemixShurikenStatusDescs, ITREMIX_SHURIKEN_STATUS_SLOW);
}

/** Shuriken.falling_initial_: disables the hitbox and hurtbox and enters the falling state. */
static void itRemixShurikenFallSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->attack_coll.attack_state = nGMAttackStateOff;
    ip->damage_coll.hitstatus = 0;
    itMainSetStatus(item_gobj, sITRemixShurikenStatusDescs, ITREMIX_SHURIKEN_STATUS_FALL);
}

/** Returns the absolute value of a float. */
static f32 itRemixShurikenAbs(f32 value)
{
    return (value < 0.0F) ? -value : value;
}

/** Shuriken.thrown_main_ helpers: zeroes the velocity component with the smaller magnitude, unless one component is already zero. */
static void itRemixShurikenZeroSmallerVelocity(ITStruct *ip)
{
    Vec3f *vel = &ip->physics.vel_air;

    if ((vel->x == 0.0F) || (vel->y == 0.0F))
    {
        return;
    }
    if (itRemixShurikenAbs(vel->x) > itRemixShurikenAbs(vel->y))
    {
        vel->y = 0.0F;
    }
    else
    {
        vel->x = 0.0F;
    }
}

/** Shuriken.thrown_main_ helper: dust effect on the Shuriken position, emitted every 8 thrown frames. */
static void itRemixShurikenMakeSmokeEffect(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);

    efManagerDustLightMakeEffect(&dobj->translate.vec.f, 1, 1.0F);
}

/** Shuriken.thrown_main_: THROW_ITEM update. Spins, emits smoke, then starts slowing when the timer ends. */
static sb32 itRemixShurikenThrownProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixShurikenVars *vars = itRemixShurikenGetVars(ip);

    if (vars->status_timer == ITREMIX_SHURIKEN_THROW_TIMER)
    {
        ip->is_thrown = FALSE;
        itRemixShurikenZeroSmallerVelocity(ip);
    }
    itVisualsUpdateSpin(item_gobj);

    if ((vars->status_timer & ITREMIX_SHURIKEN_SMOKE_FRAME_MASK) == 0)
    {
        itRemixShurikenMakeSmokeEffect(item_gobj);
    }
    vars->status_timer--;
    if (vars->status_timer == 0)
    {
        itRemixShurikenSlowSetStatus(item_gobj);
    }
    return FALSE;
}

/** Shuriken.slow_main_ helper: scales both velocity components by `multiplier`. */
static void itRemixShurikenScaleVelocity(ITStruct *ip, f32 multiplier)
{
    ip->physics.vel_air.x *= multiplier;
    ip->physics.vel_air.y *= multiplier;
}

/** Shuriken.slow_main_ helper: ticks the hitbox refresh timer and clears the hit records when it runs out. */
static void itRemixShurikenTickHitRefresh(ITStruct *ip, ITRemixShurikenVars *vars)
{
    if (vars->hit_refresh_timer == 0)
    {
        return;
    }
    vars->hit_refresh_timer--;
    if (vars->hit_refresh_timer == 0)
    {
        itMainClearAttackRecord(ip);
    }
}

/** Shuriken.slow_main_: slowing update. Applies drag and starts falling when the slow timer ends. */
static sb32 itRemixShurikenSlowProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixShurikenVars *vars = itRemixShurikenGetVars(ip);

    itVisualsUpdateSpin(item_gobj);
    itRemixShurikenScaleVelocity(ip, ITREMIX_SHURIKEN_SLOW_MULTIPLIER);
    itRemixShurikenTickHitRefresh(ip, vars);

    vars->status_timer--;
    if (vars->status_timer == 0)
    {
        itRemixShurikenFallSetStatus(item_gobj);
    }
    return FALSE;
}

/** Shuriken.throw_collision_: thrown map collision. Plays the stab sfx and destroys the Shuriken on contact. */
static sb32 itRemixShurikenThrownProcMap(GObj *item_gobj)
{
    if (itMapTestAllCollisionFlag(item_gobj, ITREMIX_SHURIKEN_COLL_MASK) == FALSE)
    {
        return FALSE;
    }
    func_800269C0_275C0(ITREMIX_SHURIKEN_STAB_FGM);

    return TRUE;
}

/** Shuriken.throw_collide_: thrown hit. Halves the velocity, arms the hit refresh timer and starts slowing. */
static sb32 itRemixShurikenThrownProcHit(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    /* Remix throw_collide_ writes the refresh timer through v1, which the routine never sets. The port writes it through the item vars (the evident intent); see notes. */
    itRemixShurikenGetVars(ip)->hit_refresh_timer = ITREMIX_SHURIKEN_TIME_BETWEEN_HITS;
    itRemixShurikenScaleVelocity(ip, ITREMIX_SHURIKEN_HIT_MULTIPLIER);
    itRemixShurikenSlowSetStatus(item_gobj);

    return FALSE;
}

/** Shuriken.slow_collide_: slowing hit. Halves the velocity and arms the hit refresh timer. */
static sb32 itRemixShurikenSlowProcHit(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixShurikenGetVars(ip)->hit_refresh_timer = ITREMIX_SHURIKEN_TIME_BETWEEN_HITS;
    itRemixShurikenScaleVelocity(ip, ITREMIX_SHURIKEN_HIT_MULTIPLIER);

    return FALSE;
}

/** Shuriken.detect_collision_: wall, ceiling and floor check for the slowing Shuriken. Remix copies the vanilla map callback and drops the floor-edge adjust and map sfx; see notes. */
static sb32 itRemixShurikenDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags)
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
    return (coll_data->mask_stat & coll_flags) != 0;
}

/** Shuriken.slow_collision_: slowing map collision. Stops both velocity components on contact. */
static sb32 itRemixShurikenSlowProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    sb32 is_collided = mpProcessUpdateMain(&ip->coll_data, itRemixShurikenDetectCollision, item_gobj, ITREMIX_SHURIKEN_COLL_MASK);

    if (is_collided != FALSE)
    {
        ip->physics.vel_air.x = 0.0F;
        ip->physics.vel_air.y = 0.0F;
    }
    return FALSE;
}

/** Shuriken.reflect_: reflector hit. Runs the common reflect and restarts the thrown state. */
static sb32 itRemixShurikenProcReflector(GObj *item_gobj)
{
    itMainCommonProcReflector(item_gobj);
    itRemixShurikenReflectedSetStatus(item_gobj);

    return FALSE;
}

/** Shuriken.prepickup_: PICKUP_ITEM_INIT. Disables the hurtbox and enters the picked-up state. */
static void itRemixShurikenPrepickupProc(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixShurikenStatusDescs, ITREMIX_SHURIKEN_STATUS_PICKUP);
}

/** Descriptor read by the vanilla dispatch in itmain.c, itmanager.c and ftcommonget.c. */
const ITRemixItemDesc dITRemixShurikenItemDesc =
{
    0x43,                                   /* Item.Shuriken.id */
    itRemixShurikenMakeItem,                /* SPAWN_ITEM */
    TRUE,                                   /* SHOW_GFX_WHEN_SPAWNED */
    itRemixShurikenPrepickupProc,           /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,               /* PICKUP_ITEM_MAIN: pickup_shuriken jumps to 0x80145C4C */
    itTomatoDroppedSetStatus,               /* DROP_ITEM (0x801745FC, same routine as Maxim Tomato) */
    itRemixShurikenThrowSetStatus,          /* THROW_ITEM */
    NULL                                    /* PLAYER_COLLISION */
};

#endif
