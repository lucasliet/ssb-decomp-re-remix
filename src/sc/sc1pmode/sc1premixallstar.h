#ifndef _SC1PREMIXALLSTAR_H_
#define _SC1PREMIXALLSTAR_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
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

/** Moves the progress past the fighters of the match just won. */
extern void sc1PRemixAllstarAdvance(void);

#endif

#endif
