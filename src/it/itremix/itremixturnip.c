#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixturnip.h"
#include <ft/fighter.h>
#include <it/item.h>
#include <it/itcommon/ittomato.h>
#include <it/itcommon/itbombhei.h>
#include <it/itmain.h>
#include <it/itmanager.h>
#include <it/itmap.h>
#include <it/itvisuals.h>
#include <if/ifcommon.h>
#include <sys/utils.h>

#define TURNIP_KB_GROWTH 60
#define TURNIP_BASE_KNOCKBACK 25
#define TURNIP_KB_ANGLE 361
#define TURNIP_HIT_FGM 0x0038
#define TURNIP_GROUND_TIMER 0x030C
#define TURNIP_THROW_TIMER 50
#define TURNIP_COLLIDE_FLAGS 0x0C21
#define TURNIP_SPAWN_IMAGE_INDEX 5

#define TURNIP_THROWN_GRAVITY 1.7546875476837158F /* 0x3FE0999A */
#define TURNIP_THROWN_TVEL 100.0F                  /* 0x42C80000 */
#define TURNIP_HIT_MULTIPLIER -0.125F              /* 0xBE000000 */
#define TURNIP_HIT_Y_VELOCITY 40.0F                /* 0x42200000 */

#define TURNIP_FACE_ROLL_RANGE 58
#define TURNIP_STITCH_ROLL 52
#define TURNIP_DOT_ROLL 53

enum ITRemixTurnipStatus
{
    nITRemixTurnipStatusGround,
    nITRemixTurnipStatusAir,
    nITRemixTurnipStatusPickup,
    nITRemixTurnipStatusThrown,
    nITRemixTurnipStatusFall,
    nITRemixTurnipStatusCount
};

typedef struct ITRemixTurnipVars
{
    s32 throw_timer;
    s32 hitbox_refresh_timer;
    sb32 is_hitbox_collision;
} ITRemixTurnipVars;

_Static_assert(sizeof(ITRemixTurnipVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixTurnipVars must fit in ITStruct.item_vars");

typedef struct ITRemixTurnipFaceDesc
{
    s32 damage;
    u16 image_index;
} ITRemixTurnipFaceDesc;

static ITStatusDesc sITRemixTurnipStatusDescs[nITRemixTurnipStatusCount];

/* Remix turnip_vars_: the Turnip's state lives in item_vars. */
static ITRemixTurnipVars* itRemixTurnipGetVars(ITStruct *ip)
{
    return (ITRemixTurnipVars*)&ip->item_vars;
}

/* Remix throw_initial_: enables the hurtbox and enters the thrown state. */
static void itRemixTurnipThrownProc(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    itMainSetStatus(item_gobj, sITRemixTurnipStatusDescs, nITRemixTurnipStatusThrown);
}

/* Remix reflected_initial_: a reflected Turnip gets a fresh throw timer, then enters the thrown state. */
static void itRemixTurnipReflectedSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixTurnipGetVars(ip)->throw_timer = TURNIP_THROW_TIMER;
    itMainSetStatus(item_gobj, sITRemixTurnipStatusDescs, nITRemixTurnipStatusThrown);
}

/* Remix falling_initial_: used for drop and for a Turnip that hit something. */
static void itRemixTurnipDroppedProc(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    /* REMIX-TODO(ITStruct): the asm also clears the "don't randomly despawn" bit at N64 0x2CE (mask 0x02). The decomp stores it in pickup_wait, which the next line overwrites, so the result is the same. */
    ip->pickup_wait = TURNIP_GROUND_TIMER;
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    itMainSetStatus(item_gobj, sITRemixTurnipStatusDescs, nITRemixTurnipStatusFall);
}

/* Remix thrown_main_: spins, counts down the throw timer and applies gravity. */
static sb32 itRemixTurnipThrownProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixTurnipVars *vars = itRemixTurnipGetVars(ip);

    itVisualsUpdateSpin(item_gobj);
    vars->throw_timer -= 1;
    itMainApplyGravityClampTVel(ip, TURNIP_THROWN_GRAVITY, TURNIP_THROWN_TVEL);
    itVisualsUpdateSpin(item_gobj);

    return FALSE;
}

/* Remix throw_collision_: destroys the Turnip when it hits a map collision with the flags below. */
static sb32 itRemixTurnipThrownProcMap(GObj *item_gobj)
{
    return itMapTestAllCollisionFlag(item_gobj, TURNIP_COLLIDE_FLAGS) != FALSE;
}

/* Remix throw_collide_ helper: damps the x velocity and sets the bounce y velocity. */
static void itRemixTurnipApplyHitVelocity(ITStruct *ip)
{
    ip->physics.vel_air.x = ip->physics.vel_air.x * TURNIP_HIT_MULTIPLIER;
    ip->physics.vel_air.y = TURNIP_HIT_Y_VELOCITY;
}

/* Remix throw_collide_: a hit, shield or clang disables both boxes and falls. */
static sb32 itRemixTurnipThrownCollide(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    itRemixTurnipApplyHitVelocity(ip);
    itRemixTurnipDroppedProc(item_gobj);

    return FALSE;
}

/* Remix throw_collide_hitbox_: a hurtbox hit damps and falls; the hitbox stays active. */
static sb32 itRemixTurnipThrownCollideHitbox(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixTurnipApplyHitVelocity(ip);
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    itRemixTurnipDroppedProc(item_gobj);

    return FALSE;
}

/* Remix reflect_: the generic reflect routine, then the reflected state. */
static sb32 itRemixTurnipReflectorProc(GObj *item_gobj)
{
    itMainCommonProcReflector(item_gobj);
    itRemixTurnipReflectedSetStatus(item_gobj);

    return FALSE;
}

/* Remix prepickup_: the pre-pickup routine disables the hurtbox and enters the pickup state. */
static void itRemixTurnipHoldProc(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixTurnipStatusDescs, nITRemixTurnipStatusPickup);
}

/* Turnip.asm stage_setting_ face roll: 0-51 normal, 52 stitch, 53 dot, 54-57 wink. */
static ITRemixTurnipFace itRemixTurnipRollFace(void)
{
    s32 roll = syUtilsRandIntRange(TURNIP_FACE_ROLL_RANGE);

    if (roll < TURNIP_STITCH_ROLL)
    {
        return nITRemixTurnipFaceNormal;
    }
    if (roll == TURNIP_STITCH_ROLL)
    {
        return nITRemixTurnipFaceStitch;
    }
    if (roll == TURNIP_DOT_ROLL)
    {
        return nITRemixTurnipFaceDot;
    }
    return nITRemixTurnipFaceWink;
}

static const ITRemixTurnipFaceDesc sITRemixTurnipFaceDescs[nITRemixTurnipFaceCount] =
{
    { 2, 6 },   /* normal */
    { 6, 3 },   /* wink */
    { 12, 5 },  /* dot */
    { 30, 4 },  /* stitch */
};

/* Remix _set_turnip: sets the base damage and the face image of the Turnip. */
void itRemixTurnipSetFace(GObj *item_gobj, ITRemixTurnipFace face)
{
    ITStruct *ip = itGetStruct(item_gobj);
    const ITRemixTurnipFaceDesc *desc = &sITRemixTurnipFaceDescs[face];

    DObjGetStruct(item_gobj)->mobj->texture_id_curr = desc->image_index;
    ip->attack_coll.damage = desc->damage;
}

/* Remix stage_setting_ init of the Turnip's own vars: rotation direction and timers start at zero. */
static void itRemixTurnipResetVars(ITStruct *ip)
{
    ITRemixTurnipVars *vars = itRemixTurnipGetVars(ip);

    vars->throw_timer = 0;
    vars->hitbox_refresh_timer = 0;
    vars->is_hitbox_collision = FALSE;
}

static ITDesc sITRemixTurnipItemDesc =
{
    0x4B,                                       /* Item.Turnip.id */
    NULL,                                       /* p_file: set per owner in itRemixTurnipMakeItem */
    0x00000040,                                 /* item footer offset (asm 0x08) */
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

/* Remix stage_setting_: spawns the Turnip for the owner's file 7, sets its face and attack values. */
GObj* itRemixTurnipMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc item_desc = sITRemixTurnipItemDesc;
    GObj *item_gobj;
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    item_desc.p_file = fp->data->p_file_special2;
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }

    joint = DObjGetStruct(item_gobj);
    joint->mobj->texture_id_curr = TURNIP_SPAWN_IMAGE_INDEX;
    translate = joint->translate.vec.f;
    itMainClearOwnerStats(item_gobj);
    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = translate;

    ip = itGetStruct(item_gobj);
    itRemixTurnipResetVars(ip);
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    ip->attack_coll.knockback_scale = TURNIP_KB_GROWTH;
    ip->attack_coll.knockback_base = TURNIP_BASE_KNOCKBACK;
    ip->attack_coll.angle = TURNIP_KB_ANGLE;
    ip->attack_coll.fgm_id = TURNIP_HIT_FGM;
    itRemixTurnipSetFace(item_gobj, itRemixTurnipRollFace());
    /* REMIX-TODO(ITStruct): the asm also sets N64 0x398 (blast-wall routine slot), 0x33E (bomb flag) and 0x2D3 (bit 0x04). None of them has a named ITStruct field, so they are not written. */
    joint->rotate.vec.f.z = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

static ITStatusDesc sITRemixTurnipStatusDescs[nITRemixTurnipStatusCount] =
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
        itRemixTurnipThrownProcUpdate,
        itRemixTurnipThrownProcMap,
        itRemixTurnipThrownCollide,
        itRemixTurnipThrownCollide,
        itMainCommonProcHop,
        itRemixTurnipThrownCollide,
        itRemixTurnipReflectorProc,
        itRemixTurnipThrownCollideHitbox
    },

    /* Falling (Remix status 4) */
    {
        itTomatoFallProcUpdate,
        itTomatoFallProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

const ITRemixItemDesc dITRemixTurnipItemDesc =
{
    0x4B,                       /* Item.Turnip.id */
    itRemixTurnipMakeItem,      /* SPAWN_ITEM */
    TRUE,                       /* SHOW_GFX_WHEN_SPAWNED */
    itRemixTurnipHoldProc,      /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,   /* PICKUP_ITEM_MAIN: pickup_ jumps to 0x80145C4C */
    itRemixTurnipDroppedProc,   /* DROP_ITEM */
    itRemixTurnipThrownProc,    /* THROW_ITEM */
    NULL                        /* PLAYER_COLLISION */
};

#endif
