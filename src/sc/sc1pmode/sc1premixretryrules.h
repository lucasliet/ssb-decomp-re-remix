#ifndef _SC1PREMIXRETRYRULES_H_
#define _SC1PREMIXRETRYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the L retry of the Remix bonus runs (SinglePlayerModes.asm _reset_button, _reset and _end_match_screen, Practice_1P.asm
 * hold_l_to_retry_), kept apart so the headless unit tests (tests/remix) run them. Bonus 3, Multi-Man, Cruel Multi-Man and the
 * Home-Run Contest run on the asm fake screen 0x77; Bonus 1 and 2 practice runs keep their own screen (0x35).
 */

/**
 * Whether the mode runs on the fake screen 0x77 of the asm: Bonus 3, Multi-Man, Cruel Multi-Man and the Home-Run Contest. These
 * show the "L: RETRY" legend and retry on L. All-Star has its own flow and Remix 1P is a plain 1P game.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @return Non-zero for the fake screen modes.
 */
extern int sc1PRemixRetryIsFakeScreenMode(int mode);

/**
 * Whether L tapped in the pause menu retries the run of a fake screen mode (_reset, `_multiman`).
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @param is_1p_game_scene Non-zero while the 1P Game battle scene runs.
 * @param button_tap Buttons pressed this frame by the pausing player.
 * @return Non-zero when the retry starts.
 */
extern int sc1PRemixRetryIsPauseRetry(int mode, int is_1p_game_scene, unsigned int button_tap);

/**
 * Whether L held while GAME SET shows retries the run (hold_l_to_retry_): a Bonus 1 or Bonus 2 practice run (the screen before the
 * stage was the Bonus CSS), or a fake screen mode run. A Bonus stage of the 1P game itself never retries.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @param is_1p_game_scene Non-zero while the 1P Game battle scene runs.
 * @param is_bonus_practice_scene Non-zero while a Bonus stage runs that was entered from the Bonus 1 or Bonus 2 CSS.
 * @param button_hold Held buttons of the controllers, ORed.
 * @return Non-zero when the retry starts.
 */
extern int sc1PRemixRetryIsGameSetRetry(int mode, int is_1p_game_scene, int is_bonus_practice_scene, unsigned int button_hold);

#endif

#endif
