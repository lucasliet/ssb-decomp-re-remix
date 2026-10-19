/**
 * grremixblastzone.c - Remix blast zone toggles for the vanilla physics map. See ft/ftremix/ftremixhooks.h.
 */

#ifdef PORT

#include "gm/gmremixtoggle.h"
#include <ssb_types.h>

/* Toggles.entry_blastzone_warp: bit 1 (value & 2) is the top/bottom warp. */
#define GRREMIX_BLASTZONE_WARP_TOP_BOTTOM ((gmRemixToggle(blastzone_warp) & 2) != 0)

/**
 * TRUE when the altitude warning sound of the bottom blast zone is skipped (Blastzone.asm skip_bottom_blast_sfx).
 */
sb32 grRemixBlastZoneSkipsBottomSfx(void)
{
    return GRREMIX_BLASTZONE_WARP_TOP_BOTTOM;
}

#endif
