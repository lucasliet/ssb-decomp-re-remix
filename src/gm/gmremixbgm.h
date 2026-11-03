#ifndef _GMREMIXBGM_H_
#define _GMREMIXBGM_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Runtime BGM state of the Remix (BGM.asm current_track and default_track, the vanilla current and stage tracks). The
 * stage default and current tracks are gMPCollisionBGMDefault and gMPCollisionBGMCurrent; this module adds the Remix
 * default (before the alternate and random picks) and the stage alternates, and changes the song while a match runs.
 */

/**
 * Records the tracks of the stage being loaded (BGM.asm apply_alt_or_random_music_).
 *
 * @param gkind Stage id.
 * @param default_track Default track of the stage (before the alternate roll).
 * @param current_track Track the stage will play after the alternate and random picks.
 */
extern void gmRemixBgmSetStageTracks(s32 gkind, u32 default_track, u32 current_track);

/**
 * Changes the match song: it becomes the stage track and the current track, and plays unless it is already the stage
 * track (Pause.asm set_alternate_music_ _save, Hazards.asm time_twister START_NEW_BGM). Item music is left alone.
 *
 * @param track BGM id.
 * @return TRUE when the song changed.
 */
extern sb32 gmRemixBgmChange(u32 track);

/**
 * Pause D-pad song change (Pause.asm set_alternate_music_): Right plays the next alternate of the stage, Down a random
 * track. The first press acts after 2 frames, a held direction repeats every 30. Call it every pause frame.
 *
 * @param button_hold Buttons of the player who paused.
 */
extern void gmRemixBgmPauseUpdate(u16 button_hold);

/**
 * Fades the match song out (Hazards.asm time_twister_bg_fade_ at the begin-fade frame) unless item music plays.
 *
 * @param frames Length of the fade.
 */
extern void gmRemixBgmFadeOut(s32 frames);

#endif

#endif
