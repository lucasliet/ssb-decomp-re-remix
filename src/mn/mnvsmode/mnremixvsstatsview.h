#ifndef _MNREMIXVSSTATSVIEW_H_
#define _MNREMIXVSSTATSVIEW_H_

#include <ssb_types.h>

/**
 * Resets the stats view when the VS Results screen starts (VsStats.asm setup_: always page 1, hidden).
 */
void mnRemixVSStatsViewSetup(void);

/**
 * Runs once per Results frame: reveals the Press A prompt, handles A (show), B (hide) and R (next page), and draws the
 * prompt or the current page (VsStats.asm check_menu_toggle_).
 *
 * @param results_tics Frames elapsed on the Results screen.
 * @param allow_exit_wait Frames until START exits the Results screen.
 */
void mnRemixVSStatsViewRun(s32 results_tics, s32 allow_exit_wait);

#endif
