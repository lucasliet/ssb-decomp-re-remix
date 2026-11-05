#ifndef _SC1PREMIXSCORE_H_
#define _SC1PREMIXSCORE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * 1P game score of Remix 1P and All-Star (SinglePlayer.asm extend_high_score_1p_write_, _1p_, _bonus_count_, _difficulty_): each
 * mode keeps its own score, continues, bonus count, best difficulty and completion per fighter in the Remix records of the save.
 */

/** TRUE when the run in progress (or the mode of the select) keeps its score apart from the standard record. */
extern sb32 sc1PRemixScoreUsesOwnTable(void);

/** High score of a fighter in the table of the mode. @param fkind Fighter kind (0 when out of range). */
extern u32 sc1PRemixScoreGetHiScore(s32 fkind);

/** Bonus count of the best run of a fighter in the table of the mode. */
extern u32 sc1PRemixScoreGetBonusCount(s32 fkind);

/** Difficulty of the best finished run of a fighter in the table of the mode, 0 when it never finished. */
extern s32 sc1PRemixScoreGetBestDifficulty(s32 fkind);

/**
 * Keeps the score of the run when it beats the stored one (sc1PManagerTrySaveBackup on the table of the mode).
 *
 * @param is_complete_spgame TRUE when the run was finished.
 */
extern void sc1PRemixScoreTrySave(sb32 is_complete_spgame);

#endif

#endif
