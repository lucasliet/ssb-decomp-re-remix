#if defined(PORT) && defined(REGION_REMIX)

#ifndef _FTREMIXPARTIMAGEPALETTE_H_
#define _FTREMIXPARTIMAGEPALETTE_H_

#include <ssb_types.h>

/*
 * Palette arrays of the Remix part images (Costumes.asm palette arrays, Shield.asm table_palettes).
 *
 * The Remix data keeps a palette array as N64 words (one RAM address per palette) in the RAM image, and a palette as
 * RGBA5551 halfwords in the image's native word order. MObjSub.palettes of the port wants a token of an array of tokens,
 * and the renderer reads a palette as big-endian bytes, so each palette is copied into a pool as big-endian bytes and each
 * array gets a slot array of tokens of those copies. Both stay valid across scenes.
 */

#define FTREMIX_PALETTE_ARRAY_SLOTS 16

/**
 * Token of a big-endian copy of the palette at the N64 address `src_addr` of the RAM image, made once per address and reused.
 * Returns 0 when the address is 0 or the pool is full. Usable as the token of a sprite LUT or of an MObj palette slot.
 */
u32 ftRemixPartImageGetPaletteToken(u32 src_addr);

/**
 * Token of a slot array whose entry i is the token of a big-endian copy of the palette that the N64 word array at
 * `array_addr` lists at index i. Entries with a zero address stay zero. Built once per array address and reused.
 * Returns 0 when the array lies outside the image, `count` is not 1 to FTREMIX_PALETTE_ARRAY_SLOTS or a pool is full.
 */
u32 ftRemixPartImageGetPaletteArrayToken(u32 array_addr, s32 count);

#endif

#endif
