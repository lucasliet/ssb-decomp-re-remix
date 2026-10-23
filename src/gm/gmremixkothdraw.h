#ifndef _GMREMIXKOTHDRAW_H_
#define _GMREMIXKOTHDRAW_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * King of the Hill on screen: the hill strip over the floor line (render_ground_gfx_, drawn as a display list in the
 * world) and the score meters (update_indicators_, drawn by the port overlay remix_hud_text).
 */

/** Creates the object that draws the hill strip every frame. */
extern void gmRemixKothDrawMake(void);

/** Match start: sets the colour of each meter and clears the full-bar flash. */
extern void gmRemixKothHudSetup(void);

/** Every frame: puts the meter of each port (free-for-all) or team in the overlay. */
extern void gmRemixKothHudUpdate(void);

/** Goal of the match in frames on the hill (the CSS goal in seconds times 60). */
extern s32 gmRemixKothGetTargetFrames(void);

/** Frames on the hill of the team (the port in free-for-all) `key`. */
extern s32 gmRemixKothGetScoreOf(s32 key);

#endif

#endif
