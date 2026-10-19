#ifndef _GMREMIXMUSIC_H_
#define _GMREMIXMUSIC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Scales a BGM or FGM volume by a master level (Toggles.entry_bgm_volume or entry_fgm_volume, 0 to 10) as the
 * Remix patches of 0x80020B38 and 0x80020E64 do. The product is truncated toward zero.
 *
 * @param volume Volume before the master level, 0 to 30720.
 * @param level Master level, 0 to 10. Level 0 silences the output.
 * @return The scaled volume.
 */
u32 gmRemixMasterVolume(u32 volume, s32 level);

#endif

#endif
