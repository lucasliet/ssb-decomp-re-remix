#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixsupermushroom.h"
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixhooks.h"
#include <it/item.h>
#include <it/itmain.h>
#include <it/itmap.h>
#include <it/itvars.h>
#include <it/itcommon/itstar.h>
#include <ft/fighter.h>
#include <ft/ftparam.h>
#include <gm/gmdef.h>
#include <mp/mpdef.h>
#include <sc/scene.h>
#include <sys/objman.h>
#include <sys/objdef.h>

#define ITREMIX_SUPERMUSHROOM_FILE_OFFSET 0x0D90
#define ITREMIX_SUPERMUSHROOM_XOBJ_KIND 0x2E /* render routine shared with Star (asm a1 = 0x2E) */
#define ITREMIX_SUPERMUSHROOM_SPAWN_SPEED 16
#define ITREMIX_SUPERMUSHROOM_SPAWN_SPEED_STEP 2
#define ITREMIX_SUPERMUSHROOM_ACTIVE_DELAY 0x20 /* ITStruct.multi frames before the item is tangible; Star uses 16 */
#define ITREMIX_SUPERMUSHROOM_PORTS 4
#define ITREMIX_SUPERMUSHROOM_ROUTINE_POOL 16
#define ITREMIX_SUPERMUSHROOM_REVERT_FRAMES 600
#define ITREMIX_SUPERMUSHROOM_GROW_FRAMES 57
#define ITREMIX_SUPERMUSHROOM_SHRINK_FRAMES 48
#define ITREMIX_SUPERMUSHROOM_MAINTAIN_FRAMES (ITREMIX_SUPERMUSHROOM_REVERT_FRAMES - ITREMIX_SUPERMUSHROOM_GROW_FRAMES - ITREMIX_SUPERMUSHROOM_SHRINK_FRAMES)
#define ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS 12
#define ITREMIX_SUPERMUSHROOM_RUMBLE_ID 7
#define ITREMIX_SUPERMUSHROOM_RUMBLE_FRAMES 60
#define ITREMIX_SUPERMUSHROOM_FGM_GROW 0x00D4
#define ITREMIX_SUPERMUSHROOM_FGM_SHRINK 0x00D5
#define ITREMIX_SUPERMUSHROOM_FGM_CRASH 0x05BF
#define ITREMIX_SUPERMUSHROOM_FLASH_COLANIM 5 /* asm Global.flash_ a1 = 5, quick bright flash */
#define ITREMIX_SUPERMUSHROOM_SIZE_NORMAL 1.0F
#define ITREMIX_SUPERMUSHROOM_SIZE_GIANT 2.25F /* 0x40100000 */
#define ITREMIX_SUPERMUSHROOM_SIZE_TINY 0.5F /* 0x3F000000 */

enum itRemixSuperMushroomState
{
    nITRemixSuperMushroomStateTransition1,
    nITRemixSuperMushroomStateMaintain,
    nITRemixSuperMushroomStateTransition2,
};

/* Matches Size.state in Size.asm (NORMAL 0, GIANT 1, TINY 2). */
enum itRemixSuperMushroomSize
{
    nITRemixSuperMushroomSizeNormal,
    nITRemixSuperMushroomSizeGiant,
    nITRemixSuperMushroomSizeTiny,
};

/* Remix private state of the item, stored in the item_vars union. */
typedef struct ITRemixSuperMushroomVars
{
    s32 is_grounded;
} ITRemixSuperMushroomVars;

_Static_assert(sizeof(ITRemixSuperMushroomVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixSuperMushroomVars must fit in ITStruct.item_vars");

/* One size transition: per-step frame numbers and sizes, the length of the transition and its sfx. */
typedef struct ITRemixSuperMushroomTransition
{
    const u8 *frames;
    const f32 *sizes;
    s32 duration;
    s32 fgm_id;
} ITRemixSuperMushroomTransition;

/* The two transitions of one size setup, and the size held during MAINTAIN. */
typedef struct ITRemixSuperMushroomSetup
{
    const ITRemixSuperMushroomTransition *first;
    const ITRemixSuperMushroomTransition *second;
    f32 hold_size;
} ITRemixSuperMushroomSetup;

/* One active effect on a player. Created on pickup and freed when the effect ends or is superseded. */
typedef struct ITRemixSuperMushroomRoutine
{
    GObj *routine_gobj;             /* NULL while the pool slot is free */
    FTStruct *fp;
    s32 kind;                       /* Item.SuperMushroom.id or Item.PoisonMushroom.id */
    s32 timer;                      /* frames since the effect started, 0 before its first update */
    s32 transition_index;           /* next step of the size transition tables */
    s32 state;                      /* nITRemixSuperMushroomState* */
    s32 match_size_state;           /* nITRemixSuperMushroomSize* of the player when picked up */
    s32 flash_sfx_kind;             /* asm 0x60: item id whose sfx already played this frame, 0 if none */
    sb32 is_pending_sfx;            /* asm 0x64: superseded effect still owes the sfx of its item */
    f32 size_multiplier;            /* last value written to Size.multiplier_table for the player */
} ITRemixSuperMushroomRoutine;

static const u8 sITRemixSuperMushroomGrowFrames[ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS] = { 11, 16, 21, 26, 30, 34, 38, 42, 45, 48, 51, 54 };
static const u8 sITRemixSuperMushroomShrinkFrames[ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS] = { 4, 7, 10, 13, 16, 19, 22, 25, 28, 33, 38, 43 };

static const f32 sITRemixSuperMushroomGrowSizes[ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS] =
{
    1.2998046875F,  /* 0x3FA66000 */
    1.0F,           /* 0x3F800000 */
    1.2998046875F,  /* 0x3FA66000 */
    1.0F,           /* 0x3F800000 */
    1.2998046875F,  /* 0x3FA66000 */
    1.89990234375F, /* 0x3FF33000 */
    2.25F,          /* 0x40100000 */
    1.0F,           /* 0x3F800000 */
    1.2998046875F,  /* 0x3FA66000 */
    1.89990234375F, /* 0x3FF33000 */
    2.25F,          /* 0x40100000 */
    1.2998046875F   /* 0x3FA66000 */
};

static const f32 sITRemixSuperMushroomShrinkSizes[ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS] =
{
    1.2998046875F,  /* 0x3FA66000 */
    2.25F,          /* 0x40100000 */
    1.89990234375F, /* 0x3FF33000 */
    1.2998046875F,  /* 0x3FA66000 */
    1.0F,           /* 0x3F800000 */
    2.25F,          /* 0x40100000 */
    1.89990234375F, /* 0x3FF33000 */
    1.2998046875F,  /* 0x3FA66000 */
    1.0F,           /* 0x3F800000 */
    1.2998046875F,  /* 0x3FA66000 */
    1.0F,           /* 0x3F800000 */
    1.2998046875F   /* 0x3FA66000 */
};

static const f32 sITRemixSuperMushroomGrowPoisonSizes[ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS] =
{
    0.65F,  /* 0x3F266666 */
    0.5F,   /* 0x3F000000 */
    0.65F,  /* 0x3F266666 */
    0.5F,   /* 0x3F000000 */
    0.65F,  /* 0x3F266666 */
    0.9F,   /* 0x3F666666 */
    1.0F,   /* 0x3F800000 */
    0.5F,   /* 0x3F000000 */
    0.65F,  /* 0x3F266666 */
    0.9F,   /* 0x3F666666 */
    1.0F,   /* 0x3F800000 */
    0.65F   /* 0x3F266666 */
};

static const f32 sITRemixSuperMushroomShrinkPoisonSizes[ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS] =
{
    0.65F,  /* 0x3F266666 */
    1.0F,   /* 0x3F800000 */
    0.9F,   /* 0x3F666666 */
    0.65F,  /* 0x3F266666 */
    0.5F,   /* 0x3F000000 */
    1.0F,   /* 0x3F800000 */
    0.9F,   /* 0x3F666666 */
    0.65F,  /* 0x3F266666 */
    0.5F,   /* 0x3F000000 */
    0.65F,  /* 0x3F266666 */
    0.5F,   /* 0x3F000000 */
    0.65F   /* 0x3F266666 */
};

static const ITRemixSuperMushroomTransition sITRemixSuperMushroomGrow =
{
    sITRemixSuperMushroomGrowFrames, sITRemixSuperMushroomGrowSizes, ITREMIX_SUPERMUSHROOM_GROW_FRAMES, ITREMIX_SUPERMUSHROOM_FGM_GROW
};

static const ITRemixSuperMushroomTransition sITRemixSuperMushroomShrink =
{
    sITRemixSuperMushroomShrinkFrames, sITRemixSuperMushroomShrinkSizes, ITREMIX_SUPERMUSHROOM_SHRINK_FRAMES, ITREMIX_SUPERMUSHROOM_FGM_SHRINK
};

static const ITRemixSuperMushroomTransition sITRemixSuperMushroomGrowPoison =
{
    sITRemixSuperMushroomGrowFrames, sITRemixSuperMushroomGrowPoisonSizes, ITREMIX_SUPERMUSHROOM_GROW_FRAMES, ITREMIX_SUPERMUSHROOM_FGM_GROW
};

static const ITRemixSuperMushroomTransition sITRemixSuperMushroomShrinkPoison =
{
    sITRemixSuperMushroomShrinkFrames, sITRemixSuperMushroomShrinkPoisonSizes, ITREMIX_SUPERMUSHROOM_SHRINK_FRAMES, ITREMIX_SUPERMUSHROOM_FGM_SHRINK
};

/* Super Mushroom on a normal player: grow, hold giant, then shrink. */
static const ITRemixSuperMushroomSetup sITRemixSuperMushroomSuperNormal = { &sITRemixSuperMushroomGrow, &sITRemixSuperMushroomShrink, 2.25F };

/* Super Mushroom on a tiny player: grow back from poison sizes, hold normal. */
static const ITRemixSuperMushroomSetup sITRemixSuperMushroomSuperTiny = { &sITRemixSuperMushroomGrowPoison, &sITRemixSuperMushroomShrinkPoison, 1.0F };

/* Poison Mushroom on a giant player: shrink, hold normal. */
static const ITRemixSuperMushroomSetup sITRemixSuperMushroomPoisonGiant = { &sITRemixSuperMushroomShrink, &sITRemixSuperMushroomGrow, 1.0F };

/* Poison Mushroom on a normal player: shrink from poison sizes, hold tiny. */
static const ITRemixSuperMushroomSetup sITRemixSuperMushroomPoisonNormal = { &sITRemixSuperMushroomShrinkPoison, &sITRemixSuperMushroomGrowPoison, 0.5F };

static ITRemixSuperMushroomRoutine sITRemixSuperMushroomRoutines[ITREMIX_SUPERMUSHROOM_ROUTINE_POOL];
static ITRemixSuperMushroomRoutine *sITRemixSuperMushroomActive[ITREMIX_SUPERMUSHROOM_PORTS];  /* asm player_shrooms */
static s32 sITRemixSuperMushroomSpawnCount;                                                  /* asm spawning_shrooms */

extern void *func_800269C0_275C0(u16 id);

static void itRemixSuperMushroomProcUpdate(GObj *routine_gobj);
static void itRemixSuperMushroomFreeRoutine(ITRemixSuperMushroomRoutine *routine);

static ITDesc sITRemixSuperMushroomItemInfo =
{
    ITREMIX_SUPERMUSHROOM_KIND,             /* Item.SuperMushroom.id */
    &gITManagerCommonData,                  /* item file 0xFB, see notes REMIX-TODO(file) */
    ITREMIX_SUPERMUSHROOM_FILE_OFFSET,      /* item footer offset */

    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itRemixSuperMushroomMovementProcUpdate, /* movement_routine_no_bounce_ */
    itRemixSuperMushroomGroundProcMap,      /* ground_collision_no_bounce_ */
    itStarCommonProcHit,                    /* 0x80174A0C, same as Star */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* SuperMushroom: item vars, kept at the start of item_vars (only used by this item) */
static ITRemixSuperMushroomVars* itRemixSuperMushroomGetVars(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    return (ITRemixSuperMushroomVars*) &ip->item_vars;
}

/* SuperMushroom: spawn velocity of spawn_custom_item_based_on_star_. Shared with the Poison
 * Mushroom, whose SPAWN_ITEM is this same asm routine: horizontal speed grows with each mushroom
 * spawned this frame (spawning_shrooms) and alternates direction. */
void itRemixSuperMushroomGetSpawnVel(Vec3f *pos, Vec3f *vel)
{
    CObj *cobj = CObjGetStruct(gGMCameraGObj);
    f32 speed;

    sITRemixSuperMushroomSpawnCount += 1;
    speed = (f32) (ITREMIX_SUPERMUSHROOM_SPAWN_SPEED + (ITREMIX_SUPERMUSHROOM_SPAWN_SPEED_STEP * sITRemixSuperMushroomSpawnCount));
    vel->x = (pos->x < cobj->vec.at.x) ? speed : -speed;
    if ((sITRemixSuperMushroomSpawnCount % 2) == 0)
    {
        vel->x = -vel->x;
    }
    vel->y = speed;
    vel->z = 0.0F;
}

/* SuperMushroom: tail of spawn_custom_item_based_on_star_ after itManagerMakeItem (Star code with
 * the active delay and no gravity hold). Shared with the Poison Mushroom. The copied Star tail also
 * sets ITStruct byte 0x2D3 bit 0x4 (asm lines 98-100, ROM 0xEF51C), ported as is_unused_item_bool. */
void itRemixSuperMushroomSetupItem(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);
    Vec3f translate = dobj->translate.vec.f;

    ip->attack_coll.interact_mask = GMHITCOLLISION_FLAG_FIGHTER;
    ip->multi = ITREMIX_SUPERMUSHROOM_ACTIVE_DELAY;
    ip->is_unused_item_bool = TRUE;
    itRemixSuperMushroomGetVars(item_gobj)->is_grounded = FALSE;
    gcAddXObjForDObjFixed(dobj, ITREMIX_SUPERMUSHROOM_XOBJ_KIND, 0);
    dobj->rotate.vec.f.z = 0.0F;
    dobj->translate.vec.f = translate;
}

/* SuperMushroom: stage_setting_. Remix SPAWN_ITEM (a0 parent, a1 pos, a2 vel ignored, a3 flags). The asm builds its own velocity. */
GObj* itRemixSuperMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    Vec3f spawn_vel;
    GObj *item_gobj;

    itRemixSuperMushroomGetSpawnVel(pos, &spawn_vel);
    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixSuperMushroomItemInfo, pos, &spawn_vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixSuperMushroomSetupItem(item_gobj);

    return item_gobj;
}

/* SuperMushroom: movement_routine_no_bounce_ (ITDesc word 0x14 of both mushrooms). Star update
 * with gravity skipped while grounded (coll_data.mask_stat MAP_FLAG_FLOOR). Shared with the
 * Poison Mushroom (asm item_info word 0x14 of both items). */
sb32 itRemixSuperMushroomMovementProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    sITRemixSuperMushroomSpawnCount = 0;
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

/* SuperMushroom: handle_ground_to_air_transition_. Empty in the asm, so it is empty here too. */
static void itRemixSuperMushroomGroundToAirNoop(GObj *item_gobj)
{
}

/* SuperMushroom: ground_collision_no_bounce_ (ITDesc word 0x18 of both mushrooms). Star ground
 * collision with the floor bounce removed. Shared with the Poison Mushroom. */
sb32 itRemixSuperMushroomGroundProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixSuperMushroomVars *vars = itRemixSuperMushroomGetVars(item_gobj);
    sb32 is_grounded;

    if (vars->is_grounded != FALSE)
    {
        is_grounded = itMapCheckLRWallProcNoFloor(item_gobj, itRemixSuperMushroomGroundToAirNoop);
    }
    else
    {
        is_grounded = itMapTestAllCollisionFlag(item_gobj, MAP_FLAG_FLOOR);
    }
    vars->is_grounded = is_grounded;

    if (itMapCheckCollideAllRebound(item_gobj, (MAP_FLAG_CEIL | MAP_FLAG_RWALL | MAP_FLAG_LWALL), 1.0F, NULL) != FALSE)
    {
        itMainSetSpinVelLR(item_gobj);
    }
    if (is_grounded != FALSE)
    {
        ip->physics.vel_air.y = 0.0F;
    }

    return FALSE;
}

/* SuperMushroom: picks the size setup for the item kind and the player's match size state */
static const ITRemixSuperMushroomSetup* itRemixSuperMushroomGetSetup(const ITRemixSuperMushroomRoutine *routine)
{
    sb32 is_normal = (routine->match_size_state == nITRemixSuperMushroomSizeNormal);

    if (routine->kind == ITREMIX_SUPERMUSHROOM_POISON_KIND)
    {
        return is_normal ? &sITRemixSuperMushroomPoisonNormal : &sITRemixSuperMushroomPoisonGiant;
    }
    return is_normal ? &sITRemixSuperMushroomSuperNormal : &sITRemixSuperMushroomSuperTiny;
}

/* SuperMushroom: plays the sfx of an item kind (poison uses the shrink sound) */
static void itRemixSuperMushroomPlayItemSfx(s32 kind)
{
    if (kind == ITREMIX_SUPERMUSHROOM_POISON_KIND)
    {
        func_800269C0_275C0(ITREMIX_SUPERMUSHROOM_FGM_SHRINK);
        return;
    }
    func_800269C0_275C0(ITREMIX_SUPERMUSHROOM_FGM_GROW);
}

/* SuperMushroom: _flash. Plays the item sfx, marks it as played this frame and adds the quick
 * bright flash (asm Global.flash_, flash id 5). */
static void itRemixSuperMushroomFlash(ITRemixSuperMushroomRoutine *routine)
{
    routine->flash_sfx_kind = routine->kind;
    itRemixSuperMushroomPlayItemSfx(routine->kind);
    ftParamCheckSetFighterColAnimID(routine->fp->fighter_gobj, ITREMIX_SUPERMUSHROOM_FLASH_COLANIM, 0);
}

/* SuperMushroom: the transition sfx on the first step, unless the flash already played one, then the rumble */
static void itRemixSuperMushroomStartTransition(ITRemixSuperMushroomRoutine *routine, const ITRemixSuperMushroomTransition *transition)
{
    if (routine->flash_sfx_kind == 0)
    {
        func_800269C0_275C0(transition->fgm_id);
    }
    ftParamMakeRumble(routine->fp, ITREMIX_SUPERMUSHROOM_RUMBLE_ID, ITREMIX_SUPERMUSHROOM_RUMBLE_FRAMES);
}

/* SuperMushroom: writes the player's Size.multiplier_table entry. */
static void itRemixSuperMushroomSetSize(ITRemixSuperMushroomRoutine *routine, f32 size)
{
    routine->size_multiplier = size;
    ftRemixSizeSetMultiplier(routine->fp->player, size);
}

/* SuperMushroom: one transition step. Advances the state at the end and writes the size at each listed frame. */
static void itRemixSuperMushroomApplyTransition(ITRemixSuperMushroomRoutine *routine, const ITRemixSuperMushroomTransition *transition, s32 step_timer)
{
    if (step_timer == 1)
    {
        itRemixSuperMushroomStartTransition(routine, transition);
    }
    if (step_timer == transition->duration)
    {
        routine->state += 1;
    }
    if (transition->frames[routine->transition_index] != step_timer)
    {
        return;
    }
    itRemixSuperMushroomSetSize(routine, transition->sizes[routine->transition_index]);
    routine->transition_index = (routine->transition_index + 1) % ITREMIX_SUPERMUSHROOM_TRANSITION_STEPS;
}

/* SuperMushroom: size state machine for one frame. The second transition runs on a timer shifted by the first one's length. */
static void itRemixSuperMushroomUpdateSize(ITRemixSuperMushroomRoutine *routine)
{
    const ITRemixSuperMushroomSetup *setup = itRemixSuperMushroomGetSetup(routine);
    s32 step_timer = routine->timer;

    if (routine->state == nITRemixSuperMushroomStateTransition1)
    {
        itRemixSuperMushroomApplyTransition(routine, setup->first, step_timer);
        return;
    }
    if (routine->state == nITRemixSuperMushroomStateTransition2)
    {
        step_timer -= setup->first->duration + ITREMIX_SUPERMUSHROOM_MAINTAIN_FRAMES;
        itRemixSuperMushroomApplyTransition(routine, setup->second, step_timer);
        return;
    }
    if (step_timer == setup->first->duration + ITREMIX_SUPERMUSHROOM_MAINTAIN_FRAMES)
    {
        routine->state = nITRemixSuperMushroomStateTransition2;
    }
    itRemixSuperMushroomSetSize(routine, setup->hold_size);
}

/* SuperMushroom: size the player returns to when the effect ends */
static f32 itRemixSuperMushroomGetRestSize(s32 match_size_state)
{
    if (match_size_state == nITRemixSuperMushroomSizeGiant)
    {
        return ITREMIX_SUPERMUSHROOM_SIZE_GIANT;
    }
    if (match_size_state == nITRemixSuperMushroomSizeTiny)
    {
        return ITREMIX_SUPERMUSHROOM_SIZE_TINY;
    }
    return ITREMIX_SUPERMUSHROOM_SIZE_NORMAL;
}

/* SuperMushroom: _revert. Ends the effect and frees the routine. */
static void itRemixSuperMushroomRevert(ITRemixSuperMushroomRoutine *routine)
{
    itRemixSuperMushroomSetSize(routine, itRemixSuperMushroomGetRestSize(routine->match_size_state));
    sITRemixSuperMushroomActive[routine->fp->player] = NULL;
    itRemixSuperMushroomFreeRoutine(routine);
}

/* SuperMushroom: _update_timer. Ends the effect on timeout or death, otherwise advances the size state machine. */
static void itRemixSuperMushroomAdvance(ITRemixSuperMushroomRoutine *routine)
{
    routine->timer += 1;
    if (routine->timer >= ITREMIX_SUPERMUSHROOM_REVERT_FRAMES)
    {
        itRemixSuperMushroomRevert(routine);
        return;
    }
    if (routine->fp->status_id <= nFTCommonStatusDeadUpFall)
    {
        itRemixSuperMushroomRevert(routine);
        return;
    }
    itRemixSuperMushroomUpdateSize(routine);
}

/* SuperMushroom: true when the pickup contradicts the player's size (giant + super, tiny + poison) during the first grow or shrink */
static sb32 itRemixSuperMushroomIsSizeMismatch(const ITRemixSuperMushroomRoutine *routine)
{
    if (routine->match_size_state == nITRemixSuperMushroomSizeNormal)
    {
        return FALSE;
    }
    if ((routine->match_size_state == nITRemixSuperMushroomSizeGiant) && (routine->kind == ITREMIX_SUPERMUSHROOM_POISON_KIND))
    {
        return FALSE;
    }
    if ((routine->match_size_state == nITRemixSuperMushroomSizeTiny) && (routine->kind != ITREMIX_SUPERMUSHROOM_POISON_KIND))
    {
        return FALSE;
    }
    return (routine->state == nITRemixSuperMushroomStateTransition1);
}

/* SuperMushroom: _check_match_size. A mismatched pickup cancels the effect on its first frame, then the flash plays. */
static void itRemixSuperMushroomCheckMatchSize(ITRemixSuperMushroomRoutine *routine)
{
    if (itRemixSuperMushroomIsSizeMismatch(routine))
    {
        routine->timer = ITREMIX_SUPERMUSHROOM_REVERT_FRAMES;
    }
    itRemixSuperMushroomFlash(routine);
}

/* SuperMushroom: pool slot allocation. Returns NULL when all slots are in use, so the pickup does nothing. */
static ITRemixSuperMushroomRoutine* itRemixSuperMushroomAllocRoutine(void)
{
    s32 index;

    for (index = 0; index < ITREMIX_SUPERMUSHROOM_ROUTINE_POOL; index++)
    {
        if (sITRemixSuperMushroomRoutines[index].routine_gobj == NULL)
        {
            return &sITRemixSuperMushroomRoutines[index];
        }
    }
    return NULL;
}

/* SuperMushroom: ejects the routine GObj and frees its pool slot */
static void itRemixSuperMushroomFreeRoutine(ITRemixSuperMushroomRoutine *routine)
{
    gcEjectGObj(routine->routine_gobj);
    routine->routine_gobj = NULL;
}

/* SuperMushroom: makes this routine the active effect of the player's port. The previous effect of another kind
 * is kept and gets its timer moved to the start of its final transition. Returns FALSE when this routine must stop. */
static sb32 itRemixSuperMushroomStartRoutine(ITRemixSuperMushroomRoutine *routine)
{
    ITRemixSuperMushroomRoutine *prev = sITRemixSuperMushroomActive[routine->fp->player];

    if (prev == NULL)
    {
        sITRemixSuperMushroomActive[routine->fp->player] = routine;
        return TRUE;
    }
    if (routine->kind == prev->kind)
    {
        sITRemixSuperMushroomActive[routine->fp->player] = routine;
        routine->state = nITRemixSuperMushroomStateMaintain;
        itRemixSuperMushroomFreeRoutine(prev);
        return TRUE;
    }
    sITRemixSuperMushroomActive[routine->fp->player] = prev;
    if (prev->state == nITRemixSuperMushroomStateTransition2)
    {
        routine->is_pending_sfx = TRUE;
    }
    else
    {
        prev->timer = ITREMIX_SUPERMUSHROOM_GROW_FRAMES + ITREMIX_SUPERMUSHROOM_MAINTAIN_FRAMES - 1;
        if (routine->kind == ITREMIX_SUPERMUSHROOM_POISON_KIND)
        {
            prev->timer += ITREMIX_SUPERMUSHROOM_SHRINK_FRAMES - ITREMIX_SUPERMUSHROOM_GROW_FRAMES;
        }
        prev->transition_index = 0;
        prev->state = nITRemixSuperMushroomStateMaintain;
    }
    if (routine->is_pending_sfx != FALSE)
    {
        itRemixSuperMushroomPlayItemSfx(routine->kind);
    }
    itRemixSuperMushroomFreeRoutine(routine);

    return FALSE;
}

/* SuperMushroom: handle_active_mushroom_. Runs every frame of one effect. */
static void itRemixSuperMushroomProcUpdate(GObj *routine_gobj)
{
    ITRemixSuperMushroomRoutine *routine = (ITRemixSuperMushroomRoutine*) routine_gobj->user_data.p;

    routine->flash_sfx_kind = 0;
    if (routine->timer == 0)
    {
        if (itRemixSuperMushroomStartRoutine(routine) == FALSE)
        {
            return;
        }
        itRemixSuperMushroomCheckMatchSize(routine);
    }
    itRemixSuperMushroomAdvance(routine);
}

/* SuperMushroom: fills a new routine from the pickup, with the match size state of the player (Size.match_state_table) */
static void itRemixSuperMushroomInitRoutine(ITRemixSuperMushroomRoutine *routine, FTStruct *fp, s32 kind)
{
    routine->fp = fp;
    routine->kind = kind;
    routine->timer = 0;
    routine->transition_index = 0;
    routine->state = nITRemixSuperMushroomStateTransition1;
    routine->match_size_state = ftRemixSizeGetMatchState(fp->player);
    routine->flash_sfx_kind = 0;
    routine->is_pending_sfx = FALSE;
    routine->size_multiplier = ITREMIX_SUPERMUSHROOM_SIZE_NORMAL;
}

/* SuperMushroom: PLAYER_COLLISION (collide_mushroom_). Shared by both mushrooms (asm
 * PoisonMushroom.asm reuses this routine): dispatches on ip->kind, so a Poison pickup registers
 * into the same per-port slot and the cross-kind handling of handle_active_mushroom_ applies. */
void itRemixSuperMushroomProcTouch(FTStruct *fp, ITStruct *ip, GObj *item_gobj)
{
    ITRemixSuperMushroomRoutine *routine;
    GObj *routine_gobj;

    ip->attack_coll.attack_state = nGMAttackStateOff;
    if (fp->fkind == REMIX_ID_CRASH)
    {
        func_800269C0_275C0(ITREMIX_SUPERMUSHROOM_FGM_CRASH);
    }
    routine = itRemixSuperMushroomAllocRoutine();
    if (routine == NULL)
    {
        return;
    }
    itRemixSuperMushroomInitRoutine(routine, fp, ip->kind);
    routine_gobj = gcMakeGObjSPAfter(nGCCommonKindEffect, NULL, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT);
    routine_gobj->user_data.p = routine;
    routine->routine_gobj = routine_gobj;
    gcAddGObjProcess(routine_gobj, itRemixSuperMushroomProcUpdate, nGCProcessKindFunc, 3);
}

/* SuperMushroom: clear_active_mushrooms_. Forgets every effect of both mushroom kinds: the
 * routine pool, the per-port active table (asm player_shrooms) and the spawn count (asm
 * spawning_shrooms). The routine GObjs are freed with the scene. */
void itRemixSuperMushroomClearStates(void)
{
    s32 index;

    for (index = 0; index < ITREMIX_SUPERMUSHROOM_ROUTINE_POOL; index++)
    {
        sITRemixSuperMushroomRoutines[index].routine_gobj = NULL;
    }
    for (index = 0; index < ITREMIX_SUPERMUSHROOM_PORTS; index++)
    {
        sITRemixSuperMushroomActive[index] = NULL;
    }
    sITRemixSuperMushroomSpawnCount = 0;
}

sb32 itRemixSuperMushroomIsActive(s32 player)
{
    if ((player < 0) || (player >= ITREMIX_SUPERMUSHROOM_PORTS))
    {
        return FALSE;
    }
    return sITRemixSuperMushroomActive[player] != NULL;
}

void itRemixSuperMushroomDestroyForPlayer(s32 player)
{
    ITRemixSuperMushroomRoutine *routine;

    if (itRemixSuperMushroomIsActive(player) == FALSE)
    {
        return;
    }
    routine = sITRemixSuperMushroomActive[player];
    sITRemixSuperMushroomActive[player] = NULL;
    itRemixSuperMushroomFreeRoutine(routine);
}

const ITRemixItemDesc dITRemixSuperMushroomItemDesc =
{
    ITREMIX_SUPERMUSHROOM_KIND,
    itRemixSuperMushroomMakeItem,
    TRUE,
    NULL,                                   /* PICKUP_ITEM_INIT(0) */
    NULL,                                   /* PICKUP_ITEM_MAIN(0) */
    NULL,                                   /* DROP_ITEM(0) */
    NULL,                                   /* THROW_ITEM(0) */
    itRemixSuperMushroomProcTouch           /* PLAYER_COLLISION(collide_mushroom_) */
};

#endif
