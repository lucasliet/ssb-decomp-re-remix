#ifndef _SC1PREMIXBONUSEXIT_H_
#define _SC1PREMIXBONUSEXIT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Leaves the Bonus 2 CSS for the 1P game when a Remix 1P mode asked for it (bonus2_css_2_1p): steps the RNG, then moves the
 * scene to the 1P game with the CSS fighter and costume. The standard bonus stages are left alone.
 *
 * @param fkind Fighter chosen in the CSS.
 * @param costume Costume chosen in the CSS.
 * @return TRUE when the scene now is the 1P game, FALSE when the caller keeps the vanilla bonus stage.
 */
extern sb32 sc1PRemixModeExitBonusCSS(s32 fkind, s32 costume);

/** Steps the RNG 1 to 64 times by the frame count of the screen, so two runs started at different moments get different paths. */
extern void sc1PRemixModeAdvanceSeed(void);

/**
 * Ends a run of a Remix bonus mode (Multi-Man, Cruel, Home-Run Contest): the scene goes back to the Bonus 2 CSS, or restarts the
 * 1P game when the run ended through the reset button. The reset flag is cleared.
 *
 * @return TRUE when the run restarts, FALSE when the Bonus 2 CSS comes back.
 */
extern sb32 sc1PRemixModeLeaveRun(void);

#endif

#endif
