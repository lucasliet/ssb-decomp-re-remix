#ifndef _SC1PREMIXMODERULES_H_
#define _SC1PREMIXMODERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Remix 1P modes that need no game state (SinglePlayerModes.asm bonus2_css_2_1p), kept apart so the headless
 * unit tests (tests/remix) run them.
 */

/**
 * Whether the Bonus 2 CSS of a Remix 1P mode hands over to the 1P game instead of the bonus stage: Bonus 3, Multi-Man, Cruel
 * Multi-Man and the Home-Run Contest. Remix 1P and All-Star use the 1P CSS and never reach the Bonus 2 CSS; the standard
 * bonus stages keep their own flow.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @return Non-zero when the Bonus 2 CSS starts the 1P game.
 */
extern int sc1PRemixModeLeavesBonusCSSFor1P(int mode);

/**
 * Times the RNG is stepped when a mode leaves the Bonus 2 CSS, so the polygon fighters of the first match differ between runs:
 * one to 64 steps by the frame count of the screen.
 *
 * @param frame_count Frames the CSS ran.
 * @return Steps to run, 1 to 64.
 */
extern unsigned int sc1PRemixModeSeedAdvanceCount(unsigned int frame_count);

#endif

#endif
