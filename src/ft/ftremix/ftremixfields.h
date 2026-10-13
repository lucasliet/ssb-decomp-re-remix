#ifndef _FTREMIXFIELDS_H_
#define _FTREMIXFIELDS_H_

/*
 * Remix routines use FTStruct words by N64 offset (for example `sw t0, 0x0B18(a0)`).
 * The port's FTStruct is laid out for LP64, so `(u8 *)fp + 0x0B18` is a different
 * field there. These accessors map the N64 offsets Remix uses as scratch words onto
 * the same place in the port struct:
 *
 * - 0x0ADC..0x0B13: passive_vars (union start 0x0ADC on N64)
 * - 0x0B18..0x0B4B: status_vars (union start 0x0B18 on N64)
 * - 0x0980: modelpart_status[2].modelpart_id_base (Kirby's hat id byte)
 *
 * Words keep their relative position inside each union, so Remix routines
 * that share a word still share it in the port.
 */

#include <ft/fighter.h>

#define FTREMIX_N64_OFFSET_KIRBY_HAT 0x0980

/** Address of the 32-bit word at N64 FTStruct offset `n64_offset`. Aborts on an unmapped offset. */
s32 *ftRemixFighterWord(FTStruct *fp, u32 n64_offset);

/** ftRemixFighterWord, read as an f32. */
f32 *ftRemixFighterFloat(FTStruct *fp, u32 n64_offset);

/** Kirby's hat id byte (N64 0x0980, `lb t0, 0x0980(a0)`). */
s8 *ftRemixFighterKirbyHat(FTStruct *fp);

#endif
