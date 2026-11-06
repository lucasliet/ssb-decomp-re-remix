#ifndef _MNREMIXSPLASHRULES_H_
#define _MNREMIXSPLASHRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * State-free rules of the Remix splash image that follows the N64 logo (Boot.asm splash_), kept apart so the headless unit tests
 * (tests/remix) can run them.
 */

/** Frames the splash stays up before it ends by itself (Boot.asm splash_ `lli t8, 0x0200`). */
#define MN_REMIX_SPLASH_FRAMES 512

/** Controllers whose L button counts for the jingle (Boot.asm _fgm checks P1 to P4). */
#define MN_REMIX_SPLASH_PLAYER_COUNT 4

/** Jingle FGM ids (Boot.asm _fgm). */
#define MN_REMIX_SPLASH_FGM_NICE 0x0A0
#define MN_REMIX_SPLASH_FGM_ONE_L 0x2C2
#define MN_REMIX_SPLASH_FGM_ALL_L 0x2F9

/**
 * Jingle of the splash: a nice sound, another one while someone holds L, a third one while all four players hold L.
 *
 * @param held_l_count Number of players holding L, 0 to MN_REMIX_SPLASH_PLAYER_COUNT.
 * @return The FGM id.
 */
extern int mnRemixSplashRulesGetJingleFgm(int held_l_count);

/**
 * Countdown of one frame (Boot.asm splash_._countdown): the splash ends when the timer is already 0, otherwise it counts down.
 *
 * @param frames_left Frames left, read and written.
 * @return Non-zero when the splash ends this frame.
 */
extern int mnRemixSplashRulesTickCountdown(int *frames_left);

#endif

#endif
