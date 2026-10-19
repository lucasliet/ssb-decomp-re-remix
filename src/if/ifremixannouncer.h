#ifndef _IFREMIXANNOUNCER_H_
#define _IFREMIXANNOUNCER_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/*
 * Smash Remix Pokemon Stadium announcer (src/PokemonAnnouncer.asm). Each enum value is one call table of the asm, in the
 * order the asm declares them. The toggle announcer_mode decides where the announcer speaks (see ifRemixAnnouncerIsActive).
 */
typedef enum IFRemixAnnouncerCall
{
    nIFRemixAnnouncerCallPokemon,
    nIFRemixAnnouncerCallQuickKO,
    nIFRemixAnnouncerCallRegularKO,
    nIFRemixAnnouncerCallLongKO,
    nIFRemixAnnouncerCallGeneral,
    nIFRemixAnnouncerCallCloseMatch,
    nIFRemixAnnouncerCallSleep,
    nIFRemixAnnouncerCallCombo,
    nIFRemixAnnouncerCallKnockbackHigh,
    nIFRemixAnnouncerCallKnockbackMid,
    nIFRemixAnnouncerCallDamage,
    nIFRemixAnnouncerCallThunder,
    nIFRemixAnnouncerCallQuick,
    nIFRemixAnnouncerCallCharge,
    nIFRemixAnnouncerCallDisable,
    nIFRemixAnnouncerCallTeleport,
    nIFRemixAnnouncerCallFirePunch,
    nIFRemixAnnouncerCallRest,
    nIFRemixAnnouncerCallPound,
    nIFRemixAnnouncerCallTough,
    nIFRemixAnnouncerCallCounter,
    nIFRemixAnnouncerCallLittleDamage,
    nIFRemixAnnouncerCallBurn,
    nIFRemixAnnouncerCallSeismicToss,
    nIFRemixAnnouncerCallRecovery,
    nIFRemixAnnouncerCallClang,
    nIFRemixAnnouncerCallStun,
    nIFRemixAnnouncerCallTeamKO,
    nIFRemixAnnouncerCallSlash,
    nIFRemixAnnouncerCallFlamethrower,
    nIFRemixAnnouncerCallGymLeader,
    nIFRemixAnnouncerCallOpening,
    nIFRemixAnnouncerCallSing,
    nIFRemixAnnouncerCallAttack,
    nIFRemixAnnouncerCallDamageEarly,
    nIFRemixAnnouncerCallBurrow,
    nIFRemixAnnouncerCallBurrowAgain,
    nIFRemixAnnouncerCallDig,
    nIFRemixAnnouncerCallBodySlam,
    nIFRemixAnnouncerCallTaunt,
    nIFRemixAnnouncerCallCount
} IFRemixAnnouncerCall;

/** TRUE when the announcer toggle allows speech on the current stage (asm toggle_announcer_ == 0). */
sb32 ifRemixAnnouncerIsActive(void);

/**
 * Resets the timers and creates the per-frame GObj of the announcer. Does nothing when the announcer is not active on the
 * current stage. Called at stage load (asm announcer_setup_, Stages.asm).
 */
void ifRemixAnnouncerSetup(void);

/**
 * Plays a clip of the table of the call (asm announcer_main_ STANDARD). Returns TRUE when a clip played.
 */
sb32 ifRemixAnnouncerPlayCall(IFRemixAnnouncerCall call);

/**
 * Plays the Pokemon name clip of a pokemon item kind (asm announcer_main_ NAME). Returns TRUE when a clip played.
 */
sb32 ifRemixAnnouncerPlayName(s32 item_kind);

/**
 * Plays the KO clip for the victim, chosen by the elapsed time and the victim's damage (asm announcer_main_ KO).
 * Returns TRUE when a clip played.
 */
sb32 ifRemixAnnouncerPlayKO(FTStruct *victim_fp);

/** Requests the sleep clip; it plays on a later frame when the global timer allows it. */
void ifRemixAnnouncerRequestSleep(void);

/**
 * Replaces the vanilla game set clip with THERE GOES THE BATTLE when the announcer is active (asm there_goes_).
 * Returns the FGM id to play.
 */
u16 ifRemixAnnouncerGameSetFgm(u16 vanilla_fgm);

/** Returns the announcer TIME UP clip in place of the vanilla one (FGM 0x20F) while the announcer is active (asm time_up_announcement_). */
u16 ifRemixAnnouncerTimeUpFgm(u16 vanilla_fgm);

/**
 * Lengthens the delay before the battle end transition when the announcer is active (asm delay_, 0x5A to 0x80).
 * Returns the delay in frames.
 */
s32 ifRemixAnnouncerGameSetDelay(s32 vanilla_delay);

#endif

#endif
