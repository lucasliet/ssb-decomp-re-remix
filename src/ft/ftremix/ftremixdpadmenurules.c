#if defined(PORT) && defined(REGION_REMIX)

#include <stddef.h>
#include "ft/ftremix/ftremixdpadcontrolrules.h"
#include "ft/ftremix/ftremixdpadmenurules.h"

#define FTREMIX_DPAD_MENU_BUTTONS_LEFT_RIGHT (FTREMIX_DPAD_MENU_BUTTON_L | FTREMIX_DPAD_MENU_BUTTON_R)
#define FTREMIX_DPAD_MENU_BUTTONS_UP_DOWN (FTREMIX_DPAD_MENU_BUTTON_U | FTREMIX_DPAD_MENU_BUTTON_D)

static int ftRemixDpadMenuIsSwapMode(int mode)
{
    return (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP) || (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP_J);
}

static int ftRemixDpadMenuAbs(int value)
{
    return (value < 0) ? -value : value;
}

/** The cursor of the character select screens answers to the option, or to the toggle when the option is Off (_cursor_control). */
static int ftRemixDpadMenuPickCursorAction(int mode, const FTRemixDpadMenuScreen *screen)
{
    if (screen->is_css_scene == 0)
    {
        return nFTRemixDpadMenuActionNone;
    }
    return ((mode != FTREMIX_DPAD_CONTROL_OFF) || (screen->is_cursor_toggle_on != 0)) ? nFTRemixDpadMenuActionCursor
                                                                                       : nFTRemixDpadMenuActionNone;
}

/** Stick Swap swaps everywhere but an unpaused match; the Training screen is swapped in the match too (see the header). */
static int ftRemixDpadMenuPickSwapAction(const FTRemixDpadMenuScreen *screen)
{
    if (screen->is_training != 0)
    {
        return nFTRemixDpadMenuActionSwapTraining;
    }
    return ((screen->is_match_scene != 0) && (screen->is_paused == 0)) ? nFTRemixDpadMenuActionNone : nFTRemixDpadMenuActionSwap;
}

/** Stickless: the paused Training menu, the stick of the unpaused matches and the title, then the cursor (_SNJ_check_pause). */
static int ftRemixDpadMenuPickSticklessAction(int mode, const FTRemixDpadMenuScreen *screen)
{
    if ((screen->is_match_scene != 0) && (screen->is_paused != 0))
    {
        return (screen->is_training != 0) ? nFTRemixDpadMenuActionTrainingPause : nFTRemixDpadMenuActionNone;
    }
    if ((screen->is_match_scene != 0) || (screen->is_title_scene != 0))
    {
        return (mode == FTREMIX_DPAD_CONTROL_STICKLESS) ? nFTRemixDpadMenuActionClearStick : nFTRemixDpadMenuActionNone;
    }
    return ftRemixDpadMenuPickCursorAction(mode, screen);
}

int ftRemixDpadMenuPickAction(int mode, const FTRemixDpadMenuScreen *screen)
{
    if (ftRemixDpadMenuIsSwapMode(mode) != 0)
    {
        return ftRemixDpadMenuPickSwapAction(screen);
    }
    if (mode == FTREMIX_DPAD_CONTROL_OFF)
    {
        return ftRemixDpadMenuPickCursorAction(mode, screen);
    }
    return ftRemixDpadMenuPickSticklessAction(mode, screen);
}

int ftRemixDpadMenuStepTurbo(int *turbo_frames)
{
    (*turbo_frames)++;

    if (*turbo_frames < FTREMIX_DPAD_MENU_TURBO_FRAMES)
    {
        return 0;
    }
    *turbo_frames = 0;
    return 1;
}

/** Presses the D-pad direction of the stick on X (check_stick_x_ and check_stick_x_hold_); the stick X is 0 afterwards. A timer adds the DR turbo. */
static void ftRemixDpadMenuSwapStickX(FTRemixDpadMenuSample *sample, unsigned int prev_hold, int *turbo_frames)
{
    unsigned int direction;

    if (sample->stick_x == 0)
    {
        return;
    }
    if (ftRemixDpadMenuAbs(sample->stick_x) < FTREMIX_DPAD_MENU_SWAP_DEADZONE_X)
    {
        sample->stick_x = 0;
        return;
    }
    direction = (sample->stick_x < 0) ? FTREMIX_DPAD_MENU_BUTTON_L : FTREMIX_DPAD_MENU_BUTTON_R;
    sample->hold |= direction;

    if ((direction == FTREMIX_DPAD_MENU_BUTTON_R) && (turbo_frames != NULL) && (ftRemixDpadMenuStepTurbo(turbo_frames) != 0))
    {
        sample->update |= FTREMIX_DPAD_MENU_BUTTON_R;
    }
    if ((prev_hold & FTREMIX_DPAD_MENU_BUTTONS_LEFT_RIGHT) == 0)
    {
        sample->tap |= direction;
    }
    sample->stick_x = 0;
}

/** Presses the D-pad direction of the stick on Y (check_stick_y_ and check_stick_y_hold_); the stick Y is 0 afterwards. */
static void ftRemixDpadMenuSwapStickY(FTRemixDpadMenuSample *sample, unsigned int prev_hold)
{
    unsigned int direction;

    if (sample->stick_y == 0)
    {
        return;
    }
    if (ftRemixDpadMenuAbs(sample->stick_y) < FTREMIX_DPAD_MENU_SWAP_DEADZONE_Y)
    {
        sample->stick_y = 0;
        return;
    }
    direction = (sample->stick_y < 0) ? FTREMIX_DPAD_MENU_BUTTON_D : FTREMIX_DPAD_MENU_BUTTON_U;
    sample->hold |= direction;

    if ((prev_hold & FTREMIX_DPAD_MENU_BUTTONS_UP_DOWN) == 0)
    {
        sample->tap |= direction;
    }
    sample->stick_y = 0;
}

/** Moves the stick to 80 on the axes of the D-pad directions held (_check_dpad); right and up are tested last, so they win. */
static void ftRemixDpadMenuStickFromDpad(FTRemixDpadMenuSample *sample, unsigned int dpad_hold)
{
    if (dpad_hold & FTREMIX_DPAD_MENU_BUTTON_L)
    {
        sample->stick_x = -FTREMIX_DPAD_MENU_SWAP_STICK;
    }
    if (dpad_hold & FTREMIX_DPAD_MENU_BUTTON_R)
    {
        sample->stick_x = FTREMIX_DPAD_MENU_SWAP_STICK;
    }
    if (dpad_hold & FTREMIX_DPAD_MENU_BUTTON_D)
    {
        sample->stick_y = -FTREMIX_DPAD_MENU_SWAP_STICK;
    }
    if (dpad_hold & FTREMIX_DPAD_MENU_BUTTON_U)
    {
        sample->stick_y = FTREMIX_DPAD_MENU_SWAP_STICK;
    }
}

static void ftRemixDpadMenuClearDpad(FTRemixDpadMenuSample *sample)
{
    sample->hold &= ~FTREMIX_DPAD_MENU_BUTTONS;
    sample->tap &= ~FTREMIX_DPAD_MENU_BUTTONS;
    sample->update &= ~FTREMIX_DPAD_MENU_BUTTONS;
}

/** Stick Swap with an optional DR turbo timer (NULL outside the Training screen). */
static void ftRemixDpadMenuSwapWithTurbo(FTRemixDpadMenuSample *sample, unsigned int prev_hold, int *turbo_frames)
{
    unsigned int dpad_hold = sample->hold & FTREMIX_DPAD_MENU_BUTTONS;

    ftRemixDpadMenuClearDpad(sample);
    ftRemixDpadMenuSwapStickX(sample, prev_hold, turbo_frames);
    ftRemixDpadMenuSwapStickY(sample, prev_hold);
    ftRemixDpadMenuStickFromDpad(sample, dpad_hold);
}

void ftRemixDpadMenuSwap(FTRemixDpadMenuSample *sample, unsigned int prev_hold)
{
    ftRemixDpadMenuSwapWithTurbo(sample, prev_hold, NULL);
}

void ftRemixDpadMenuSwapTraining(FTRemixDpadMenuSample *sample, unsigned int prev_hold, int *turbo_frames)
{
    ftRemixDpadMenuSwapWithTurbo(sample, prev_hold, turbo_frames);
}

void ftRemixDpadMenuClearStick(FTRemixDpadMenuSample *sample)
{
    sample->stick_x = 0;
    sample->stick_y = 0;
}

void ftRemixDpadMenuDpadAsStick(FTRemixDpadMenuSample *sample)
{
    unsigned int dpad_hold = sample->hold & FTREMIX_DPAD_MENU_BUTTONS;

    ftRemixDpadMenuClearDpad(sample);
    ftRemixDpadMenuStickFromDpad(sample, dpad_hold);
}

/** Moves the stick to 48 on the axes of the D-pad directions held on the previous frame (_check_dpad_buttons). */
static void ftRemixDpadMenuCursorStickFromDpad(FTRemixDpadMenuSample *sample, unsigned int prev_hold)
{
    if (prev_hold & FTREMIX_DPAD_MENU_BUTTON_L)
    {
        sample->stick_x = -FTREMIX_DPAD_MENU_CURSOR_STICK;
    }
    if (prev_hold & FTREMIX_DPAD_MENU_BUTTON_R)
    {
        sample->stick_x = FTREMIX_DPAD_MENU_CURSOR_STICK;
    }
    if (prev_hold & FTREMIX_DPAD_MENU_BUTTON_D)
    {
        sample->stick_y = -FTREMIX_DPAD_MENU_CURSOR_STICK;
    }
    if (prev_hold & FTREMIX_DPAD_MENU_BUTTON_U)
    {
        sample->stick_y = FTREMIX_DPAD_MENU_CURSOR_STICK;
    }
}

void ftRemixDpadMenuCursor(FTRemixDpadMenuSample *sample, unsigned int prev_hold, int has_mode, int *timer, int is_token_over_fighter)
{
    if ((prev_hold & FTREMIX_DPAD_MENU_BUTTONS) == 0)
    {
        *timer = (has_mode != 0) ? FTREMIX_DPAD_MENU_CURSOR_DELAY_STICKLESS : FTREMIX_DPAD_MENU_CURSOR_DELAY;
        return;
    }
    if ((*timer != 0) && (is_token_over_fighter == 0))
    {
        *timer = 0;
    }
    else if (*timer != 0)
    {
        (*timer)--;

        if (*timer > 0)
        {
            return;
        }
    }
    ftRemixDpadMenuCursorStickFromDpad(sample, prev_hold);
}

#endif
