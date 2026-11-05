#ifndef _MNREMIXSTAGEPAGER_H_
#define _MNREMIXSTAGEPAGER_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Makes the page strip under the grid of the stage select (Stages.asm setup_): the left and right arrows, the Z and R button
 * images that switch the page. The "Page N" text is drawn every frame by mnRemixStagePagerUpdate.
 *
 * @param arrows_file Loaded llMNPlayersCommonFileID file (the page arrows live there).
 */
extern void mnRemixStagePagerMake(void *arrows_file);

/**
 * Draws "Page N" (Render.draw_string of string_page and draw_number_adjusted of page_number, adjusted by 1). Call every frame:
 * the text overlay lives two frames.
 *
 * @param page Current page, 0 based.
 */
extern void mnRemixStagePagerUpdate(s32 page);

#endif

#endif
