#ifndef _SC1PREMIXHRC_H_
#define _SC1PREMIXHRC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Home-Run Contest (SinglePlayerModes.asm, HRC.asm): the first 1P stage turned into the Home-Run Contest stage with a Sandbag
 * as the only opponent, one stock, no items and ten seconds on the match timer (counted by ifcommon.c, see the timer limit in sc1premixhrcrules.h).
 */

/** TRUE while the 1P game being played is the Home-Run Contest. */
extern sb32 sc1PRemixHrcIsActive(void);

/** Applies the contest rules to the battle of the stage being set up: the contest stage, no items, no time limit. */
extern void sc1PRemixHrcSetupBattle(void);

/** Gives the 1P game player the single stock of the contest; no-op outside it. */
extern void sc1PRemixHrcSetupPlayer(s32 player);

/** Makes the opponent of the contest the Sandbag; no-op outside it. */
extern void sc1PRemixHrcSetupEnemy(s32 player);

/** TRUE for the item that never leaves the contest by the random despawn of a landed item: the Home-Run Bat (HRC.asm prevent_item_despawn_). */
extern sb32 sc1PRemixHrcKeepsItem(GObj *item_gobj);

/**
 * Plays "Are You Ready?" in the contest (HRC.asm play_are_you_ready_) and gives the tics to wait before "Go!".
 *
 * @param default_tics Tics the stage waits outside the contest.
 * @return The tics to wait: a little longer in the contest, default_tics elsewhere.
 */
extern s32 sc1PRemixHrcPlayReady(s32 default_tics);

/** Ends the run of the contest: back to the Bonus 2 CSS, or a restart through reset; no-op outside it. */
extern void sc1PRemixHrcLeaveRun(void);

#endif

#endif
