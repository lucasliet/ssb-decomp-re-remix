#ifndef _IFREMIXPAUSELEGEND_H_
#define _IFREMIXPAUSELEGEND_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/** Resets the pause legends for a new pause: the R legend shown, the camera legend hidden (Pause.asm show_legend_helper_). */
void ifRemixPauseLegendReset(void);

/**
 * Runs the pause legend controls and draws the legends of the current frame (Pause.asm check_toggle_hud_press_ and
 * check_legend_toggle_press_). Call it once per frame from the default pause, where the camera controls are allowed.
 *
 * @param button_tap Buttons pressed this frame of the pausing player.
 * @param button_hold Buttons held this frame of the pausing player.
 */
void ifRemixPauseLegendUpdate(u16 button_tap, u16 button_hold);

#endif

#endif
