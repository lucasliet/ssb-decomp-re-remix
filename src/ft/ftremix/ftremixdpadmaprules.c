#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixdpadmaprules.h"

#define FTREMIX_DPAD_MAP_CONTROL_STICKLESS 2
#define FTREMIX_DPAD_MAP_CONTROL_STICKLESS_J 4

#define FTREMIX_DPAD_MAP_STICK_MAX 0x50
#define FTREMIX_DPAD_MAP_STICK_MIN 0xB0
#define FTREMIX_DPAD_MAP_STICK_HALF_MAX 0x28
#define FTREMIX_DPAD_MAP_STICK_HALF_MIN 0xD8
#define FTREMIX_DPAD_MAP_STICK_SHIELD_UP 0x35

/** Signed value of a stick byte of the asm stick words (0xB0 is -80). */
static int ftRemixDpadMapSignedByte(unsigned int byte)
{
    return (byte < 0x80) ? (int)byte : (int)byte - 0x100;
}

/** Writes the stick of an asm stick word (X byte high, Y byte low) and the rewrite the _overwrite label does. */
static void ftRemixDpadMapOverwrite(const FTRemixDpadMapInput *input, unsigned int stick_word, FTRemixDpadMapResult *result)
{
    result->is_stick_set = 1;
    result->stick_x = ftRemixDpadMapSignedByte((stick_word >> 8) & 0xFF);
    result->stick_y = ftRemixDpadMapSignedByte(stick_word & 0xFF);
    result->hold = input->hold | FTREMIX_DPAD_MAP_BUTTON_A;
}

/** Full stick word of the first D-pad direction held, in the asm order up, down, left, right (_add_stick_input). */
static unsigned int ftRemixDpadMapFullStickWord(unsigned int hold)
{
    if (hold & FTREMIX_DPAD_MAP_DPAD_UP)
    {
        return FTREMIX_DPAD_MAP_STICK_MAX;
    }
    if (hold & FTREMIX_DPAD_MAP_DPAD_DOWN)
    {
        return FTREMIX_DPAD_MAP_STICK_MIN;
    }
    if (hold & FTREMIX_DPAD_MAP_DPAD_LEFT)
    {
        return FTREMIX_DPAD_MAP_STICK_MIN << 8;
    }
    return FTREMIX_DPAD_MAP_STICK_MAX << 8;
}

/** TRUE for the actions where D-pad up jumps: dash, run and shield (_check_usmash). */
static int ftRemixDpadMapUpJumps(int status_id)
{
    return (status_id == FTREMIX_DPAD_MAP_STATUS_DASH) || (status_id == FTREMIX_DPAD_MAP_STATUS_RUN) ||
           (status_id == FTREMIX_DPAD_MAP_STATUS_SHIELD);
}

/** TRUE for the jump squats, where D-pad up presses the button at once (_check_usmash). */
static int ftRemixDpadMapUpPressesInJumpSquat(int status_id)
{
    return (status_id == FTREMIX_DPAD_MAP_STATUS_JUMP_SQUAT) || (status_id == FTREMIX_DPAD_MAP_STATUS_SHIELD_JUMP_SQUAT);
}

/**
 * Smash and Special (dpad_macro_check_ _check_usmash .. _add_stick_input): C-Up jump from D-pad up while dashing,
 * running or shielding; otherwise the button (A or B) plus the full stick, once per D-pad press (a jump squat presses
 * again every frame).
 */
static void ftRemixDpadMapPressButton(const FTRemixDpadMapInput *input, unsigned int button, FTRemixDpadMapResult *result)
{
    if (input->hold & FTREMIX_DPAD_MAP_DPAD_UP)
    {
        if (ftRemixDpadMapUpJumps(input->status_id))
        {
            result->is_applied = 1;
            result->tap_buttons = input->hold | FTREMIX_DPAD_MAP_BUTTON_C_UP;
            return;
        }
        if (!ftRemixDpadMapUpPressesInJumpSquat(input->status_id) && (input->hold_prev & FTREMIX_DPAD_MAP_DPAD_ALL))
        {
            return;
        }
    }
    else if (input->hold_prev & FTREMIX_DPAD_MAP_DPAD_ALL)
    {
        return;
    }
    result->is_applied = 1;
    result->tap_buttons = input->hold | button;
    ftRemixDpadMapOverwrite(input, ftRemixDpadMapFullStickWord(input->hold), result);
}

/** TRUE for the actions where Tilt moves the stick without pressing A (_check_tilt_action). */
static int ftRemixDpadMapIsShieldAction(int status_id)
{
    return (status_id == FTREMIX_DPAD_MAP_STATUS_SHIELD) || (status_id == FTREMIX_DPAD_MAP_STATUS_SHIELD_ON) ||
           (status_id == FTREMIX_DPAD_MAP_STATUS_ROLL_F) || (status_id == FTREMIX_DPAD_MAP_STATUS_ROLL_B);
}

/** Half stick word of the D-pad directions held; the asm ORs one value per direction (_check_tilt_direction). */
static unsigned int ftRemixDpadMapTiltStickWord(unsigned int hold)
{
    unsigned int stick_word = 0;

    if (hold & FTREMIX_DPAD_MAP_DPAD_UP)
    {
        stick_word |= FTREMIX_DPAD_MAP_STICK_HALF_MAX;
    }
    if (hold & FTREMIX_DPAD_MAP_DPAD_DOWN)
    {
        stick_word |= FTREMIX_DPAD_MAP_STICK_HALF_MIN;
    }
    if (hold & FTREMIX_DPAD_MAP_DPAD_LEFT)
    {
        stick_word |= FTREMIX_DPAD_MAP_STICK_HALF_MIN << 8;
    }
    if (hold & FTREMIX_DPAD_MAP_DPAD_RIGHT)
    {
        stick_word |= FTREMIX_DPAD_MAP_STICK_HALF_MAX << 8;
    }
    return stick_word;
}

/**
 * Tilt while shielding or rolling (_shielded_tilt): no A press in the tap mask except for down, which is an A press with
 * a half stick; up uses a 0x35 stick, left and right a full stick. A joins the held buttons in every case.
 */
static void ftRemixDpadMapPressShieldTilt(const FTRemixDpadMapInput *input, FTRemixDpadMapResult *result)
{
    unsigned int stick_word;

    result->is_applied = 1;
    result->tap_buttons = input->hold;
    if (input->hold & FTREMIX_DPAD_MAP_DPAD_UP)
    {
        stick_word = FTREMIX_DPAD_MAP_STICK_SHIELD_UP;
    }
    else if (input->hold & FTREMIX_DPAD_MAP_DPAD_DOWN)
    {
        stick_word = FTREMIX_DPAD_MAP_STICK_HALF_MIN;
        result->tap_buttons = input->hold | FTREMIX_DPAD_MAP_BUTTON_A;
    }
    else if (input->hold & FTREMIX_DPAD_MAP_DPAD_LEFT)
    {
        stick_word = FTREMIX_DPAD_MAP_STICK_MIN << 8;
    }
    else
    {
        stick_word = FTREMIX_DPAD_MAP_STICK_MAX << 8;
    }
    ftRemixDpadMapOverwrite(input, stick_word, result);
}

/** Tilt (_dpad_macro_tilt): once per D-pad press, A with a half stick, or the shield tilt. */
static void ftRemixDpadMapPressTilt(const FTRemixDpadMapInput *input, FTRemixDpadMapResult *result)
{
    if (input->hold_prev & FTREMIX_DPAD_MAP_DPAD_ALL)
    {
        return;
    }
    if (ftRemixDpadMapIsShieldAction(input->status_id))
    {
        ftRemixDpadMapPressShieldTilt(input, result);
        return;
    }
    result->is_applied = 1;
    result->tap_buttons = input->hold | FTREMIX_DPAD_MAP_BUTTON_A;
    ftRemixDpadMapOverwrite(input, ftRemixDpadMapTiltStickWord(input->hold), result);
}

int ftRemixDpadMapIsEnabled(int map_mode, int control_mode, int single_button_mode, int is_training, int is_training_dpad_off)
{
    if (map_mode == FTREMIX_DPAD_MAP_OFF)
    {
        return 0;
    }
    if ((control_mode == FTREMIX_DPAD_MAP_CONTROL_STICKLESS) || (control_mode == FTREMIX_DPAD_MAP_CONTROL_STICKLESS_J))
    {
        return 0;
    }
    if (single_button_mode != 0)
    {
        return 0;
    }
    return (is_training == 0) || (is_training_dpad_off != 0);
}

void ftRemixDpadMapPress(const FTRemixDpadMapInput *input, FTRemixDpadMapResult *result)
{
    result->is_applied = 0;
    result->tap_buttons = 0;
    result->is_stick_set = 0;
    result->stick_x = 0;
    result->stick_y = 0;
    result->hold = input->hold;

    if (!(input->hold & FTREMIX_DPAD_MAP_DPAD_ALL))
    {
        return;
    }
    switch (input->map_mode)
    {
    case FTREMIX_DPAD_MAP_SMASH:
        ftRemixDpadMapPressButton(input, FTREMIX_DPAD_MAP_BUTTON_A, result);
        break;

    case FTREMIX_DPAD_MAP_SPECIAL:
        ftRemixDpadMapPressButton(input, FTREMIX_DPAD_MAP_BUTTON_B, result);
        break;

    case FTREMIX_DPAD_MAP_TILT:
        ftRemixDpadMapPressTilt(input, result);
        break;

    default:
        break;
    }
}

unsigned int ftRemixDpadMapHeld(int map_mode, int is_charged_smashes_on, unsigned int hold)
{
    if (!(hold & FTREMIX_DPAD_MAP_DPAD_ALL))
    {
        return hold;
    }
    if (map_mode == FTREMIX_DPAD_MAP_SPECIAL)
    {
        return hold | FTREMIX_DPAD_MAP_BUTTON_B;
    }
    if ((map_mode == FTREMIX_DPAD_MAP_SMASH) && (is_charged_smashes_on != 0))
    {
        return hold | FTREMIX_DPAD_MAP_BUTTON_A;
    }
    return hold;
}

#endif
