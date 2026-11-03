#ifndef _GRREMIXSTAGEBG_H_
#define _GRREMIXSTAGEBG_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Access of the Remix stage hazards to the stage model and the stage background (Hazards.asm time_twister_* and
 * scuttle_town_*): they walk the first GObj of link 2 (the stage model, RAM 0x800466F8) and swap the bitmap of the
 * wallpaper sprite (the room 0 object at RAM 0x80046800, field +0x44 = Sprite.bitmap).
 */

/**
 * Bitmap token of a stage background and the fix-up of its sprite, read from an image header of the ground data (asm:
 * header field -> image header -> +0x34). Calling it again for the same header is harmless.
 *
 * @param ground_field Offset in the ground data of the pointer to the image header.
 * @return The Sprite.bitmap token to give to grRemixStageBgSetBitmap, 0 when the header is missing.
 */
extern u32 grRemixStageBgBitmapToken(u32 ground_field);

/**
 * Replaces the bitmap of the stage wallpaper (asm: sw bg_address, 0x44 of the bg image object).
 *
 * @param bitmap_token Value from grRemixStageBgBitmapToken.
 * @return TRUE when the stage has a wallpaper sprite and its bitmap was replaced.
 */
extern sb32 grRemixStageBgSetBitmap(u32 bitmap_token);

/**
 * Time Twister warp ball (asm time_twister_bg_fade_ walk): top joint, first joint, four siblings along.
 *
 * @param out_bg_index Receives the current image index of the ball, the index of the next stage background.
 * @return TRUE when the ball is displayed and the index was read; FALSE while it is hidden or missing.
 */
extern sb32 grRemixStageReadWarpBallBgIndex(s32 *out_bg_index);

/**
 * Raw bits of the palette index of the stage model first joint (asm scuttle_town_bg_fade_ walk: top joint, first joint,
 * part image, +0x88). Zero while the model is missing.
 */
extern u32 grRemixStageReadFirstJointPaletteBits(void);

#endif

#endif
