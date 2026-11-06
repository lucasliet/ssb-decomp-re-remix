#ifndef _MNREMIXSPLASH_H_
#define _MNREMIXSPLASH_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Remix splash image that follows the N64 logo (Boot.asm splash_): the Congratulations scene with Character.id NONE, a jingle and a
 * timer of MN_REMIX_SPLASH_FRAMES frames. The scene is mncongra.c; A, B or START end it earlier and a 90 frame fade leads to the title.
 */

/**
 * Starts or clears the splash for the Congratulations scene being loaded.
 *
 * @param scene_prev Scene the Congratulations scene was entered from; the splash runs when it is nSCKindStartup.
 */
extern void mnRemixSplashStartScene(s32 scene_prev);

/** TRUE while the Congratulations scene shows the splash. */
extern sb32 mnRemixSplashIsActive(void);

/** Plays the jingle in place of the Congratulations announcer call and sets the BGM and FGM volume (Boot.asm splash_._fgm). */
extern void mnRemixSplashPlayJingle(void);

/**
 * Counts one frame of the splash (Boot.asm splash_._countdown).
 *
 * @return TRUE when the timer ran out and the splash ends as if a button was pressed.
 */
extern sb32 mnRemixSplashTickCountdown(void);

#endif

#endif
