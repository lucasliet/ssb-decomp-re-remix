#ifndef _MNREMIXVSSTATSPAGES_H_
#define _MNREMIXVSSTATSPAGES_H_

#include <ssb_types.h>

/**
 * Draws one page of the VS match stats (VsStats.asm draw_port_headers_, draw_line_, draw_underline_ and the page 1 and
 * page 2 sections of setup_) with the overlay text and bar helpers. Call every frame while the page is shown.
 *
 * @param page 0 for the damage and combo page, 1 for the Z-cancel, tech, ledge and special move page.
 */
void mnRemixVSStatsPagesDraw(s32 page);

#endif
