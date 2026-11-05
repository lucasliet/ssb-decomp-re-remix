#ifndef _SC1PTRAININGSBMRULES_H_
#define _SC1PTRAININGSBMRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rule of the Single Button Mode gate of the Training screen (SingleButtonMode.asm _check_pause_training), kept apart so the headless
 * unit tests (tests/remix) run it. Training.toggle_menu is byte 0x11 of the Training battle state, which is the game status:
 * 1 both menus down (the match runs), 2 vanilla pause menu up, 3 Z menu up.
 */

#define SC1PTRAININGSBM_MENU_BOTH_DOWN 1
#define SC1PTRAININGSBM_MENU_SSB_UP 2
#define SC1PTRAININGSBM_MENU_CUSTOM_UP 3

/**
 * Value of Training.toggle_menu from what the port keeps: the game status of the Training battle, and the Z menu flag, which takes
 * the value 3 whatever the status is.
 *
 * @param game_status Game status of the Training battle state (nSCBattleGameStatus*).
 * @param is_custom_menu_open Non-zero while the Z menu is open.
 * @return The toggle_menu value.
 */
extern int sc1PTrainingSbmGetToggleMenu(int game_status, int is_custom_menu_open);

/**
 * Whether a Training menu is up, which sends the Single Button Mode filter to its Training menu path (toggle_menu is 2 or more).
 *
 * @param toggle_menu Value of Training.toggle_menu.
 * @return Non-zero when a menu is up.
 */
extern int sc1PTrainingSbmIsMenuUp(int toggle_menu);

#endif

#endif
