#ifndef _FTREMIXDPADMAPRULES_H_
#define _FTREMIXDPADMAPRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix CSS panel D-pad Map option (css/DpadFunctions.asm dpad_macro_check_ and
 * dpad_macro_check_held_), so the headless unit tests (tests/remix) can run them without game state. Buttons are the
 * N64 button bits, status ids the Remix action ids that the macros compare (Action.asm). */

#define FTREMIX_DPAD_MAP_OFF 0
#define FTREMIX_DPAD_MAP_SMASH 1
#define FTREMIX_DPAD_MAP_TILT 2
#define FTREMIX_DPAD_MAP_SPECIAL 3

#define FTREMIX_DPAD_MAP_BUTTON_A 0x8000
#define FTREMIX_DPAD_MAP_BUTTON_B 0x4000
#define FTREMIX_DPAD_MAP_BUTTON_C_UP 0x0008
#define FTREMIX_DPAD_MAP_DPAD_UP 0x0800
#define FTREMIX_DPAD_MAP_DPAD_DOWN 0x0400
#define FTREMIX_DPAD_MAP_DPAD_LEFT 0x0200
#define FTREMIX_DPAD_MAP_DPAD_RIGHT 0x0100
#define FTREMIX_DPAD_MAP_DPAD_ALL 0x0F00

#define FTREMIX_DPAD_MAP_STATUS_DASH 0x0F
#define FTREMIX_DPAD_MAP_STATUS_RUN 0x10
#define FTREMIX_DPAD_MAP_STATUS_JUMP_SQUAT 0x14
#define FTREMIX_DPAD_MAP_STATUS_SHIELD_JUMP_SQUAT 0x15
#define FTREMIX_DPAD_MAP_STATUS_SHIELD_ON 0x98
#define FTREMIX_DPAD_MAP_STATUS_SHIELD 0x99
#define FTREMIX_DPAD_MAP_STATUS_ROLL_F 0x9C
#define FTREMIX_DPAD_MAP_STATUS_ROLL_B 0x9D

/** Frame inputs of the D-pad Map press macros: the held buttons of the frame (asm a3), of the previous frame, and the action id. */
typedef struct FTRemixDpadMapInput
{
    int map_mode;
    unsigned int hold;
    unsigned int hold_prev;
    int status_id;

} FTRemixDpadMapInput;

/** What a press macro rewrites. Every field is untouched (is_applied 0) when the frame runs as the vanilla controller. */
typedef struct FTRemixDpadMapResult
{
    int is_applied;                 /* asm v1 was replaced: tap_buttons is the pressed mask of the frame */
    unsigned int tap_buttons;
    int is_stick_set;               /* asm _overwrite: the stick is stick_x, stick_y and A joins the held buttons */
    int stick_x;
    int stick_y;
    unsigned int hold;              /* held buttons after the macro */

} FTRemixDpadMapResult;

/**
 * TRUE when the macros may run for a port (dpad_macro_check_ _check_dpad_controls .. _check_macro_type): the map is on,
 * D-pad Control is not Stickless / Stickless J, Single Button Mode is off, and in Training the D-pad shortcuts are off.
 *
 * @param map_mode D-pad Map panel value (0 Off, 1 Smash, 2 Tilt, 3 Special).
 * @param control_mode D-pad Control panel value (0 Off, 1 Stick Swap, 2 Stickless, 3 Stick Swap J, 4 Stickless J).
 * @param single_button_mode Toggles single_button_mode value.
 * @param is_training TRUE in the Training scene.
 * @param is_training_dpad_off TRUE when the Training "D-pad Controls" entry is OFF.
 */
extern int ftRemixDpadMapIsEnabled(int map_mode, int control_mode, int single_button_mode, int is_training, int is_training_dpad_off);

/**
 * Press macro of one frame (dpad_macro_check_ _check_macro_type .. _overwrite). Smash and Special press A or B with the
 * stick pointing where the D-pad points, once per D-pad press; Tilt presses A with a half stick. Results are in *result.
 */
extern void ftRemixDpadMapPress(const FTRemixDpadMapInput *input, FTRemixDpadMapResult *result);

/**
 * Held buttons of the frame after the held hook (dpad_macro_check_held_ _check_macro_type .. _end): Special keeps B held
 * while the D-pad is held, Smash keeps A held only when Charged Smash Attacks is on, Tilt adds nothing.
 *
 * @param map_mode D-pad Map panel value.
 * @param is_charged_smashes_on TRUE when the Toggles charged_smashes entry is not 0.
 * @param hold Held buttons of the frame.
 */
extern unsigned int ftRemixDpadMapHeld(int map_mode, int is_charged_smashes_on, unsigned int hold);

#endif

#endif
