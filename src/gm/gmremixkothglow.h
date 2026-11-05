#ifndef _GMREMIXKOTHGLOW_H_
#define _GMREMIXKOTHGLOW_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fttypes.h>

/**
 * KingOfTheHill.asm track_score_ _apply_gfx_routines: a fighter that scores on the hill gets the pulsing overlay of its colour
 * (GFXRoutine.KOTH_RED, KOTH_BLUE, KOTH_GREEN, KOTH_YELLOW, or KOTH_GRAY for a CPU in free-for-all), unless it already has a
 * routine. The routine stays as the port override, so it starts again every time the animation ends.
 *
 * @param fighter_gobj Fighter that scored this frame.
 */
extern void gmRemixKothGlowApply(GObj *fighter_gobj);

#endif

#endif
