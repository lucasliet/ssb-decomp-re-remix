#ifndef _GRREMIXTIMETWISTER_H_
#define _GRREMIXTIMETWISTER_H_

/*
 * Time Twister background fade (Hazards.asm time_twister_setup_, time_twister_bg_fade_): a full screen fade
 * rectangle that runs the warp transition when the warp ball appears.
 */

#include <ssb_types.h>

#ifdef PORT

void grRemixTimeTwisterSetup(void);

/**
 * Turns the song swaps of the warps off for the rest of the match (Pause.asm set_alternate_music_ _save clears
 * Hazards.time_twister_bgm_change): the player picked a song in the pause, so the warps keep it.
 */
void grRemixTimeTwisterCancelBgmChange(void);

#endif

#endif
