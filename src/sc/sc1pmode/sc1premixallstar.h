#ifndef _SC1PREMIXALLSTAR_H_
#define _SC1PREMIXALLSTAR_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sc/sctypes.h>
#include "sc/sc1pmode/sc1premix1prules.h"

/*
 * All-Star (SinglePlayerModes.asm allstar_randomization and its progress counters): the run faces every fighter of the pool once, on
 * a stage of its own, with a Rest Area between matches. The order is built when START is pressed on the 1P CSS.
 */

/** Matches of the run by how many opponents they have: 1 up to DOUBLE, 2 up to TRIPLE, 3 up to FINAL (the progress counts fighters, not matches). */
#define SC1PREMIX_ALLSTAR_DOUBLE_PROGRESS 4
#define SC1PREMIX_ALLSTAR_TRIPLE_PROGRESS 0x12
#define SC1PREMIX_ALLSTAR_FINAL_PROGRESS 0x1B

/** TRUE while the 1P game being played is All-Star with its order built. */
extern sb32 sc1PRemixAllstarIsActive(void);

/** Builds a new order and clears the run (the hook of the 1P CSS START); no-op outside All-Star. */
extern void sc1PRemixAllstarRandomize(void);

/** Fighters already faced (index into the order of the next match). */
extern s32 sc1PRemixAllstarGetProgress(void);

/** Fighter kind of an opponent slot of the order. @param slot 0 to SC1PREMIX_ALLSTAR_FIGHTER_COUNT - 1. */
extern s32 sc1PRemixAllstarGetFkind(s32 slot);

/** Stage id (gkind) of an opponent slot of the order. @param slot 0 to SC1PREMIX_ALLSTAR_FIGHTER_COUNT - 1. */
extern s32 sc1PRemixAllstarGetGkind(s32 slot);

/**
 * How many fighters the next match has (advance_allstar): 1 while the progress is below DOUBLE, 2 below TRIPLE, 3 after.
 *
 * @param progress Fighters faced so far.
 */
extern s32 sc1PRemixAllstarOpponentsAt(s32 progress);

/** Stage types of the run, which the 1P game keeps in spgame_stage: the Rest Area and the matches of 1, 2 and 3 opponents and the final. */
#define SC1PREMIX_ALLSTAR_STAGE_REST 0
#define SC1PREMIX_ALLSTAR_STAGE_SINGLE 2
#define SC1PREMIX_ALLSTAR_STAGE_DOUBLE 4
#define SC1PREMIX_ALLSTAR_STAGE_TRIPLE 5
#define SC1PREMIX_ALLSTAR_STAGE_FINAL 0xC

/**
 * Line of the All-Star stage table for a stage type, with the opponents and the stage of the next match already in it.
 *
 * @param stage_index Stage type.
 * @return The line, or NULL when All-Star is not active or the type is past the table.
 */
extern SC1PGameStage* sc1PRemixAllstarGetStage(s32 stage_index);

/** CPU settings of a stage type of the All-Star table; NULL when All-Star is not active. */
extern SC1PGameComputer* sc1PRemixAllstarGetComputer(s32 stage_index);

/**
 * Stage type that follows the one just played (progress_advancement, advance_allstar, rest_area_routine): from the Rest Area the type
 * of the next match for the current progress, from a match the Rest Area or, after the last fighter, the end of the run.
 *
 * @param stage Stage type just played, already stepped back by one when the player continued.
 * @param is_retry TRUE when the player continued after losing, which plays the same stage again.
 * @return The next stage type, or SC1PREMIX_ALLSTAR_STAGE_END.
 */
extern s32 sc1PRemixAllstarStepStage(s32 stage, sb32 is_retry);

/** Per-tic check of the Rest Area: the run leaves it when the player stands on the exit (rest_area_routine). */
extern void sc1PRemixAllstarUpdate(void);

/** Damage the human fighter starts a match with: the percent it ended the last one with (player_percent). @param player Port of the fighter. */
extern s32 sc1PRemixAllstarPlayerDamage(s32 player);

/** Keeps the percent of the human fighter at the end of a match (record_percent). */
extern void sc1PRemixAllstarRecordPercent(s32 percent);

/** The human lost a stock: the kept percent is zero and the run is in limbo until the next fighter is made (clear_percent, _ko_save). */
extern void sc1PRemixAllstarOnPlayerKo(void);

/** The human continued after losing: the kept percent is zero and the limbo is over (clear_percent_continue). */
extern void sc1PRemixAllstarOnContinue(void);

/** TRUE while the 1P game being played is the Rest Area of an All-Star run (stage type 0). */
extern sb32 sc1PRemixAllstarIsRestArea(void);

/**
 * Rules of the Rest Area battle (_time_limit): no time limit. Call after the stage table set the time limit; no-op outside the Rest Area.
 */
extern void sc1PRemixAllstarSetupBattle(void);

/**
 * Whether the run has already begun (match_begin_flag, set by rest_area_setup and cleared when START is pressed on the CSS): the
 * scene start of the 1P game then keeps the stocks, the total damage and the score of the run (stock_prevent, damage_prevent,
 * prevent_erase) instead of clearing them.
 */
extern sb32 sc1PRemixAllstarKeepsRunState(void);

/** Places the Rest Area hearts a run still has (rest_area_setup): three, minus the ones already used. */
extern void sc1PRemixAllstarSetupRestArea(void);

/** Counts a heart the human used in the Rest Area (heart_counter). */
extern void sc1PRemixAllstarOnHeartUsed(void);

/** The final team has this many members, the last three fighters of the order taking turns. */
#define SC1PREMIX_ALLSTAR_FINAL_MEMBERS 24

/** Rolls the costumes of the final team (_nSC1PGameStageAllstarFinal_): a random one of six, four at most for the vanilla fighters. */
extern void sc1PRemixAllstarPrepareFinal(void);

/** Fighter kind of a member of the final team. @param member 0 to SC1PREMIX_ALLSTAR_FINAL_MEMBERS - 1. */
extern s32 sc1PRemixAllstarFinalFkind(s32 member);

/** Costume of a member of the final team. @param member 0 to SC1PREMIX_ALLSTAR_FINAL_MEMBERS - 1. */
extern s32 sc1PRemixAllstarFinalCostume(s32 member);

/** Stage value past the last stage of the 1P game, which ends the run. */
#define SC1PREMIX_ALLSTAR_STAGE_END 14

/** Moves the progress past the fighters of the match just won. */
extern void sc1PRemixAllstarAdvance(void);

#endif

#endif
