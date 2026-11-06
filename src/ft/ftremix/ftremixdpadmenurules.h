#ifndef _FTREMIXDPADMENURULES_H_
#define _FTREMIXDPADMENURULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the controller transform that the Remix D-pad Control option and the Dpad CSS Cursor Control toggle apply to the
 * global controller (css/DpadControl.asm css_cursor_and_dpad_controls_, 0x80004000): the Stick Swap exchange of the stick and the
 * D-pad (with the DR turbo of the Training screen), the stick cleared by Stickless in the unpaused matches and on the title, the
 * D-pad as a stick on the paused Training menu, and the D-pad cursor of the character select screens. The headless unit tests
 * (tests/remix) run them without game state; ftremixdpadmenu.c reads the game state and calls them. The fighter side of the option
 * is in ftremixdpad.c. */

#define FTREMIX_DPAD_MENU_BUTTON_R 0x0100
#define FTREMIX_DPAD_MENU_BUTTON_L 0x0200
#define FTREMIX_DPAD_MENU_BUTTON_D 0x0400
#define FTREMIX_DPAD_MENU_BUTTON_U 0x0800
#define FTREMIX_DPAD_MENU_BUTTONS 0x0F00

#define FTREMIX_DPAD_MENU_SWAP_DEADZONE_X 40
#define FTREMIX_DPAD_MENU_SWAP_DEADZONE_Y 30
#define FTREMIX_DPAD_MENU_SWAP_STICK 80
#define FTREMIX_DPAD_MENU_CURSOR_STICK 48
#define FTREMIX_DPAD_MENU_CURSOR_DELAY 30
#define FTREMIX_DPAD_MENU_CURSOR_DELAY_STICKLESS 12
#define FTREMIX_DPAD_MENU_TURBO_FRAMES 5

/** What the controller transform does to a port outside the matches. */
typedef enum FTRemixDpadMenuAction
{
    nFTRemixDpadMenuActionNone,
    nFTRemixDpadMenuActionSwap,             /* Stick Swap: the stick presses the D-pad and the D-pad moves the stick */
    nFTRemixDpadMenuActionSwapTraining,     /* Stick Swap on the Training screen: the same, plus the DR turbo of the stick pushed right */
    nFTRemixDpadMenuActionClearStick,       /* Stickless in an unpaused match or on the title: the stick is zeroed */
    nFTRemixDpadMenuActionTrainingPause,    /* Stickless on the paused Training menu: the D-pad moves the stick */
    nFTRemixDpadMenuActionCursor            /* character select: the D-pad moves the cursor stick */

} FTRemixDpadMenuAction;

/** The screen a port is read on: the facts that css_cursor_and_dpad_controls_ tests before it picks what to do. */
typedef struct FTRemixDpadMenuScreen
{
    int is_match_scene;         /* a scene that runs a match (VS, 1P, Bonus or Training) */
    int is_title_scene;         /* the title screen, which the asm groups with the 1P screens (Global.screen.TITLE_AND_1P) */
    int is_paused;              /* the match scene is paused; always 0 outside a match scene */
    int is_training;            /* the Training scene */
    int is_css_scene;           /* a character select screen (VS, 1P, Training, Bonus 1, Bonus 2) */
    int is_cursor_toggle_on;    /* the Dpad CSS Cursor Control toggle is on */

} FTRemixDpadMenuScreen;

/** The controller values of a port that the transform reads and writes (the held, pressed and repeat buttons and the stick). */
typedef struct FTRemixDpadMenuSample
{
    unsigned int hold;
    unsigned int tap;
    unsigned int update;
    int stick_x;
    int stick_y;

} FTRemixDpadMenuSample;

/**
 * Picks what the transform does to a port (css_cursor_and_dpad_controls_ up to _check_screen_id). Stick Swap (1, 3) swaps on every
 * screen but an unpaused match, plus the DR turbo on the Training screen. The Training screen is swapped even while the match runs,
 * because its D-pad shortcuts (reset, freeze, frame advance, model display) read the global controller; every other unpaused match
 * is left to ftremixdpad.c, which applies the swap to the fighter sample. Stickless (2, 4) opens the paused Training menu to the D-pad,
 * and Stickless 2 (not 4) zeroes the stick in an unpaused match or on the title; the cursor works on the character select screens for
 * any of the options 2 and 4, and with option 0 only when the Dpad CSS Cursor Control toggle is on.
 *
 * @param mode D-pad Control panel value of the port.
 * @param screen The screen the port is read on.
 * @return A FTRemixDpadMenuAction.
 */
extern int ftRemixDpadMenuPickAction(int mode, const FTRemixDpadMenuScreen *screen);

/**
 * Stick Swap (_stick_dpad_switcheroo): the stick presses the D-pad once it passes the dead zone (40 on X, 30 on Y), a press only on the
 * frame the direction was not held before, and the held D-pad sets the stick to 80 on that axis. The physical D-pad bits are removed.
 *
 * @param sample Controller values of the port, rewritten.
 * @param prev_hold Buttons held on the previous frame.
 */
extern void ftRemixDpadMenuSwap(FTRemixDpadMenuSample *sample, unsigned int prev_hold);

/**
 * Stick Swap on the Training screen: ftRemixDpadMenuSwap plus the turbo that the Training frame advance needs (the check of DR in
 * the turbo mask). While the stick is pushed right past the dead zone, every 5th frame adds DR to the repeat buttons; the timer is
 * not reset when the stick is released, as in DR_stick_training_turbo_timer.
 *
 * @param sample Controller values of the port, rewritten.
 * @param prev_hold Buttons held on the previous frame.
 * @param turbo_frames Frames since the last DR turbo of the port, updated.
 */
extern void ftRemixDpadMenuSwapTraining(FTRemixDpadMenuSample *sample, unsigned int prev_hold, int *turbo_frames);

/**
 * One step of the DR turbo timer (DR_stick_training_turbo_timer, a countdown from 5 in the asm; counted up here so a zeroed timer
 * starts the same cycle): the 5th call after a reset returns 1 and restarts the cycle.
 *
 * @param turbo_frames Frames since the last DR turbo, updated.
 * @return 1 when this frame carries the DR turbo, 0 otherwise.
 */
extern int ftRemixDpadMenuStepTurbo(int *turbo_frames);

/**
 * Stickless in an unpaused match or on the title (_SNJ_clear_x_y): the stick is zeroed and the buttons stay as they are.
 *
 * @param sample Controller values of the port, the stick rewritten.
 */
extern void ftRemixDpadMenuClearStick(FTRemixDpadMenuSample *sample);

/**
 * Stickless on the paused Training menu (_training_pause_control): the D-pad bits are removed and the held D-pad moves the stick to 80
 * on that axis; the stick itself stays as it is.
 *
 * @param sample Controller values of the port, rewritten.
 */
extern void ftRemixDpadMenuDpadAsStick(FTRemixDpadMenuSample *sample);

/**
 * D-pad cursor (_check_screen_id .. _check_dpad_buttons): the D-pad held on the previous frame moves the stick to 48 on that axis. The
 * first frame of a press only arms the delay (30 frames, 12 with D-pad Control on). While a token is held over a fighter the delay
 * counts down before the stick moves, so the D-pad can pick a variant; in any other case the stick moves at once.
 *
 * @param sample Controller values of the port, the stick rewritten.
 * @param prev_hold Buttons held on the previous frame.
 * @param has_mode TRUE when the D-pad Control panel value of the port is not Off.
 * @param timer Delay of the port, updated.
 * @param is_token_over_fighter TRUE when the token of the port is held over a fighter.
 */
extern void ftRemixDpadMenuCursor(FTRemixDpadMenuSample *sample, unsigned int prev_hold, int has_mode, int *timer,
                                  int is_token_over_fighter);

#endif

#endif
