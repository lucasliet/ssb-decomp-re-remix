/*
 * ftremixchargesmash.c - Charge Smashes toggle (Smash Remix ChargeSmashAttacks.asm). A smash held on its charge frame
 * freezes the animation, builds a charge time, shakes the fighter and releases the charge after MAX_CHARGE_TIME frames
 * (never with the unlimited value). Every entry point is a no-op while the toggle is off, except the animation end hold,
 * which is only taken when the toggle is on.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftdisplaymain.h>
#include <ef/efmanager.h>
#include <gm/gmdef.h>
#include <sys/audio.h>
#include <sys/obj.h>
#include <sys/objanim.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_CHARGE_SMASH_GFX_ROUTINE 0x78       /* GFXRoutine.CHARGE_SMASH, build/remixasm/bass.log: ID is 0x78 */
#define FTREMIX_CHARGE_SMASH_MAX_TIME 60            /* ChargeSmashAttacks.asm MAX_CHARGE_TIME (1 second) */
#define FTREMIX_CHARGE_SMASH_SFX 0x528              /* ChargeSmashAttacks.asm FGM.play(0x528) */
#define FTREMIX_CHARGE_SMASH_SMOKE_INTERVAL 16      /* ChargeSmashAttacks.asm _spawn_smoke: charge time % 16 */
#define FTREMIX_CHARGE_SMASH_SMOKE_OFFSET 80.0F     /* ChargeSmashAttacks.asm _spawn_smoke: -80.0F on x and y */
#define FTREMIX_CHARGE_SMASH_SHUFFLE_TICS 0xFF      /* ChargeSmashAttacks.asm _shuffle */
#define FTREMIX_CHARGE_SMASH_SHUFFLE_ELECTRIC 2     /* ChargeSmashAttacks.asm _shuffle: is_shuffle_electric = 2 */
#define FTREMIX_CHARGE_SMASH_SHUFFLE_INDEX_MAX 4    /* ChargeSmashAttacks.asm _shuffle: shuffle_index_max = 4 */
#define FTREMIX_CHARGE_SMASH_TOGGLE_UNLIMITED 0x2   /* Toggles.asm entry_charged_smashes value 2 */

typedef enum FTRemixChargeSmashAction
{
    nFTRemixChargeSmashActionBeamSwordSmash = 0x80,
    nFTRemixChargeSmashActionBatSmash = 0x84,
    nFTRemixChargeSmashActionFanSmash = 0x88,
    nFTRemixChargeSmashActionStarRodSmash = 0x8C,
    nFTRemixChargeSmashActionFSmashHigh = 0xCA,
    nFTRemixChargeSmashActionUSmash = 0xCF,
    nFTRemixChargeSmashActionDSmash = 0xD0,
    nFTRemixChargeSmashActionAttackAirN = 0xD1
} FTRemixChargeSmashAction;

typedef struct FTRemixChargeFrames
{
    u8 forward; /* first frame a forward smash can be charged */
    u8 up;
    u8 down;
} FTRemixChargeFrames;

typedef struct FTRemixChargeState
{
    s32 time;        /* charge time in frames, 0 when not charging */
    f32 saved_speed; /* DObj anim speed kept while the charge freezes the animation */
} FTRemixChargeState;

/* ChargeSmashAttacks.asm shared_item_smash_table, indexed by (action - BeamSwordSmash) / 4 */
static const s32 sFTRemixChargeSmashItemFrames[] = { 14, 22, 7, 15 };

/* ChargeSmashAttacks.asm custom_shuffle: the offsets of a charged hit, (8, 0), (0, 8), (-8, 0), (0, -8) */
static const Vec2f sFTRemixChargeSmashShuffle[4] =
{
    {  8.0F,  0.0F },
    {  0.0F,  8.0F },
    { -8.0F,  0.0F },
    {  0.0F, -8.0F }
};

static FTRemixChargeState sFTRemixChargeStates[GMCOMMON_PLAYERS_MAX];

/* ChargeSmashAttacks.asm entry_* tables: charge start frame of forward, up and down smash */
static const FTRemixChargeFrames sFTRemixChargeEntryMario = { 5, 5, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryFox = { 2, 3, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryDK = { 6, 5, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntrySamus = { 8, 8, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryLink = { 4, 3, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryYoshi = { 9, 6, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryFalcon = { 6, 2, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryNess = { 6, 6, 8 };
static const FTRemixChargeFrames sFTRemixChargeEntryKirby = { 3, 3, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryPikachu = { 3, 2, 5 };
static const FTRemixChargeFrames sFTRemixChargeEntryPuff = { 3, 3, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryFalco = { 8, 2, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryGanon = { 0x15, 6, 9 };
static const FTRemixChargeFrames sFTRemixChargeEntryDrMario = { 3, 3, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryWario = { 3, 5, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryDarkSamus = { 7, 8, 10 };
static const FTRemixChargeFrames sFTRemixChargeEntryLucas = { 5, 5, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryBowser = { 0x12, 0xB, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryPiano = { 2, 6, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryWolf = { 2, 3, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryConker = { 4, 7, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryMewtwo = { 0xD, 4, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryMarth = { 7, 7, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntrySonic = { 4, 2, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntrySheik = { 5, 8, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryMarina = { 2, 4, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryDedede = { 8, 10, 5 };
static const FTRemixChargeFrames sFTRemixChargeEntryGoemon = { 5, 4, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryEbisumaru = { 3, 3, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryPeppy = { 3, 3, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntrySlippy = { 3, 3, 2 };
static const FTRemixChargeFrames sFTRemixChargeEntryBanjo = { 4, 3, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryDragonKing = { 7, 2, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryLanky = { 6, 6, 6 };
static const FTRemixChargeFrames sFTRemixChargeEntryGiga = { 42, 23, 10 };
/* Set by Crash.asm, Peach.asm and Roy.asm through set_charged_smash_attacks */
static const FTRemixChargeFrames sFTRemixChargeEntryCrash = { 7, 2, 4 };
static const FTRemixChargeFrames sFTRemixChargeEntryPeach = { 7, 9, 3 };
static const FTRemixChargeFrames sFTRemixChargeEntryRoy = { 7, 7, 3 };

/* ChargeSmashAttacks.asm frame_table: the charge frames of each Remix fighter, indexed by Remix id (= fkind) */
static const FTRemixChargeFrames *sFTRemixChargeFramesByKind[REMIX_ID_NCRASH + 1] =
{
    [REMIX_ID_MARIO] = &sFTRemixChargeEntryMario,
    [REMIX_ID_FOX] = &sFTRemixChargeEntryFox,
    [REMIX_ID_DONKEY] = &sFTRemixChargeEntryDK,
    [REMIX_ID_SAMUS] = &sFTRemixChargeEntrySamus,
    [REMIX_ID_LUIGI] = &sFTRemixChargeEntryMario,
    [REMIX_ID_LINK] = &sFTRemixChargeEntryLink,
    [REMIX_ID_YOSHI] = &sFTRemixChargeEntryYoshi,
    [REMIX_ID_CAPTAIN] = &sFTRemixChargeEntryFalcon,
    [REMIX_ID_KIRBY] = &sFTRemixChargeEntryKirby,
    [REMIX_ID_PIKACHU] = &sFTRemixChargeEntryPikachu,
    [REMIX_ID_JIGGLY] = &sFTRemixChargeEntryPuff,
    [REMIX_ID_NESS] = &sFTRemixChargeEntryNess,
    [REMIX_ID_METAL] = &sFTRemixChargeEntryMario,
    [REMIX_ID_BOSS] = &sFTRemixChargeEntryMario,
    [REMIX_ID_NMARIO] = &sFTRemixChargeEntryMario,
    [REMIX_ID_NFOX] = &sFTRemixChargeEntryFox,
    [REMIX_ID_NDONKEY] = &sFTRemixChargeEntryDK,
    [REMIX_ID_NSAMUS] = &sFTRemixChargeEntrySamus,
    [REMIX_ID_NLUIGI] = &sFTRemixChargeEntryMario,
    [REMIX_ID_NLINK] = &sFTRemixChargeEntryLink,
    [REMIX_ID_NYOSHI] = &sFTRemixChargeEntryYoshi,
    [REMIX_ID_NCAPTAIN] = &sFTRemixChargeEntryFalcon,
    [REMIX_ID_NKIRBY] = &sFTRemixChargeEntryKirby,
    [REMIX_ID_NPIKACHU] = &sFTRemixChargeEntryPikachu,
    [REMIX_ID_NJIGGLY] = &sFTRemixChargeEntryPuff,
    [REMIX_ID_NNESS] = &sFTRemixChargeEntryNess,
    [REMIX_ID_GDONKEY] = &sFTRemixChargeEntryDK,
    [REMIX_ID_FALCO] = &sFTRemixChargeEntryFalco,
    [REMIX_ID_GND] = &sFTRemixChargeEntryGanon,
    [REMIX_ID_YLINK] = &sFTRemixChargeEntryLink,
    [REMIX_ID_DRM] = &sFTRemixChargeEntryDrMario,
    [REMIX_ID_WARIO] = &sFTRemixChargeEntryWario,
    [REMIX_ID_DSAMUS] = &sFTRemixChargeEntryDarkSamus,
    [REMIX_ID_ELINK] = &sFTRemixChargeEntryLink,
    [REMIX_ID_JSAMUS] = &sFTRemixChargeEntrySamus,
    [REMIX_ID_JNESS] = &sFTRemixChargeEntryNess,
    [REMIX_ID_LUCAS] = &sFTRemixChargeEntryLucas,
    [REMIX_ID_JLINK] = &sFTRemixChargeEntryLink,
    [REMIX_ID_JFALCON] = &sFTRemixChargeEntryFalcon,
    [REMIX_ID_JFOX] = &sFTRemixChargeEntryFox,
    [REMIX_ID_JMARIO] = &sFTRemixChargeEntryMario,
    [REMIX_ID_JLUIGI] = &sFTRemixChargeEntryMario,
    [REMIX_ID_JDK] = &sFTRemixChargeEntryDK,
    [REMIX_ID_EPIKA] = &sFTRemixChargeEntryPikachu,
    [REMIX_ID_JPUFF] = &sFTRemixChargeEntryPuff,
    [REMIX_ID_EPUFF] = &sFTRemixChargeEntryPuff,
    [REMIX_ID_JKIRBY] = &sFTRemixChargeEntryKirby,
    [REMIX_ID_JYOSHI] = &sFTRemixChargeEntryYoshi,
    [REMIX_ID_JPIKA] = &sFTRemixChargeEntryPikachu,
    [REMIX_ID_ESAMUS] = &sFTRemixChargeEntrySamus,
    [REMIX_ID_BOWSER] = &sFTRemixChargeEntryBowser,
    [REMIX_ID_GBOWSER] = &sFTRemixChargeEntryGiga,
    [REMIX_ID_PIANO] = &sFTRemixChargeEntryPiano,
    [REMIX_ID_WOLF] = &sFTRemixChargeEntryWolf,
    [REMIX_ID_CONKER] = &sFTRemixChargeEntryConker,
    [REMIX_ID_MTWO] = &sFTRemixChargeEntryMewtwo,
    [REMIX_ID_MARTH] = &sFTRemixChargeEntryMarth,
    [REMIX_ID_SONIC] = &sFTRemixChargeEntrySonic,
    [REMIX_ID_SANDBAG] = &sFTRemixChargeEntryMario,
    [REMIX_ID_SSONIC] = &sFTRemixChargeEntrySonic,
    [REMIX_ID_SHEIK] = &sFTRemixChargeEntrySheik,
    [REMIX_ID_MARINA] = &sFTRemixChargeEntryMarina,
    [REMIX_ID_DEDEDE] = &sFTRemixChargeEntryDedede,
    [REMIX_ID_GOEMON] = &sFTRemixChargeEntryGoemon,
    [REMIX_ID_PEPPY] = &sFTRemixChargeEntryPeppy,
    [REMIX_ID_SLIPPY] = &sFTRemixChargeEntrySlippy,
    [REMIX_ID_BANJO] = &sFTRemixChargeEntryBanjo,
    [REMIX_ID_MLUIGI] = &sFTRemixChargeEntryMario,
    [REMIX_ID_EBI] = &sFTRemixChargeEntryEbisumaru,
    [REMIX_ID_DRAGONKING] = &sFTRemixChargeEntryDragonKing,
    [REMIX_ID_NWARIO] = &sFTRemixChargeEntryWario,
    [REMIX_ID_NLUCAS] = &sFTRemixChargeEntryLucas,
    [REMIX_ID_NBOWSER] = &sFTRemixChargeEntryBowser,
    [REMIX_ID_NWOLF] = &sFTRemixChargeEntryWolf,
    [REMIX_ID_NDRM] = &sFTRemixChargeEntryDrMario,
    [REMIX_ID_NSONIC] = &sFTRemixChargeEntrySonic,
    [REMIX_ID_NSHEIK] = &sFTRemixChargeEntrySheik,
    [REMIX_ID_NMARINA] = &sFTRemixChargeEntryMarina,
    [REMIX_ID_NFALCO] = &sFTRemixChargeEntryFalco,
    [REMIX_ID_NGND] = &sFTRemixChargeEntryGanon,
    [REMIX_ID_NDSAMUS] = &sFTRemixChargeEntryDarkSamus,
    [REMIX_ID_NMARTH] = &sFTRemixChargeEntryMarth,
    [REMIX_ID_NMTWO] = &sFTRemixChargeEntryMewtwo,
    [REMIX_ID_NDEDEDE] = &sFTRemixChargeEntryDedede,
    [REMIX_ID_NYLINK] = &sFTRemixChargeEntryLink,
    [REMIX_ID_NGOEMON] = &sFTRemixChargeEntryGoemon,
    [REMIX_ID_NCONKER] = &sFTRemixChargeEntryConker,
    [REMIX_ID_NBANJO] = &sFTRemixChargeEntryBanjo,
    [REMIX_ID_DRL] = &sFTRemixChargeEntryMario,
    [REMIX_ID_LANKY] = &sFTRemixChargeEntryLanky,
    [REMIX_ID_ROY] = &sFTRemixChargeEntryRoy,
    [REMIX_ID_PEACH] = &sFTRemixChargeEntryPeach,
    [REMIX_ID_CRASH] = &sFTRemixChargeEntryCrash,
    [REMIX_ID_NPEACH] = &sFTRemixChargeEntryPeach,
    [REMIX_ID_NCRASH] = &sFTRemixChargeEntryCrash
};

/** Returns the charge frames of Remix fighter `fkind`, or NULL when the fighter has none (no charge then). */
static const FTRemixChargeFrames *ftRemixChargeSmashFrames(s32 fkind)
{
    if ((fkind < 0) || (fkind > REMIX_ID_NCRASH))
    {
        return NULL;
    }
    return sFTRemixChargeFramesByKind[fkind];
}

/** Returns TRUE when the action can be charged: a smash, or an item smash while an item is held. */
static sb32 ftRemixChargeSmashIsChargeable(FTStruct *fp)
{
    s32 action_id = fp->status_id;

    if ((action_id >= nFTRemixChargeSmashActionFSmashHigh) && (action_id < nFTRemixChargeSmashActionAttackAirN))
    {
        return TRUE;
    }
    if (fp->item_gobj == NULL)
    {
        return FALSE;
    }
    return (action_id == nFTRemixChargeSmashActionBeamSwordSmash) || (action_id == nFTRemixChargeSmashActionFanSmash)
        || (action_id == nFTRemixChargeSmashActionStarRodSmash);
}

/** Returns the frame at which the charge of `action_id` starts (ChargeSmashAttacks.asm _check_smash_type). */
static s32 ftRemixChargeSmashStartFrame(s32 action_id, const FTRemixChargeFrames *frames)
{
    if (action_id == nFTRemixChargeSmashActionUSmash)
    {
        return frames->up;
    }
    if (action_id == nFTRemixChargeSmashActionDSmash)
    {
        return frames->down;
    }
    if (action_id < nFTRemixChargeSmashActionFSmashHigh)
    {
        return sFTRemixChargeSmashItemFrames[(action_id - nFTRemixChargeSmashActionBeamSwordSmash) / 4];
    }
    return frames->forward;
}

/** Clears the charge state at the first frame of the action (ChargeSmashAttacks.asm _smash_attack). */
static void ftRemixChargeSmashReset(FTRemixChargeState *state)
{
    state->time = 0;
    state->saved_speed = 0.0F;
}

/** Sets the charge GFX routine unless the fighter already has a colour animation. */
static void ftRemixChargeSmashSetGfx(FTStruct *fp)
{
    if (fp->colanim.colanim_id == 0)
    {
        fp->colanim.colanim_id = FTREMIX_CHARGE_SMASH_GFX_ROUTINE;
    }
}

/** Starts the fighter shake of a held charge (ChargeSmashAttacks.asm _shuffle). */
static void ftRemixChargeSmashShake(FTStruct *fp)
{
    fp->shuffle_tics = FTREMIX_CHARGE_SMASH_SHUFFLE_TICS;
    fp->shuffle_frame_index = 0;
    fp->is_shuffle_electric = FTREMIX_CHARGE_SMASH_SHUFFLE_ELECTRIC;
    fp->shuffle_index_max = FTREMIX_CHARGE_SMASH_SHUFFLE_INDEX_MAX;
}

/** Spawns the heavy dust effect behind the fighter (ChargeSmashAttacks.asm _spawn_smoke). */
static void ftRemixChargeSmashSpawnSmoke(FTStruct *fp)
{
    Vec3f pos;
    Vec3f *translate = fp->coll_data.p_translate;

    pos.x = translate->x + ((f32)fp->lr * -FTREMIX_CHARGE_SMASH_SMOKE_OFFSET);
    pos.y = translate->y - FTREMIX_CHARGE_SMASH_SMOKE_OFFSET;
    pos.z = 0.0F;
    efManagerDustHeavyMakeEffect(&pos, -fp->lr);
}

/** Ends a charge: stops the shake and restores the animation speed saved at the charge start (_end_charge). */
static void ftRemixChargeSmashRelease(GObj *fighter_gobj, FTStruct *fp, FTRemixChargeState *state)
{
    f32 saved_speed = state->saved_speed;

    fp->shuffle_tics = 0;
    if (saved_speed == 0.0F)
    {
        return;
    }
    state->saved_speed = 0.0F;
    gcSetAnimSpeed(fighter_gobj, saved_speed);
}

/** First frame of a held charge: freezes the animation, sets the GFX routine and shakes the fighter (_charge_begin_check). */
static void ftRemixChargeSmashBegin(GObj *fighter_gobj, FTStruct *fp, FTRemixChargeState *state)
{
    DObj *dobj = DObjGetStruct(fighter_gobj);

    state->saved_speed = dobj->anim_speed;
    ftRemixChargeSmashSetGfx(fp);
    gcSetAnimSpeed(fighter_gobj, 0.0F);
    state->time += 1;
    syAudioPlayFGM(FTREMIX_CHARGE_SMASH_SFX);
    ftRemixChargeSmashShake(fp);
}

/** Adds a frame to a held charge, smoke included, and releases it at the maximum time (_increment_charge). */
static void ftRemixChargeSmashIncrement(GObj *fighter_gobj, FTStruct *fp, FTRemixChargeState *state)
{
    DObj *dobj = DObjGetStruct(fighter_gobj);

    if (dobj->anim_speed != 0.0F)
    {
        return;
    }
    ftRemixChargeSmashSetGfx(fp);
    state->time += 1;
    if ((state->time % FTREMIX_CHARGE_SMASH_SMOKE_INTERVAL) == 0)
    {
        ftRemixChargeSmashSpawnSmoke(fp);
    }
    if (gmRemixToggle(charged_smashes) & FTREMIX_CHARGE_SMASH_TOGGLE_UNLIMITED)
    {
        return;
    }
    if (state->time == FTREMIX_CHARGE_SMASH_MAX_TIME)
    {
        ftRemixChargeSmashRelease(fighter_gobj, fp, state);
    }
}

/**
 * Runs one frame of the charge logic for the fighter (ChargeSmashAttacks.asm smash_attack_logic). Called when the
 * action changes (smash_attack_start, from ftMainSetStatus) and on every frame the animation end would transition.
 */
void ftRemixChargeSmashLogic(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixChargeState *state;
    const FTRemixChargeFrames *frames;
    s32 frame;
    s32 start_frame;

    if (!gmRemixToggle(charged_smashes) || !ftRemixChargeSmashIsChargeable(fp))
    {
        return;
    }
    frames = ftRemixChargeSmashFrames(fp->fkind);
    if (frames == NULL)
    {
        return;
    }
    state = &sFTRemixChargeStates[fp->player];
    frame = (s32)fp->status_total_tics + 1;
    if (frame == 1)
    {
        ftRemixChargeSmashReset(state);
    }
    start_frame = ftRemixChargeSmashStartFrame(fp->status_id, frames);
    if ((frame < start_frame) || ((frame > start_frame) && (state->time == 0)))
    {
        return;
    }
    if (!(fp->input.pl.button_hold & A_BUTTON))
    {
        ftRemixChargeSmashRelease(fighter_gobj, fp, state);
        return;
    }
    if (state->time != 0)
    {
        ftRemixChargeSmashIncrement(fighter_gobj, fp, state);
        return;
    }
    ftRemixChargeSmashBegin(fighter_gobj, fp, state);
}

/**
 * Holds the wait transition at the end of an animation while a charge freezes the fighter (ChargeSmashAttacks.asm
 * smash_attack_main, hooked into ftAnimEndSetWait). Returns TRUE when the charge holds the fighter. The hold is never
 * taken while the toggle is off, so vanilla fighters keep the vanilla transition.
 */
sb32 ftRemixChargeSmashHoldsAnimEnd(GObj *fighter_gobj)
{
    DObj *dobj;

    if (!gmRemixToggle(charged_smashes))
    {
        return FALSE;
    }
    dobj = DObjGetStruct(fighter_gobj);
    if (dobj->anim_speed != 0.0F)
    {
        return FALSE;
    }
    ftRemixChargeSmashLogic(fighter_gobj);
    return TRUE;
}

/** The shake offset of the fighter model: the vanilla table, or the charge table when the shake is electric (level 2). */
Vec2f ftRemixChargeSmashShufflePosition(FTStruct *fp)
{
    if (fp->is_shuffle_electric < FTREMIX_CHARGE_SMASH_SHUFFLE_ELECTRIC)
    {
        return dFTDisplayMainShufflePositions[fp->is_shuffle_electric][fp->shuffle_frame_index];
    }
    return sFTRemixChargeSmashShuffle[fp->shuffle_frame_index];
}

#endif
