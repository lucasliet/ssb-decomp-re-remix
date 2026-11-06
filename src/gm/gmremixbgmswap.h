#ifndef _GMREMIXBGMSWAP_H_
#define _GMREMIXBGMSWAP_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Mid-match song swaps of the Remix: the Final Destination intro that gives way to the main song after 840 updates
 * (FD.asm music_fix_) and the Hurry Up song at 30 seconds left (BGM.asm thirty_second_music_swap).
 */

/**
 * Records that the stage song just started (BGM.asm play_music_ stores the update count for FD.asm music_fix_). Call it right
 * after the stage song plays.
 */
extern void gmRemixBgmSwapNoteStageSongPlayed(void);

/**
 * Per-update check of the Final Destination intro (FD.asm music_fix_, hooked in ifCommonBattleUpdateInterfaceAll): on the
 * Final Destination stages of VS (Final Destination, Final Destination DL and Final Tentation) and on the Final Stage of the
 * 1P standard and Remix 1P runs, 840 updates after the intro (BGM 24) started the stage song becomes the main song (BGM 25).
 * A song that started in between (item music) restarts the count, one update late.
 */
extern void gmRemixBgmSwapFdIntroUpdate(void);

/**
 * Hurry Up swap at 30 seconds left (thirty_second_music_swap): Mushroom Kingdom, SMB2 Overworld and Underground, wherever they
 * play, become their Hurry Up song. Call it every update while the time left is at most 30 seconds.
 */
extern void gmRemixBgmSwapHurryUpdate(void);

#endif

#endif
