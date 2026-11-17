#ifndef _MNREMIXVSSTATSPAGES_H_
#define _MNREMIXVSSTATSPAGES_H_

#include <ssb_types.h>

/* Slots of the page text: labels from 48 to the last slot (a page puts at most 73) and rectangles (stripes and underlines)
 * from 1, the ranges the text layer of the pages binds. */
#define MNREMIX_VSSTATS_PAGES_LABEL_SLOT_FIRST 48
#define MNREMIX_VSSTATS_PAGES_RECT_SLOT_FIRST 1

/**
 * Draws one page of the VS match stats (VsStats.asm draw_port_headers_, draw_line_, draw_underline_ and the page 1 and
 * page 2 sections of setup_) with the layer text and bar helpers. Call every frame while the page is shown.
 *
 * @param page 0 for the damage and combo page, 1 for the Z-cancel, tech, ledge and special move page.
 */
void mnRemixVSStatsPagesDraw(s32 page);

/**
 * Starts the pages of a new results screen: the first row takes the stripe state the previous results screen left, because
 * VsStats.asm never resets stripe_on. Call once when the results screen is set up.
 */
void mnRemixVSStatsPagesStartScreen(void);

#endif
