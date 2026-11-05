#ifndef _SC1PTRAININGREMIXMENUDRAW_H_
#define _SC1PTRAININGREMIXMENUDRAW_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Drawing of the Remix Training Z menu (Training.asm setup_ 1639-1648 and Menu.asm draw_ 530-720). The background, the modal frame
 * and the R / Z page glyphs are game objects of the Remix group 0x16 (custom menu group); the rows, the pagination text, the cursor
 * and its underline are the HUD text overlay with the asm positions: menu at (68, 50), 23 characters wide, 12 rows of 12 pixels,
 * scale 0.921875, label at x 76 and value right aligned at x 252.
 */

/** Scene start, after the vanilla pause menu exists: makes the objects of group 0x16, hidden. */
extern void sc1PTrainingRemixMenuDrawMake(void);

/**
 * Every frame: shows or hides group 0x16 and, while the menu is open, puts the rows of the current page, the pagination text and the
 * cursor.
 *
 * @param is_open TRUE while the Z menu is open.
 */
extern void sc1PTrainingRemixMenuDrawUpdate(sb32 is_open);

#endif

#endif
