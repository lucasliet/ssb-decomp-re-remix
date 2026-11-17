#ifndef _IFREMIXTEXTLAYER_H_
#define _IFREMIXTEXTLAYER_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objtypes.h>

/*
 * Remix text drawn by the game renderer in the room of the asm (Render.draw_string_, Render.draw_rectangle_; docs/remix/
 * text_layers.md, "Draw path"). A layer is one GObj: its display function draws the remix_hud_text slots bound to it, the
 * rectangles first, then the bars, then the labels in slot order, each glyph as a sprite of the Remix font file (0xA15)
 * through the game's sprite functions. The owner keeps calling remix_hud_text_put and friends every frame.
 *
 * The display link must be drawn by a sprite camera of the scene (lbCommonDrawSprite), which clips to its viewport, at most
 * 10..310 x 10..230.
 */

/**
 * Makes a layer. Call it after the scene made its own objects of that display link, as Render.setup_ runs after the scene
 * setup: among equal priorities the newest object draws last, so over the others.
 *
 * @param gobj_link GObj link (asm group) the layer joins; gmRemixGroupDisplaySet hides or shows it with the group.
 * @param dl_link Display link (asm room) a sprite camera of the scene draws.
 * @param dl_priority Priority inside the display link; GOBJ_PRIORITY_DEFAULT is the asm default (drawn last).
 * @return The layer GObj, or NULL when the text stays on the overlay: the font file or its metrics are missing, the scene
 *         has no layer left (both go to the log) or SSB64_REMIX_TEXT_OVERLAY=1 is set.
 */
extern GObj *ifRemixTextLayerMake(s32 gobj_link, s32 dl_link, u32 dl_priority);

/**
 * Binds label slots to a layer; from then on the layer draws them and the overlay does not.
 *
 * @param layer_gobj Layer from ifRemixTextLayerMake; NULL does nothing.
 * @param first_slot First label slot.
 * @param count Number of slots.
 */
extern void ifRemixTextLayerBindLabels(GObj *layer_gobj, s32 first_slot, s32 count);

/** Same as ifRemixTextLayerBindLabels for rectangle slots (remix_hud_rect_put). */
extern void ifRemixTextLayerBindRects(GObj *layer_gobj, s32 first_slot, s32 count);

/** Same as ifRemixTextLayerBindLabels for bar slots (remix_hud_bar_put). */
extern void ifRemixTextLayerBindBars(GObj *layer_gobj, s32 first_slot, s32 count);

#endif

#endif
