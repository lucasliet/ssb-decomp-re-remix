#ifndef _SCREMIXSCREENS_H_
#define _SCREMIXSCREENS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Other Screens of the Remix settings menu (Toggles.asm head_other_screens_, view_screen_, view_credits_, view_gallery), opened from
 * the port menu. The port menu cannot include the decomp headers, so it declares scRemixScreensOpen by hand with these values.
 */

#define SC_REMIX_SCREEN_DEBUG 0
#define SC_REMIX_SCREEN_INTRO 1
#define SC_REMIX_SCREEN_HOW_TO_PLAY 2
#define SC_REMIX_SCREEN_CREDITS 3
#define SC_REMIX_SCREEN_GALLERY 4

/**
 * Leaves the current scene and enters one of the Other Screens.
 *
 * @param screen SC_REMIX_SCREEN_* value.
 * @param gallery_page Gallery picture to start on, 0 to 39; only read for SC_REMIX_SCREEN_GALLERY.
 */
extern void scRemixScreensOpen(s32 screen, s32 gallery_page);

#endif

#endif
