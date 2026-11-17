#ifndef _IFREMIXPAUSELEGEND_H_
#define _IFREMIXPAUSELEGEND_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Makes the layer that draws the text of the pause legends and of the Gentlemen's Reset request in the pause link, over the pause
 * menu objects of every pause. Call once when the battle scene sets up its interface.
 */
void ifRemixPauseLegendMakeLayer(void);

/**
 * Starts the pause legends of a new pause: makes their sprites, with the R legend shown and the music and camera legends hidden
 * (Pause.asm show_legend_helper_).
 */
void ifRemixPauseLegendReset(void);

/** Ejects the legend sprites when the pause ends. */
void ifRemixPauseLegendEnd(void);

/** Hides the music and camera legends (the Gentlemen's Reset request destroys them, TwelveCharBattle.asm 3624-3638). */
void ifRemixPauseLegendHideMusicAndCamera(void);

/**
 * Runs the pause legend controls and draws the legends of the current frame (Pause.asm check_toggle_hud_press_ and
 * check_legend_toggle_press_). Call it once per frame of every pause: the asm hook runs before the pause kind test.
 *
 * @param button_tap Buttons pressed this frame of the pausing player.
 * @param button_hold Buttons held this frame of the pausing player.
 * @param is_camera_control TRUE in the default pause, where the camera controls are allowed; the camera legend stays hidden
 *        in the other pauses (_toggle_display_camera_controls).
 */
void ifRemixPauseLegendUpdate(u16 button_tap, u16 button_hold, sb32 is_camera_control);

#endif

#endif
