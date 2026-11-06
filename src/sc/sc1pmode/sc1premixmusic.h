#ifndef _SC1PREMIXMUSIC_H_
#define _SC1PREMIXMUSIC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * BGM of the Remix 1P modes on the 1P game (SinglePlayerModes.asm skip_cflag_music_function, set_bgm,
 * skip_cflag_music_function_2): Multi-Man, Cruel, the Home-Run Contest and the mini boss stage of Remix 1P start their music
 * with the crowd cheer instead of waiting for GO, and each mode forces its own song.
 */

/** TRUE when the run being played starts the stage music at once instead of at GO; FALSE outside the Remix modes. */
extern sb32 sc1PRemixMusicStartsAtOnce(void);

/**
 * Writes the song the mode forces into the header of the stage ground (set_bgm), so the next mpCollisionSetPlayBGM plays it.
 * Keeps the header song for the standard 1P run and for the stages the mode does not override. Rolls the random number the
 * asm rolls (Multi-Man: two songs, Super Sonic: three), and nothing else.
 */
extern void sc1PRemixMusicApplyModeSong(void);

#endif

#endif
