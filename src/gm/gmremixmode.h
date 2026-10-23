#ifndef _GMREMIXMODE_H_
#define _GMREMIXMODE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sc/scdef.h>

/*
 * Common base of the Smash Remix VS modes (ROADMAP M6). The mode picked in the Remix Modes page of the VS menu is
 * gMNVSModeRemixMode (MNREMIX_VSMODE_*); the match code reads it through gmRemixModeGet. Timed Stock is the Remix stock
 * rule (TimedStock.asm): the stock rule gets the time bit too, so a stock match also ends when the timer runs out.
 */

#define SCBATTLE_GAMERULE_TIMED_STOCK (SCBATTLE_GAMERULE_TIME | SCBATTLE_GAMERULE_STOCK)

/**
 * Mode of the current VS match: a MNREMIX_VSMODE_* value. SSB64_REMIX_MODE (default, 12cb, tagteam, koth,
 * smashketball, tugofwar) overrides the menu choice for the whole run, so the smoke tests can start a mode without
 * the menu.
 */
extern s32 gmRemixModeGet(void);

/** TRUE when the current VS match mode is `mode` (MNREMIX_VSMODE_*). */
extern sb32 gmRemixModeIs(s32 mode);

/**
 * VsRemixMenu.asm mode_specific_setup_ (before_css_setup_, leave_css_setup_, start_match_setup_ of each mode). The CSS reads
 * the settings of the transfer battle state, so a mode forces its rule, stocks, goal and teams there when the CSS opens and
 * the vanilla settings come back when the CSS returns to the VS menu. Each mode keeps its own stocks, goal and teams
 * between visits. Does nothing for the default mode.
 *
 * gmRemixModeBeforeCSS: at the start of the CSS, before it reads the transfer state.
 * gmRemixModeLeaveCSS: after the CSS stored its values and goes back to the VS menu (or to the title).
 * gmRemixModeStartMatch: after the CSS stored its values and starts the match.
 */
extern void gmRemixModeBeforeCSS(void);
extern void gmRemixModeLeaveCSS(void);
extern void gmRemixModeStartMatch(void);

/**
 * Goal of the current mode, the CSS value the Remix shows in the time picker (King of the Hill: seconds on the hill,
 * Smashketball: winning score).
 */
extern s32 gmRemixModeGetGoal(void);

/** Hides the HUD labels of the modes (remix_hud_text). Called from scManagerRunLoop at each scene change. */
extern void gmRemixModeSceneClear(void);

/** Logs the mode and the rules of the match once at its start. */
extern void gmRemixModeLogMatch(void);

/** TRUE when the stock rule of the battle state also has the time bit (Timed Stock). */
extern sb32 gmRemixTimedStockIsActive(void);

/** TRUE when the match has a timer and it ran out (the timer is skipped for the infinite time limit). */
extern sb32 gmRemixTimedStockIsTimeUp(void);

/**
 * TimedStock.asm pick_scoring_: places come from the time score for the time rule and for a timed stock match that ended
 * by the timer, otherwise from the stock places.
 */
extern sb32 gmRemixTimedStockUsesTimeScore(void);

/**
 * TimedStock.asm score_fix_ (stock branch): score of `player` (0..3) of a timed stock match for the results ordering, the
 * stocks left shifted by 12 minus the damage taken when stocks are left. The time rule keeps the vanilla KOs minus deaths.
 */
extern s32 gmRemixTimedStockScore(s32 player);

/** TimedStock.asm prevent_sudden_death_: FALSE when the match must not enter sudden death after the timer. */
extern sb32 gmRemixTimedStockAllowsSuddenDeath(void);

#endif

#endif
