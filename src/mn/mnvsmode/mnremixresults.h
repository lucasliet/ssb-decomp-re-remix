#ifndef _MNREMIXRESULTS_H_
#define _MNREMIXRESULTS_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnvsmode/mnremixresultsrules.h"

/**
 * Winner data of the VS results screen that Remix keeps per character (src/resultsscreen.asm add_to_results_screen), read from
 * the Remix image. Valid for the fkinds that mnRemixResultsRulesIsExtendedFkind accepts.
 */

/** Announcer voice that says the winner's name (Character.winner_fgm.table). */
extern u32 mnRemixResultsGetWinnerFgm(s32 fkind);

/** Left X of the "WINS!" text (Character.str_wins_lx.table). */
extern f32 mnRemixResultsGetWinsX(s32 fkind);

/** TRUE when the winner line reads "WIN!" (J sound type and Banjo, see mnRemixResultsRulesUsesSingularWin). */
extern sb32 mnRemixResultsIsSingularWin(s32 fkind);

/**
 * Name, left X and X scale of the winner (Character.str_winner_ptr / lx / scale tables).
 *
 * @param fkind Fighter kind of the winner.
 * @param name Receives the letter sprite string, with the spacing digits of the asm (MNREMIX_RESULTS_NAME_BUFFER_SIZE bytes).
 * @param x Receives the left X.
 * @param scale Receives the X scale.
 * @return FALSE when the fighter has no name in the tables; the outputs are then untouched.
 */
extern sb32 mnRemixResultsGetWinnerName(s32 fkind, char *name, f32 *x, f32 *scale);

/**
 * Offsets of the winner's series emblem in the emblem models file (ResultsScreen.series_logo_*_table).
 *
 * @param fkind Fighter kind of the winner.
 * @param dobjdesc Receives the DObjDesc offset.
 * @param mobjsub Receives the MObjSub offset.
 * @param matanim Receives the MatAnimJoint offset.
 * @return FALSE when the fighter has no emblem.
 */
extern sb32 mnRemixResultsGetEmblem(s32 fkind, intptr_t *dobjdesc, intptr_t *mobjsub, intptr_t *matanim);

#endif

#endif
