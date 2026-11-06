#ifndef _IFREMIXANNOUNCERRULES_H_
#define _IFREMIXANNOUNCERRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules and call tables of the Pokemon Stadium announcer (PokemonAnnouncer.asm), separate from the audio and the
 * match state so the headless unit tests (tests/remix) can run them.
 */

/* Each enum value is one call table of the asm, in the order the asm declares them. */
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

/* One call of a table (16 bytes in the asm: id, FGM id, cooldown in frames, running timer, used flag, special flag). */
typedef struct IFRemixAnnouncement
{
    unsigned short id;
    unsigned short fgm_id;
    unsigned int duration;
    unsigned int timer;
    unsigned short used;
    unsigned short flag;
} IFRemixAnnouncement;

/*
 * A call table. In the asm the tables lie one after the other in memory and announcer_main_ draws over the a1 entries that
 * follow the base address, so a draw can run past the entries a table owns (the 12 high knockback calls draw over 18, the
 * burrow call over 3, the Gym Leader Castle call over 2). Neighbouring tables therefore share one array: a table owns
 * "count" entries (the ones whose timers it resets and counts down) and draws over "pick_count".
 */
typedef struct IFRemixAnnouncerTable
{
    IFRemixAnnouncement *entries;
    int count;
    int pick_count;
    int plays_once;
} IFRemixAnnouncerTable;

/** Draws a random index in [0, range), the asm Global.get_random_int_. */
typedef int (*IFRemixAnnouncerDrawFunc)(int range);

/** Table of a call. */
extern IFRemixAnnouncerTable *ifRemixAnnouncerTablesGet(IFRemixAnnouncerCall call);

/** Clears the timers and used flags of every entry (announcer_setup_). */
extern void ifRemixAnnouncerTablesReset(void);

/** Counts the running timer of every entry down by one frame (announcer_constant_). */
extern void ifRemixAnnouncerTablesCountDown(void);

/**
 * STANDARD pick of announcer_main_: up to pick_count draws, the first entry with no timer left is used; when all draws hit
 * a cooling entry the last one is used unless its special flag is set.
 *
 * @param table Table of the call.
 * @param draw Random draw.
 * @return The entry to play, or NULL when the call stays silent.
 */
extern IFRemixAnnouncement *ifRemixAnnouncerRulesPickStandard(IFRemixAnnouncerTable *table, IFRemixAnnouncerDrawFunc draw);

/** TRUE when the call plays at most once per match and already played (clang_announcement_ tests the used byte). */
extern int ifRemixAnnouncerRulesIsSpent(const IFRemixAnnouncerTable *table);

/**
 * TRUE when announcer_constant_ looks at the sleep request this frame. The asm reads it only after the global timer was
 * counted down, so only on frames where that timer was running.
 *
 * @param global_timer_before Global timer before this frame's countdown.
 * @param sleep_requested TRUE when a sleep clip is pending.
 */
extern int ifRemixAnnouncerRulesChecksSleep(unsigned int global_timer_before, int sleep_requested);

#endif

#endif
