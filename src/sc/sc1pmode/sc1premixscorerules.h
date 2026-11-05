#ifndef _SC1PREMIXSCORERULES_H_
#define _SC1PREMIXSCORERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Which table keeps the 1P game score of a run (SinglePlayer.asm extend_high_score_1p_*, :1157-1357): Remix 1P and All-Star have a
 * table each, with one row per fighter, and every other run keeps the standard record. Kept apart so the headless unit tests
 * (tests/remix) run it.
 */
#define SC1PREMIX_SCORE_TABLE_STANDARD 0
#define SC1PREMIX_SCORE_TABLE_REMIX_1P 1
#define SC1PREMIX_SCORE_TABLE_ALLSTAR 2

/**
 * Table of a mode.
 *
 * @param mode SC1PREMIX_MODE_*.
 * @return SC1PREMIX_SCORE_TABLE_*.
 */
extern int sc1PRemixScoreTableOf(int mode);

/**
 * Whether the score of a run replaces the stored one: only a strictly higher score does (sc1PManagerTrySaveBackup).
 *
 * @param stored High score in the table.
 * @param score Score of the run.
 */
extern int sc1PRemixScoreReplaces(unsigned int stored, unsigned int score);

#endif

#endif
