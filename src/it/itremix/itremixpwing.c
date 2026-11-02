/**
 * itremixpwing.c - C port of the Smash Remix Pwing item
 * (.toolchain/pkgs/smashremix-src/src/items/Pwing.asm).
 *
 * Pwing is a consume item. Its pickup activates a per-player effect that gives
 * infinite midair jumps for a time. The effect runs as a process on the owner
 * fighter's GObj (the asm uses a render routine). Each routine names its Remix
 * label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixpwing.h"
#include "it/itmanager.h"
#include "it/itcommon/ittomato.h"
#include "if/ifcommon.h"
#include "ef/efmanager.h"
#include "ft/fighter.h"
#include "ft/ftparam.h"
#include "gm/gmremixcolscripts.h"
#include "sys/objman.h"
#include "remix/remix_roster_data.h"

#define ITREMIX_PWING_KIND 0x37
#define ITREMIX_PWING_FILE_OFFSET 0x1060 /* asm FILE_OFFSET: item footer in file 0xFB */
#define ITREMIX_PWING_DURATION (20 * 60) /* frames */
#define ITREMIX_PWING_JUMP_COOLDOWN 27 /* frames between Pwing jumps */
#define ITREMIX_PWING_LOW_TIME (60 * 3) /* below this many frames the low-time sfx plays */
#define ITREMIX_PWING_BONK_COOLDOWN 6 /* frames a bonked fighter must fall before jumping again */
#define ITREMIX_PWING_MARINA_JUMPS 3 /* asm Marina and Polygon Marina use 3 jumps */
#define ITREMIX_PWING_MARINA_BONK_JUMPS 6 /* asm sets Marina's jumps_used to 6 on a ceiling bonk */
#define ITREMIX_PWING_CIRCLE_SIZE 20 /* asm effect size, scaled by the topjoint x scale */
#define ITREMIX_PWING_FGM_PICKUP 0x559
#define ITREMIX_PWING_FGM_JUMP 0x558
#define ITREMIX_PWING_FGM_JUMP_LOW 0x55B
#define ITREMIX_PWING_FGM_END 0x55A
#define ITREMIX_PWING_PROCESS_PRI 4 /* the port has no render order; Remix used Render group 1, order 0x8000 */
#define ITREMIX_PWING_COLANIM 0x7A /* asm GFX_ROUTINE, GFXRoutine.asm add_gfx_routine(PWING) */

enum itRemixPwingJumpState
{
    nITRemixPwingJumpNone,
    nITRemixPwingJumpJumping,
    nITRemixPwingJumpBonked
};

/* Remix Action ids that the Pwing asm compares against (Action.asm, Kirby, Dedede, Jigglypuff, Ebi). */
enum itRemixPwingStatus
{
    nITRemixPwingStatusIdle = 0x00A,
    nITRemixPwingStatusJumpAerialF = 0x018,
    nITRemixPwingStatusJumpAerialB = 0x019,
    nITRemixPwingStatusFall = 0x01A,
    nITRemixPwingStatusFallAerial = 0x01B,
    nITRemixPwingStatusFallSpecial = 0x03A,
    nITRemixPwingStatusCeilingBonk = 0x042,
    nITRemixPwingStatusCliffCatch = 0x054,
    nITRemixPwingStatusInhaleWait = 0x0AE,
    nITRemixPwingStatusEggLay = 0x0B2,
    nITRemixPwingStatusEbiUSP = 0x0DF,
    nITRemixPwingStatusKirbyJump3 = 0x0E0,
    nITRemixPwingStatusKirbyJump4 = 0x0E1,
    nITRemixPwingStatusKirbyJump5 = 0x0E2,
    nITRemixPwingStatusDededeJump4 = 0x0E1,
    nITRemixPwingStatusJigglypuffJump4 = 0x0E1
};

/* Per-player Pwing state. Remix kept the handler pointer and the jump flag in two arrays indexed by port. */
typedef struct ITRemixPwingPlayer
{
    GObj *fighter_gobj;     /* owner fighter while a Pwing is active */
    GObjProcess *process;   /* NULL when no Pwing is active for this port */
    s32 timer;              /* frames left */
    s32 jump_state;         /* enum itRemixPwingJumpState */
} ITRemixPwingPlayer;

extern void* func_800269C0_275C0(u16 id);

static ITRemixPwingPlayer sITRemixPwingPlayers[MAXCONTROLLERS];

static void itRemixPwingProcUpdate(GObj *fighter_gobj);
static sb32 itRemixPwingEvaluateCooldown(FTStruct *fp, ITRemixPwingPlayer *slot, s32 jumps_required);

/* asm item_info_array. Same tomato-style item footer and draw setup as Maxim Tomato. */
static ITDesc sITRemixPwingItemInfo =
{
    ITREMIX_PWING_KIND,
    &gITManagerCommonData,
    ITREMIX_PWING_FILE_OFFSET,

    {
        nGCMatrixKindTraRotRpyRSca,         /* item_info_array 0x0C = 0x1C000000 */
        nGCMatrixKindNull,
        0
    },

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

/** Pwing.stage_setting_ (SPAWN_ITEM): spawns the item with the same tomato setup, using the Pwing item info. */
GObj* itRemixPwingMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixPwingItemInfo, pos, vel, flags);
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    if (item_gobj == NULL)
    {
        return NULL;
    }
    joint = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = translate;
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/** Returns the player's Pwing slot. */
static ITRemixPwingPlayer* itRemixPwingGetPlayer(FTStruct *fp)
{
    return &sITRemixPwingPlayers[fp->player];
}

sb32 itRemixPwingIsJumping(FTStruct *fp)
{
    return (itRemixPwingGetPlayer(fp)->jump_state != nITRemixPwingJumpNone) ? TRUE : FALSE;
}

/** Jumps the fighter has at its base count (asm: 2 for standard jumps, 4 for multi-jump characters). */
static s32 itRemixPwingGetJumpsBase(FTStruct *fp)
{
    return (fp->attr->jumps_max == 2) ? 2 : 4;
}

/** Jumps before the Pwing starts (asm _set_number_of_jumps, Marina overrides to 3). */
static s32 itRemixPwingGetJumpsRequired(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_MARINA || fp->fkind == REMIX_ID_NMARINA)
    {
        return ITREMIX_PWING_MARINA_JUMPS;
    }
    return itRemixPwingGetJumpsBase(fp);
}

/** TRUE when the fighter's action is below Idle, which the asm treats as dead. */
static sb32 itRemixPwingIsDead(FTStruct *fp)
{
    return fp->status_id < nITRemixPwingStatusIdle;
}

/** TRUE when a Pwing is active for the fighter that owns this slot. */
static sb32 itRemixPwingIsActive(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    return slot->process != NULL && slot->fighter_gobj == fp->fighter_gobj;
}

/** TRUE for actions that cancel the jump state (asm _check_action). */
static sb32 itRemixPwingIsCancelStatus(s32 status_id)
{
    return status_id == nITRemixPwingStatusFallSpecial ||
        status_id == nITRemixPwingStatusCliffCatch ||
        status_id == nITRemixPwingStatusInhaleWait ||
        status_id == nITRemixPwingStatusEggLay;
}

/** TRUE for the multi-jump actions of Kirby, Dedede and Jigglypuff (asm _check_multi_jump, Jump6 is loaded but never compared). */
static sb32 itRemixPwingIsMultiJumpStatus(s32 status_id)
{
    return status_id == nITRemixPwingStatusKirbyJump3 ||
        status_id == nITRemixPwingStatusKirbyJump4 ||
        status_id == nITRemixPwingStatusDededeJump4 ||
        status_id == nITRemixPwingStatusJigglypuffJump4 ||
        status_id == nITRemixPwingStatusKirbyJump5;
}

/**
 * The early exits of handle_active_pwing_. Returns FALSE when the frame only
 * counts the timer down. Clears the jump state when the asm does.
 */
static sb32 itRemixPwingCanJump(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    if (fp->jumps_used == 0 || fp->ga == 0)
    {
        slot->jump_state = nITRemixPwingJumpNone;
        return FALSE;
    }
    if (fp->fkind == REMIX_ID_EBI && fp->status_id == nITRemixPwingStatusEbiUSP)
    {
        return FALSE;
    }
    if (itRemixPwingIsCancelStatus(fp->status_id))
    {
        slot->jump_state = nITRemixPwingJumpNone;
        return FALSE;
    }
    return TRUE;
}

/**
 * Ceiling bonk while jumping via Pwing (asm _check_bonk). Sets the bonked state,
 * and Marina loses her remaining jumps. Returns TRUE when a bonk was handled.
 */
static sb32 itRemixPwingCheckBonk(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    if (slot->jump_state == nITRemixPwingJumpNone || fp->status_id != nITRemixPwingStatusCeilingBonk)
    {
        return FALSE;
    }
    slot->jump_state = nITRemixPwingJumpBonked;
    if (fp->fkind == REMIX_ID_MARINA || fp->fkind == REMIX_ID_NMARINA)
    {
        fp->jumps_used = ITREMIX_PWING_MARINA_BONK_JUMPS;
    }
    return TRUE;
}

/**
 * Falling after a bonk must wait a few frames before a jump counts (asm _check_fall,
 * _check_if_we_bonked). Returns TRUE when the frame must only count the timer down.
 */
static sb32 itRemixPwingIsBonkCooldown(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    if (fp->status_id != nITRemixPwingStatusFall && fp->status_id != nITRemixPwingStatusFallAerial)
    {
        return FALSE;
    }
    if (slot->jump_state != nITRemixPwingJumpBonked)
    {
        return FALSE;
    }
    return fp->status_total_tics < ITREMIX_PWING_BONK_COOLDOWN;
}

/** Plays the jump sfx, the low-time variant when the Pwing is nearly over (asm _pwing_sfx). */
static void itRemixPwingPlayJumpSfx(ITRemixPwingPlayer *slot)
{
    if (slot->timer < ITREMIX_PWING_LOW_TIME)
    {
        func_800269C0_275C0(ITREMIX_PWING_FGM_JUMP_LOW);
        return;
    }
    func_800269C0_275C0(ITREMIX_PWING_FGM_JUMP);
}

/** Jump check for a jump action, or for a jump that started while the Pwing is off (asm _is_jumping). */
static sb32 itRemixPwingEvaluateJumping(FTStruct *fp, ITRemixPwingPlayer *slot, s32 jumps_required)
{
    if (slot->jump_state == nITRemixPwingJumpNone)
    {
        return TRUE;
    }
    if (fp->jumps_used < jumps_required)
    {
        return FALSE;
    }
    return itRemixPwingEvaluateCooldown(fp, slot, jumps_required);
}

/**
 * Cooldown that applies to standard jumps (asm _check_frame). Multi-jump
 * characters skip it. Frame 0 plays the sfx and does not grant a jump.
 */
static sb32 itRemixPwingEvaluateCooldown(FTStruct *fp, ITRemixPwingPlayer *slot, s32 jumps_required)
{
    if (fp->status_total_tics == 0)
    {
        itRemixPwingPlayJumpSfx(slot);
        return FALSE;
    }
    if (jumps_required != 2)
    {
        return TRUE;
    }
    return fp->status_total_tics >= ITREMIX_PWING_JUMP_COOLDOWN;
}

/** Grants the jump when no jumps are left, otherwise only when the Pwing is already jumping (asm _check_jumps_remaining). */
static sb32 itRemixPwingEvaluateRemaining(FTStruct *fp, ITRemixPwingPlayer *slot, s32 jumps_required)
{
    if (fp->jumps_used >= jumps_required)
    {
        return TRUE;
    }
    return slot->jump_state != nITRemixPwingJumpNone;
}

/**
 * handle_active_pwing_ jump decision. Returns TRUE when the Pwing grants jumps
 * this frame. Sfx and state changes that the asm makes on the way are done here.
 */
static sb32 itRemixPwingEvaluateJump(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    s32 jumps_required;

    if (!itRemixPwingCanJump(fp, slot))
    {
        return FALSE;
    }
    jumps_required = itRemixPwingGetJumpsRequired(fp);
    if (itRemixPwingCheckBonk(fp, slot))
    {
        return FALSE;
    }
    if (itRemixPwingIsBonkCooldown(fp, slot))
    {
        return FALSE;
    }
    if (fp->status_id == nITRemixPwingStatusJumpAerialF || fp->status_id == nITRemixPwingStatusJumpAerialB)
    {
        return itRemixPwingEvaluateJumping(fp, slot, jumps_required);
    }
    if (jumps_required >= 4 && itRemixPwingIsMultiJumpStatus(fp->status_id))
    {
        return itRemixPwingEvaluateJumping(fp, slot, jumps_required);
    }
    return itRemixPwingEvaluateRemaining(fp, slot, jumps_required);
}

/** Sets the Pwing jump state and the jumps used (asm _apply_pwing). */
static void itRemixPwingApplyJump(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    slot->jump_state = nITRemixPwingJumpJumping;
    fp->jumps_used = itRemixPwingGetJumpsRequired(fp) - 1;
}

/** Starts a Pwing for the fighter, or the Pwing's process is already running and its timer is refreshed. */
static void itRemixPwingStart(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    gmRemixColScriptsSetOverride(fp->player, ITREMIX_PWING_COLANIM);
    ftParamCheckSetFighterColAnimID(fp->fighter_gobj, ITREMIX_PWING_COLANIM, 0);
    slot->fighter_gobj = fp->fighter_gobj;
    slot->process = gcAddGObjProcess(fp->fighter_gobj, itRemixPwingProcUpdate, nGCProcessKindFunc, ITREMIX_PWING_PROCESS_PRI);
    slot->timer = ITREMIX_PWING_DURATION;
    slot->jump_state = nITRemixPwingJumpNone;
}

/** Pwing white circle effect at the fighter's centre (asm _generate_white_circle_gfx). */
static void itRemixPwingWhiteCircleGfx(FTStruct *fp)
{
    Vec3f pos;
    f32 size;

    pos.x = fp->coll_data.p_translate->x;
    pos.y = fp->coll_data.p_translate->y + fp->attr->map_coll.center;
    pos.z = fp->coll_data.p_translate->z;
    size = ITREMIX_PWING_CIRCLE_SIZE * fp->joints[0]->scale.vec.f.x;
    efManagerSetOffMakeEffect(&pos, (s32) size);
}

/**
 * Ends the Pwing (asm _destroy_pwing). Removes lingering jumps, plays the end
 * sfx and shows the white circle. Clears the slot.
 */
static void itRemixPwingDestroy(FTStruct *fp, ITRemixPwingPlayer *slot)
{
    if (slot->jump_state != nITRemixPwingJumpNone && fp->jumps_used != 0)
    {
        fp->jumps_used = itRemixPwingGetJumpsBase(fp);
    }
    func_800269C0_275C0(ITREMIX_PWING_FGM_END);
    slot->jump_state = nITRemixPwingJumpNone;
    if (gmRemixColScriptsGetOverride(fp->player) == ITREMIX_PWING_COLANIM)
    {
        gmRemixColScriptsSetOverride(fp->player, 0);
        ftParamResetStatUpdateColAnim(fp->fighter_gobj);
    }
    itRemixPwingWhiteCircleGfx(fp);
    gcEndGObjProcess(slot->process);
    slot->process = NULL;
    slot->fighter_gobj = NULL;
}

/** handle_active_pwing_: runs every frame while a Pwing is active for the fighter. */
static void itRemixPwingProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITRemixPwingPlayer *slot = itRemixPwingGetPlayer(fp);

    if (itRemixPwingIsDead(fp))
    {
        slot->jump_state = nITRemixPwingJumpNone;
        itRemixPwingDestroy(fp, slot);
        return;
    }
    if (itRemixPwingEvaluateJump(fp, slot))
    {
        itRemixPwingApplyJump(fp, slot);
    }
    if (slot->timer == 1)
    {
        itRemixPwingDestroy(fp, slot);
        return;
    }
    slot->timer -= 1;
}

/** Pwing.pickup_pwing (PICKUP_ITEM_MAIN): activates the Pwing, or refreshes its timer if already active. */
static void itRemixPwingPickup(FTStruct *fp, GObj *item_gobj)
{
    ITRemixPwingPlayer *slot = itRemixPwingGetPlayer(fp);

    if (itRemixPwingIsActive(fp, slot))
    {
        slot->timer = ITREMIX_PWING_DURATION;
    }
    else
    {
        itRemixPwingStart(fp, slot);
    }
    func_800269C0_275C0(ITREMIX_PWING_FGM_PICKUP);
    itRemixConsumeTomatoTail(fp, item_gobj);
}

/** Pwing.clear_active_pwings_: resets every player's slot. */
void itRemixPwingClearActive(void)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        sITRemixPwingPlayers[port].fighter_gobj = NULL;
        sITRemixPwingPlayers[port].process = NULL;
        sITRemixPwingPlayers[port].timer = 0;
        sITRemixPwingPlayers[port].jump_state = nITRemixPwingJumpNone;
    }
}

/** Descriptor of Pwing (item id 0x37) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixPwingItemDesc =
{
    ITREMIX_PWING_KIND,                 /* Item.Pwing.id */
    itRemixPwingMakeItem,               /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT: none */
    itRemixPwingPickup,                 /* PICKUP_ITEM_MAIN */
    itTomatoDroppedSetStatus,           /* DROP_ITEM (asm 0x801745FC, same as Maxim Tomato) */
    NULL,                               /* THROW_ITEM: none */
    NULL                                /* PLAYER_COLLISION: none */
};

#endif
