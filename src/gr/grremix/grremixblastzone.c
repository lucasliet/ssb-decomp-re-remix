/**
 * grremixblastzone.c - Remix blast zone toggles for the vanilla physics map. See ft/ftremix/ftremixhooks.h.
 */

#ifdef PORT

#include <ssb_types.h>

/*
 * REMIX-TODO(toggles): Toggles.entry_blastzone_warp (Remix default 0 = OFF, Toggles.asm) has no port accessor yet.
 * Bit 2 of the toggle is the top/bottom warp. The warp itself (Blastzone.asm loop_screen_*) is not ported.
 */
#define GRREMIX_BLASTZONE_WARP_TOP_BOTTOM FALSE

/**
 * TRUE when the altitude warning sound of the bottom blast zone is skipped (Blastzone.asm skip_bottom_blast_sfx).
 */
sb32 grRemixBlastZoneSkipsBottomSfx(void)
{
    return GRREMIX_BLASTZONE_WARP_TOP_BOTTOM;
}

#endif
