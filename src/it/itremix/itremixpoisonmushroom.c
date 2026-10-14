/*
 * itremixpoisonmushroom.c - C port of the Smash Remix Poison Mushroom
 * (.toolchain/pkgs/smashremix-src/src/items/PoisonMushroom.asm).
 *
 * The asm reuses SuperMushroom.asm for the spawn, the item routines and the
 * player collision. The routines below are a local copy restricted to the
 * Poison Mushroom. Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixpoisonmushroom.h"
#include "it/item.h"
#include "it/itmain.h"
#include "it/itmap.h"
#include "it/itvisuals.h"
#include "it/itvars.h"
#include "it/itcommon/itstar.h"
#include "mp/mpdef.h"
#include "gm/gmdef.h"
#include "gm/gmcamera.h"
#include "gm/gmrumble.h"
#include "ft/fighter.h"
#include "ft/ftparam.h"
#include "sys/obj.h"
#include "sys/objman.h"
#include "remix/remix_roster_data.h"

#define ITREMIX_POISON_MUSHROOM_KIND 0x2F
#define ITREMIX_POISON_MUSHROOM_FILE_OFFSET 0xDE0 /* asm FILE_OFFSET: item footer in file 0xFB */
#define ITREMIX_POISON_MUSHROOM_PLAYERS_NUM 4
#define ITREMIX_POISON_MUSHROOM_INTANGIBLE_FRAMES 0x20 /* asm sets multi to 0x20; vanilla Star uses ITSTAR_INTERACT_DELAY (16) */
#define ITREMIX_POISON_MUSHROOM_SPAWN_BASE_SPEED 16.0F /* asm: t8 = 0x10 + 2 * spawn count */
#define ITREMIX_POISON_MUSHROOM_SPAWN_SPEED_STEP 2
#define ITREMIX_POISON_MUSHROOM_REVERT 600
#define ITREMIX_POISON_MUSHROOM_SHRINK_DURATION 48
#define ITREMIX_POISON_MUSHROOM_GROW_DURATION 57
#define ITREMIX_POISON_MUSHROOM_MAINTAIN_DURATION (ITREMIX_POISON_MUSHROOM_REVERT - ITREMIX_POISON_MUSHROOM_GROW_DURATION - ITREMIX_POISON_MUSHROOM_SHRINK_DURATION)
#define ITREMIX_POISON_MUSHROOM_GROW_START_TIMER (ITREMIX_POISON_MUSHROOM_SHRINK_DURATION + ITREMIX_POISON_MUSHROOM_MAINTAIN_DURATION)
#define ITREMIX_POISON_MUSHROOM_DEAD_STATUS_MAX 4 /* asm: status below 4 means the fighter just died */
#define ITREMIX_POISON_MUSHROOM_FLASH_COLANIM 5 /* asm Global.flash_ a1 = 5, quick bright flash */
#define ITREMIX_POISON_MUSHROOM_SHRINK_FGM 0xD5 /* asm pickup flash sound for the Poison Mushroom */
#define ITREMIX_POISON_MUSHROOM_CRASH_FGM 0x5BF /* asm FGM.play for Crash pickups */
#define ITREMIX_POISON_MUSHROOM_RUMBLE_ID 7
#define ITREMIX_POISON_MUSHROOM_RUMBLE_TIMER 60
#define ITREMIX_POISON_MUSHROOM_PROC_PRIORITY 6 /* unverified against Remix Render order 0x8000 */

enum itRemixPoisonMushroomState
{
    nITRemixPoisonMushroomStateShrink,
    nITRemixPoisonMushroomStateMaintain,
    nITRemixPoisonMushroomStateGrow
};

/* Remix item-footer state. The asm grounded flag lives in the item GObj at 0x40; here it is in item_vars. */
typedef struct ITRemixPoisonMushroomVars
{
    sb32 is_grounded;
} ITRemixPoisonMushroomVars;

_Static_assert(sizeof(ITRemixPoisonMushroomVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixPoisonMushroomVars must fit in ITStruct.item_vars");

/* One active effect per player. The asm uses one routine object per mushroom, with player_shrooms[] as the active one. */
typedef struct ITRemixPoisonMushroomEffect
{
    sb32 is_active;
    GObj *fighter_gobj;
    GObjProcess *process;
    s32 timer;
    s32 state;
} ITRemixPoisonMushroomEffect;

extern void* func_800269C0_275C0(u16 id);

static ITRemixPoisonMushroomVars* itRemixPoisonMushroomGetVars(GObj *item_gobj);
static void itRemixPoisonMushroomSetSpawnVel(Vec3f *spawn_vel, Vec3f *pos, f32 camera_at_x);
static sb32 itRemixPoisonMushroomProcUpdate(GObj *item_gobj);
static sb32 itRemixPoisonMushroomProcMap(GObj *item_gobj);
static void itRemixPoisonMushroomGroundToAir(GObj *item_gobj);
static void itRemixPoisonMushroomCollide(FTStruct *fp, ITStruct *ip, GObj *item_gobj);
static void itRemixPoisonMushroomStartEffect(FTStruct *fp);
static void itRemixPoisonMushroomEndEffect(ITRemixPoisonMushroomEffect *effect);
static void itRemixPoisonMushroomProcEffect(GObj *fighter_gobj);
static void itRemixPoisonMushroomTick(FTStruct *fp, ITRemixPoisonMushroomEffect *effect);
static void itRemixPoisonMushroomStartFeedback(FTStruct *fp);
static void itRemixPoisonMushroomUpdateState(FTStruct *fp, ITRemixPoisonMushroomEffect *effect);
static void itRemixPoisonMushroomTransition(FTStruct *fp, ITRemixPoisonMushroomEffect *effect, s32 frame, s32 duration);
static void itRemixPoisonMushroomRumble(FTStruct *fp);

/* Count of Poison Mushrooms spawned this frame. Reset by ProcUpdate. The asm shares it with SuperMushroom. */
static s32 sITRemixPoisonMushroomSpawnCount;

static ITRemixPoisonMushroomEffect sITRemixPoisonMushroomEffects[ITREMIX_POISON_MUSHROOM_PLAYERS_NUM];

/* asm item_info_array. Standard item: the file is the common item file 0xFB. */
static ITDesc sITRemixPoisonMushroomDesc =
{
    ITREMIX_POISON_MUSHROOM_KIND,
    &gITManagerCommonData,
    ITREMIX_POISON_MUSHROOM_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixPoisonMushroomProcUpdate,
    itRemixPoisonMushroomProcMap,
    itStarCommonProcHit,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/** PoisonMushroom.movement_routine_no_bounce_ (ITDesc proc_update): Star movement without gravity while on the floor. */
static sb32 itRemixPoisonMushroomProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    sITRemixPoisonMushroomSpawnCount = 0;
    if ((ip->coll_data.mask_stat & MAP_FLAG_FLOOR) == 0)
    {
        itMainApplyGravityClampTVel(ip, ITSTAR_GRAVITY, ITSTAR_TVEL);
    }
    ip->multi--;
    if (ip->multi == 0)
    {
        itMainRefreshAttackColl(item_gobj);
    }
    itVisualsUpdateSpin(item_gobj);

    return FALSE;
}

/** PoisonMushroom.ground_collision_no_bounce_ (ITDesc proc_map): Star map collision that stops the bounce. */
static sb32 itRemixPoisonMushroomProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixPoisonMushroomVars *vars = itRemixPoisonMushroomGetVars(item_gobj);
    sb32 is_collide_floor;

    if (vars->is_grounded != FALSE)
    {
        is_collide_floor = itMapCheckLRWallProcNoFloor(item_gobj, itRemixPoisonMushroomGroundToAir);
    }
    else
    {
        is_collide_floor = itMapTestAllCollisionFlag(item_gobj, MAP_FLAG_FLOOR);
    }
    vars->is_grounded = is_collide_floor;

    if (itMapCheckCollideAllRebound(item_gobj, (MAP_FLAG_CEIL | MAP_FLAG_RWALL | MAP_FLAG_LWALL), 1.0F, NULL) != FALSE)
    {
        itMainSetSpinVelLR(item_gobj);
    }
    if (is_collide_floor != FALSE)
    {
        ip->physics.vel_air.y = 0.0F;
    }
    return FALSE;
}

/** PoisonMushroom.handle_ground_to_air_transition_: empty in Remix. */
static void itRemixPoisonMushroomGroundToAir(GObj *item_gobj)
{
}

/** PoisonMushroom.spawn_custom_item_based_on_star_ (SPAWN_ITEM): Star spawn with a Remix speed per spawn this frame. */
GObj* itRemixPoisonMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    CObj *cobj = CObjGetStruct(gGMCameraGObj);
    Vec3f spawn_vel;
    GObj *item_gobj;
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    sITRemixPoisonMushroomSpawnCount++;
    itRemixPoisonMushroomSetSpawnVel(&spawn_vel, pos, cobj->vec.at.x);

    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixPoisonMushroomDesc, pos, &spawn_vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    joint = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    ip->attack_coll.interact_mask = GMHITCOLLISION_FLAG_FIGHTER;
    ip->multi = ITREMIX_POISON_MUSHROOM_INTANGIBLE_FRAMES;
    ip->is_unused_item_bool = TRUE;
    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = translate;
    joint->rotate.vec.f.z = 0.0F;
    itRemixPoisonMushroomGetVars(item_gobj)->is_grounded = FALSE;

    return item_gobj;
}

/** Spawn velocity of the asm: speed 16 + 2 per spawn this frame; direction flips on even spawn counts. */
static void itRemixPoisonMushroomSetSpawnVel(Vec3f *spawn_vel, Vec3f *pos, f32 camera_at_x)
{
    f32 speed = ITREMIX_POISON_MUSHROOM_SPAWN_BASE_SPEED + (f32) (ITREMIX_POISON_MUSHROOM_SPAWN_SPEED_STEP * sITRemixPoisonMushroomSpawnCount);

    spawn_vel->x = (pos->x < camera_at_x) ? speed : -speed;
    if ((sITRemixPoisonMushroomSpawnCount & 1) == 0)
    {
        spawn_vel->x = -spawn_vel->x;
    }
    spawn_vel->y = speed;
    spawn_vel->z = 0.0F;
}

/** PoisonMushroom.collide_mushroom_ (PLAYER_COLLISION): starts the effect on the touching fighter. */
static void itRemixPoisonMushroomCollide(FTStruct *fp, ITStruct *ip, GObj *item_gobj)
{
    ip->attack_coll.attack_state = nGMAttackStateOff;

    if (fp->fkind == REMIX_ID_CRASH)
    {
        func_800269C0_275C0(ITREMIX_POISON_MUSHROOM_CRASH_FGM);
    }
    itRemixPoisonMushroomStartEffect(fp);
}

/**
 * Starts the effect for fp's player. A pickup while the same fighter is
 * already affected restarts the effect in the maintain state, as the asm's
 * same-item branch does. The asm branch for a different mushroom kind is not
 * ported; see the notes file.
 */
static void itRemixPoisonMushroomStartEffect(FTStruct *fp)
{
    ITRemixPoisonMushroomEffect *effect = &sITRemixPoisonMushroomEffects[fp->player];
    sb32 is_refresh = (effect->is_active != FALSE) && (effect->fighter_gobj == fp->fighter_gobj);

    if (is_refresh != FALSE)
    {
        itRemixPoisonMushroomEndEffect(effect);
    }
    effect->is_active = TRUE;
    effect->fighter_gobj = fp->fighter_gobj;
    effect->timer = 0;
    effect->state = is_refresh ? nITRemixPoisonMushroomStateMaintain : nITRemixPoisonMushroomStateShrink;
    effect->process = gcAddGObjProcess(fp->fighter_gobj, itRemixPoisonMushroomProcEffect, nGCProcessKindFunc, ITREMIX_POISON_MUSHROOM_PROC_PRIORITY);
}

/** Ends the effect and its process. A stale process (fighter gone) is dropped without ending it. */
static void itRemixPoisonMushroomEndEffect(ITRemixPoisonMushroomEffect *effect)
{
    gcEndGObjProcess(effect->process);
    effect->process = NULL;
    effect->fighter_gobj = NULL;
    effect->is_active = FALSE;
}

/** PoisonMushroom.handle_active_mushroom_ (per-frame process on the fighter). */
static void itRemixPoisonMushroomProcEffect(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITRemixPoisonMushroomEffect *effect = &sITRemixPoisonMushroomEffects[fp->player];

    if ((effect->is_active == FALSE) || (effect->fighter_gobj != fighter_gobj))
    {
        return;
    }
    itRemixPoisonMushroomTick(fp, effect);
}

/** One frame of the effect: the first frame flashes, then the timer runs and the state machine advances. */
static void itRemixPoisonMushroomTick(FTStruct *fp, ITRemixPoisonMushroomEffect *effect)
{
    if (effect->timer == 0)
    {
        itRemixPoisonMushroomStartFeedback(fp);
    }
    effect->timer++;

    if ((effect->timer >= ITREMIX_POISON_MUSHROOM_REVERT) || (fp->status_id < ITREMIX_POISON_MUSHROOM_DEAD_STATUS_MAX))
    {
        itRemixPoisonMushroomEndEffect(effect);
        return;
    }
    itRemixPoisonMushroomUpdateState(fp, effect);
}

/** PoisonMushroom pickup feedback of handle_active_mushroom_: the shrink sound and the screen flash. */
static void itRemixPoisonMushroomStartFeedback(FTStruct *fp)
{
    func_800269C0_275C0(ITREMIX_POISON_MUSHROOM_SHRINK_FGM);
    ftParamCheckSetFighterColAnimID(fp->fighter_gobj, ITREMIX_POISON_MUSHROOM_FLASH_COLANIM, 0);
}

/**
 * Poison branch of the asm state machine. Shrink runs first, then the
 * maintain state, then the grow transition, which is timed from the
 * transition-2 start. Size writes are REMIX-TODO(Size) and not made here.
 */
static void itRemixPoisonMushroomUpdateState(FTStruct *fp, ITRemixPoisonMushroomEffect *effect)
{
    switch (effect->state)
    {
    case nITRemixPoisonMushroomStateShrink:
        itRemixPoisonMushroomTransition(fp, effect, effect->timer, ITREMIX_POISON_MUSHROOM_SHRINK_DURATION);
        break;

    case nITRemixPoisonMushroomStateGrow:
        itRemixPoisonMushroomTransition(fp, effect, effect->timer - ITREMIX_POISON_MUSHROOM_GROW_START_TIMER, ITREMIX_POISON_MUSHROOM_GROW_DURATION);
        break;

    default:
        if (effect->timer == ITREMIX_POISON_MUSHROOM_GROW_START_TIMER)
        {
            effect->state = nITRemixPoisonMushroomStateGrow;
        }
        break;
    }
}

/**
 * Shared end test of the shrink and grow transitions. On the first frame the
 * rumble plays. On the last frame the state advances. The asm's transition
 * sound is skipped there, because the pickup flash already set its flag.
 */
static void itRemixPoisonMushroomTransition(FTStruct *fp, ITRemixPoisonMushroomEffect *effect, s32 frame, s32 duration)
{
    if (frame == 1)
    {
        itRemixPoisonMushroomRumble(fp);
    }
    if (frame == duration)
    {
        effect->state++;
    }
}

/**
 * Global.rumble_ of the asm, for human ports only. REMIX-TODO(rumble): the
 * asm then rewrites the rumble_id to 0x17 so an action change does not stop
 * the rumble; gmRumbleSetPlayerRumbleParams returns nothing, so that is not ported.
 */
static void itRemixPoisonMushroomRumble(FTStruct *fp)
{
    if (fp->pkind != nFTPlayerKindMan)
    {
        return;
    }
    gmRumbleSetPlayerRumbleParams(fp->player, ITREMIX_POISON_MUSHROOM_RUMBLE_ID, ITREMIX_POISON_MUSHROOM_RUMBLE_TIMER);
}

static ITRemixPoisonMushroomVars* itRemixPoisonMushroomGetVars(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    return (ITRemixPoisonMushroomVars*) &ip->item_vars;
}

/** Descriptor of Poison Mushroom (item id 0x2F) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixPoisonMushroomItemDesc =
{
    ITREMIX_POISON_MUSHROOM_KIND,       /* Item.PoisonMushroom.id */
    itRemixPoisonMushroomMakeItem,      /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT */
    NULL,                               /* PICKUP_ITEM_MAIN */
    NULL,                               /* DROP_ITEM */
    NULL,                               /* THROW_ITEM */
    itRemixPoisonMushroomCollide        /* PLAYER_COLLISION */
};

#endif
