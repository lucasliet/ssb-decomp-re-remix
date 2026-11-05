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

/**
 * Same as ifRemixSpriteMake for a sprite of a file the caller already loaded (the CSS files, for example).
 *
 * @param file Loaded file data.
 * @param sprite_offset Offset of the Sprite header in the file.
 * @return The GObj, or NULL when file is NULL.
 */
extern GObj *ifRemixSpriteMakeFromFile(void *file, u32 sprite_offset, s32 gobj_link, s32 dl_link, f32 x, f32 y, u32 tint_rgba,
                                       u32 palette_rgba, f32 scale);

/**
 * File data of a reloc file, loaded into the scene heap on first use and shared with the sprites of the same file (the KOTH hill
 * model and its meter frame are both in 0x14F3).
 *
 * @param file_id Reloc file id.
 * @return The file data, or NULL when the table of loaded files is full.
 */
extern void *ifRemixSpriteGetFile(u32 file_id);

/** Forgets the loaded sprite files; the scene heap that held them is gone. Called at each scene change. */
extern void ifRemixSpriteSceneReset(void);

#endif

#endif
