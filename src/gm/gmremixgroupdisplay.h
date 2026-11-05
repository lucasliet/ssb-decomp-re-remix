#ifndef _GMREMIXGROUPDISPLAY_H_
#define _GMREMIXGROUPDISPLAY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Remix display groups: show or hide every GObj of one common link list (Render.asm GROUP_TABLE = gGCCommonLinks).
 * Group ids used by the Remix: 0x0D menu instructions, 0x0E HUD, 0x0F and 0x10 pause legend, 0x16 results rows, 1 and 2
 * stage layers (see docs/remix/menus.md, render display groups).
 */

/** Render.toggle_group_display_: writes hide (1 or 0) as the flags word of every GObj in the group. */
extern void gmRemixGroupDisplaySet(s32 link_id, u32 hide);

/** SpecialZoom.temp_toggle_group_display_: hide saves each GObj's flags, show (hide = 0) restores them. */
extern void gmRemixGroupDisplayTempSet(s32 link_id, u32 hide);

#endif

#endif
