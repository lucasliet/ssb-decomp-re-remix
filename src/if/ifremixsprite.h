#ifndef _IFREMIXSPRITE_H_
#define _IFREMIXSPRITE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objtypes.h>
#include "if/ifremixspriterules.h"

/*
 * Remix button and extension sprites as native SObjs (Render.draw_texture_at_offset, TEXTURE_INIT_ and TEXTURE_RENDER_
 * in the asm; docs/remix/notes/f15_button_sprites.md). The sprite is drawn by the game display list, so it sits under
 * the ImGui text overlay (remix_hud_text) and above everything the same camera draws earlier.
 */

/**
 * Makes a GObj with one sprite. The asm "group" is the GObj link (gmRemixGroupDisplaySet hides or shows a whole group);
 * the display link must be drawn by a camera of the scene.
 *
 * @param id IFRemixSpriteId of the sprite.
 * @param gobj_link GObj link (asm group) the sprite joins.
 * @param dl_link Display list link a camera draws.
 * @param x Left edge in N64 screen units.
 * @param y Top edge in N64 screen units.
 * @param tint_rgba 0xRRGGBBAA colour argument of the asm (sprite PRIM colour).
 * @param palette_rgba 0xRRGGBBAA palette argument of the asm (sprite ENV colour).
 * @param scale Scale on both axes.
 * @return The GObj, or NULL when id is invalid or the file is missing (the reason goes to the log).
 */
extern GObj *ifRemixSpriteMake(s32 id, s32 gobj_link, s32 dl_link, f32 x, f32 y, u32 tint_rgba, u32 palette_rgba, f32 scale);

/** Forgets the loaded sprite files; the scene heap that held them is gone. Called at each scene change. */
extern void ifRemixSpriteSceneReset(void);

#endif

#endif
