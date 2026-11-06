#ifndef _SC1PREMIXMUSICRULES_H_
#define _SC1PREMIXMUSICRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Music rules of the Remix 1P modes (SinglePlayerModes.asm skip_cflag_music_function, set_bgm, skip_cflag_music_function_2),
 * kept apart so the headless unit tests (tests/remix) run them. Stage numbers are the asm 1P progress values (STAGE_FLAG).
 */

/* Progress values of the Remix 1P run the music rules look at. */
#define SC1PREMIX_MUSIC_STAGE_DUO 4
#define SC1PREMIX_MUSIC_STAGE_MINI_BOSS 0x0A
#define SC1PREMIX_MUSIC_STAGE_FINAL 0x0D

/* Result of sc1PRemixMusicPick when the mode keeps the BGM of the stage header. */
#define SC1PREMIX_MUSIC_KEEP_STAGE (-1)

/* Remix MIDI ids (docs/remix/audio.md) written to the stage header. */
#define SC1PREMIX_BGM_MULTIMAN 0x7E
#define SC1PREMIX_BGM_CRUEL 0x7F
#define SC1PREMIX_BGM_TARGETS 0x8E
#define SC1PREMIX_BGM_ULTIMATE_BOWSER 0x4D
#define SC1PREMIX_BGM_MULTIMAN_2 0xB0
#define SC1PREMIX_BGM_OPEN_YOUR_HEART 0xB7
#define SC1PREMIX_BGM_LIVE_AND_LEARN 0xC0
#define SC1PREMIX_BGM_EVERYTHING 0xC7
#define SC1PREMIX_BGM_ALOOF_SOLDIER 0x12B
#define SC1PREMIX_BGM_DANGEROUS_FOE 0x12D
#define SC1PREMIX_BGM_PIKA_CUP 0x12F
#define SC1PREMIX_BGM_WIZPIG 0x132
#define SC1PREMIX_BGM_BATTLE_AMONG_FRIENDS 0x162

/*
 * Rolls sc1PRemixMusicPick takes: Multi-Man chooses between two songs, the Super Sonic fight between three. Only these two
 * cases consume a random number, as in the asm.
 */
#define SC1PREMIX_MUSIC_MULTIMAN_ROLLS 2
#define SC1PREMIX_MUSIC_SONIC_ROLLS 3

/**
 * Whether the 1P game starts the stage BGM at once instead of waiting for GO (skip_cflag_music_function, and the same test of
 * skip_cflag_music_function_2 at GO): Multi-Man, Cruel Multi-Man, the Home-Run Contest and the mini boss stage of Remix 1P play
 * the music with the crowd cheer, whatever the stage kind.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @param progress Stage the 1P run is on.
 * @return Non-zero when the BGM starts at once.
 */
extern int sc1PRemixModePlaysMusicAtOnce(int mode, int progress);

/**
 * Random numbers sc1PRemixMusicPick needs for the stage.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @param progress Stage the 1P run is on.
 * @param gkind Remix stage id (nGRRemixStage_*) the 1P stage plays on.
 * @return 0, SC1PREMIX_MUSIC_MULTIMAN_ROLLS or SC1PREMIX_MUSIC_SONIC_ROLLS.
 */
extern int sc1PRemixMusicRollCount(int mode, int progress, int gkind);

/**
 * Song the mode forces on the stage (set_bgm).
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @param progress Stage the 1P run is on.
 * @param gkind Remix stage id (nGRRemixStage_*) the 1P stage plays on.
 * @param roll Random number below sc1PRemixMusicRollCount, 0 when the count is 0.
 * @return The BGM id, or SC1PREMIX_MUSIC_KEEP_STAGE when the header song plays.
 */
extern int sc1PRemixMusicPick(int mode, int progress, int gkind, int roll);

#endif

#endif
