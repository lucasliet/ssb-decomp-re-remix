#ifndef _MNREMIXITEMSWITCHLEGEND_H_
#define _MNREMIXITEMSWITCHLEGEND_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Pagination legend of the VS item switch screen (Item.asm item_switch_setup_, 944-975): R button and arrow for the next
 * page, L button and ": All On/Off" text for the toggle-all shortcut. The Render.draw_texture_at_offset and
 * Render.draw_string arguments are copied as hex floats.
 */

/** GObj link and display list link of the screen that carry the legend sprites (the decal camera draws on top). */
#define MN_REMIX_ITEM_SWITCH_LEGEND_GOBJ_LINK 3
#define MN_REMIX_ITEM_SWITCH_LEGEND_DL_LINK 4

/** Colour (sprite PRIM) and palette (sprite ENV) of the L and R button sprites, and their scale (0x3F400000). */
#define MN_REMIX_ITEM_SWITCH_BUTTON_TINT 0x848484FFU
#define MN_REMIX_ITEM_SWITCH_BUTTON_PALETTE 0x303030FFU
#define MN_REMIX_ITEM_SWITCH_BUTTON_SCALE 0.75F

/** Upper left of the R button (0x43880000, 0x42190000), of the arrow (0x43900000, 0x42240000) and of the L button (0x434A0000, 0x42190000). */
#define MN_REMIX_ITEM_SWITCH_R_X 272.0F
#define MN_REMIX_ITEM_SWITCH_R_Y 38.25F
#define MN_REMIX_ITEM_SWITCH_ARROW_X 288.0F
#define MN_REMIX_ITEM_SWITCH_ARROW_Y 41.0F
#define MN_REMIX_ITEM_SWITCH_L_X 202.0F
#define MN_REMIX_ITEM_SWITCH_L_Y 38.25F

/** Arrow colour 0xFFAE00FF, palette 0 and scale 0x3F2F0000. */
#define MN_REMIX_ITEM_SWITCH_ARROW_TINT 0xFFAE00FFU
#define MN_REMIX_ITEM_SWITCH_ARROW_PALETTE 0x00000000U
#define MN_REMIX_ITEM_SWITCH_ARROW_SCALE 0.68359375F

/** toggle_all_items string and the draw_string arguments: X 0x43560000, Y 0x421E0000, colour 0xC0C0C0FF, scale 0x3F2147AE. */
#define MN_REMIX_ITEM_SWITCH_TEXT ": All On/Off"
#define MN_REMIX_ITEM_SWITCH_TEXT_X 214.0F
#define MN_REMIX_ITEM_SWITCH_TEXT_Y 39.5F
#define MN_REMIX_ITEM_SWITCH_TEXT_RGBA 0xC0C0C0FFU
#define MN_REMIX_ITEM_SWITCH_TEXT_SCALE 0.63F

/** Slot of remix_hud_text (port/remix/remix_hud_text.h) taken by the item switch screen. */
#define MN_REMIX_ITEM_SWITCH_TEXT_SLOT 4

/** Makes the button and arrow sprites of the legend; call once when the screen starts, after its cameras exist. */
extern void mnRemixItemSwitchLegendMake(void);

/** Keeps the legend text visible; call once per frame while the screen runs. */
extern void mnRemixItemSwitchLegendPut(void);

#endif

#endif
