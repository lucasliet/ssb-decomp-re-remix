#ifndef _SC1PREMIXHRC_H_
#define _SC1PREMIXHRC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Home-Run Contest (SinglePlayerModes.asm, HRC.asm): the first 1P stage turned into the Home-Run Contest stage with a Sandbag
 * as the only opponent, one stock, no items and no time limit.
 */

/** TRUE while the 1P game being played is the Home-Run Contest. */
extern sb32 sc1PRemixHrcIsActive(void);

/** Applies the contest rules to the battle of the stage being set up: the contest stage, no items, no time limit. */
extern void sc1PRemixHrcSetupBattle(void);

/** Gives the 1P game player the single stock of the contest; no-op outside it. */
extern void sc1PRemixHrcSetupPlayer(s32 player);

/** Makes the opponent of the contest the Sandbag; no-op outside it. */
extern void sc1PRemixHrcSetupEnemy(s32 player);

/** Ends the run of the contest: back to the Bonus 2 CSS, or a restart through reset; no-op outside it. */
extern void sc1PRemixHrcLeaveRun(void);

#endif

#endif
