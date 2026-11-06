#ifndef _SC1PBONUSTIMER_H_
#define _SC1PBONUSTIMER_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Whether the timer of the running 1P game is the bonus count-up timer instead of the match timer (SinglePlayerModes.asm
 * _load_timer): Bonus 3, the Home-Run Contest (which counts down 10 seconds in the same layout) and every Practice run.
 * sc1PGameInitTimeUpMessage makes sc1PBonusStageMakeCountUpTimer when this is TRUE.
 *
 * @return TRUE when the bonus timer is used.
 */
extern sb32 sc1PBonusTimerIsCountUpRun(void);

#endif

#endif
