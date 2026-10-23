/**
 * ftremixdpad.c - Smash Remix DpadControl.asm: the controller transform of the D-pad Control panel option (Stick Swap,
 * Stickless and their J variants) and the R macro it changes (A+L instead of A+Z).
 *
 * ftRemixDpadControlController (ftMainProcUpdateInterrupt, after ftRemixInputDelayController) returns the controller
 * sample the human fighter reads this frame. Not ported here: the jump type and dash attack checks of the J variants
 * (check_jump_type_idle_, check_jump_type_running_, dash_attack_check_), the shield mask override (set_shield_mask_),
 * and the partial stick values of the D-pad (see REMIX-TODO(DpadControl) below).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmdef.h>
#include <gm/gmremixpanel.h>
#include <PR/os.h>
#include <sys/controller.h>

#define FTREMIX_DPAD_CONTROL_STICK_SWAP 1
#define FTREMIX_DPAD_CONTROL_STICKLESS 2
#define FTREMIX_DPAD_CONTROL_STICK_SWAP_J 3
#define FTREMIX_DPAD_CONTROL_STICKLESS_J 4

#define FTREMIX_DPAD_BUTTONS (U_JPAD | D_JPAD | L_JPAD | R_JPAD)
#define FTREMIX_DPAD_SWAP_DEADZONE_X 40
#define FTREMIX_DPAD_SWAP_DEADZONE_Y 30

/* One synthetic sample per port (asm writes the swapped values into the controller struct). */
static SYController sFTRemixDpadControlSamples[GMCOMMON_PLAYERS_MAX];

/**
 * Returns TRUE for the modes that exchange the stick and the D-pad (Stick Swap and Stick Swap J).
 */
static sb32 ftRemixDpadControlSwapsStick(s32 mode)
{
    return (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP) || (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP_J);
}

/**
 * Returns the D-pad bits a stick position presses. An axis presses a direction once its magnitude reaches the deadzone
 * (40 on X, 30 on Y, as in the asm).
 */
static u16 ftRemixDpadFromStick(Vec2b stick)
{
    u16 dpad = 0;

    if (stick.x >= FTREMIX_DPAD_SWAP_DEADZONE_X)
    {
        dpad |= R_JPAD;
    }
    if (stick.x <= -FTREMIX_DPAD_SWAP_DEADZONE_X)
    {
        dpad |= L_JPAD;
    }
    if (stick.y >= FTREMIX_DPAD_SWAP_DEADZONE_Y)
    {
        dpad |= U_JPAD;
    }
    if (stick.y <= -FTREMIX_DPAD_SWAP_DEADZONE_Y)
    {
        dpad |= D_JPAD;
    }
    return dpad;
}

/**
 * Returns one stick axis from the D-pad bits, at full range (+-I_CONTROLLER_RANGE_MAX). Both directions of an axis
 * cancel out, as the asm adds the two values.
 */
static s32 ftRemixDpadStickAxis(u16 dpad, u16 positive_button, u16 negative_button)
{
    s32 axis = 0;

    if (dpad & positive_button)
    {
        axis += I_CONTROLLER_RANGE_MAX;
    }
    if (dpad & negative_button)
    {
        axis -= I_CONTROLLER_RANGE_MAX;
    }
    return axis;
}

/**
 * Returns the held buttons with R replaced by A+L (set_R_macro). R is cleared so ftMainProcUpdateInterrupt does not
 * expand it again to A+Z.
 */
static u16 ftRemixDpadControlRMacro(u16 hold)
{
    if (!(hold & R_TRIG))
    {
        return hold;
    }
    return (hold & ~R_TRIG) | A_BUTTON | L_TRIG;
}

/**
 * Returns the controller sample a human fighter reads this frame with DpadControl applied. Returns controller unchanged
 * when the port has the option off or the port is invalid. Called from ftMainProcUpdateInterrupt after the InputDelay
 * sample, so the swap works on the delayed input.
 */
SYController *ftRemixDpadControlController(FTStruct *fp, SYController *controller)
{
    s32 player = fp->player;
    s32 mode;
    u16 dpad;
    u16 hold;
    SYController *sample;

    if (player >= GMCOMMON_PLAYERS_MAX)
    {
        return controller;
    }
    mode = gmRemixPanel(DPAD_CONTROL, player);
    if (mode == 0)
    {
        return controller;
    }

    sample = &sFTRemixDpadControlSamples[player];
    *sample = *controller;
    dpad = controller->button_hold & FTREMIX_DPAD_BUTTONS;
    hold = controller->button_hold;

    if (ftRemixDpadControlSwapsStick(mode))
    {
        hold = (hold & ~FTREMIX_DPAD_BUTTONS) | ftRemixDpadFromStick(controller->stick_range);
    }
    sample->button_hold = ftRemixDpadControlRMacro(hold);
    sample->stick_range.x = (s8) ftRemixDpadStickAxis(dpad, R_JPAD, L_JPAD);
    sample->stick_range.y = (s8) ftRemixDpadStickAxis(dpad, U_JPAD, D_JPAD);
    return sample;
}

/* REMIX-TODO(DpadControl): the asm uses partial stick values (+-40) for a single D-pad tap and full values (+-80) for a
 * double tap or a held D-pad, with a per-port timer (dpad_move_timer, DpadFunctions.asm _dpad_move_*). This port always
 * uses full values. Needs the timer state and the fighter action ids (Walk2, Walk3, Turn, Action.FOX.*, ...). */

#endif
