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
 * TRUE when a controller holds L and no other button (hold_l_to_retry_ and hold_l_to_retry_stage_clear_ call
 * Joypad.check_buttons_all_ with "match any" false, which compares held == mask for each controller).
 *
 * @param button_hold Held buttons of one controller.
 */
extern int gmRemixPracticeIsRetryHeld(unsigned int button_hold);

/**
 * TRUE when the scene showing GAME SET takes the L retry of a practice run (hold_l_to_retry_ _check_1p_mode and
 * _check_bonus_not_1p): the 1P Game battle, or a Bonus stage that was not entered from the Bonus 1 or 2 CSS, which is a Bonus
 * stage of the 1P run.
 *
 * @param is_1p_game_scene Non-zero while the 1P Game battle scene runs.
 * @param is_bonus_stage_scene Non-zero while a Bonus stage scene runs.
 * @param is_from_bonus_css Non-zero when the scene before it was the Bonus 1 or Bonus 2 CSS.
 */
extern int gmRemixPracticeIsGameSetRetryScene(int is_1p_game_scene, int is_bonus_stage_scene, int is_from_bonus_css);

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
