#ifndef _GMREMIXPRACTICERULES_H_
#define _GMREMIXPRACTICERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix Practice_1P panel option (Practice_1P.asm), so the headless unit tests (tests/remix) can run them
 * without game state. */

#define GMREMIX_PRACTICE_MODE_STANDARD 0
#define GMREMIX_PRACTICE_MODE_REMIX_1P 4
#define GMREMIX_PRACTICE_BUTTON_L 0x0020

/**
 * TRUE when a 1P run is a practice run (Practice_1P.asm _1p_practice _check_mode): the run is the standard 1P Game or the
 * Remix 1P, and the Practice panel value of the human port is not OFF (0).
 *
 * @param mode SC1PREMIX_MODE_* of the run.
 * @param panel_value Practice_1P panel value of the human port.
 */
extern int gmRemixPracticeIsRunActive(int mode, int panel_value);

/**
 * TRUE when L is held in the button mask (hold_to_retry_ and hold_to_retry_stage_clear_ read the held state of L).
 *
 * @param button_hold Held buttons of the controllers, ORed.
 */
extern int gmRemixPracticeIsRetryHeld(unsigned int button_hold);

/**
 * TRUE when L pressed in the pause menu starts the retry (SinglePlayerModes.asm _reset, `_practice`): a practice run, in
 * the 1P Game scene.
 *
 * @param is_practice_active TRUE for a practice run.
 * @param is_1p_game_scene TRUE while the 1P Game battle scene runs.
 * @param button_tap Buttons pressed this frame.
 */
extern int gmRemixPracticeIsPauseRetry(int is_practice_active, int is_1p_game_scene, unsigned int button_tap);

#endif

#endif
