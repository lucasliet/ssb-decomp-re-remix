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

#endif

#endif
