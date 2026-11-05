/**
 * ftremixdpadmap.c - Smash Remix css/DpadFunctions.asm D-pad Map option (Off, Smash, Tilt, Special): the macros of
 * dpad_macro_check_ (a D-pad press becomes a button press and a stick position) and dpad_macro_check_held_ (the button
 * stays held while the D-pad is held).
 *
 * ftMainProcUpdateInterrupt calls ftRemixDpadMapApplyPress after the tap mask of the frame (the asm hook at 0x800E1330)
 * and ftRemixDpadMapHeldButtons before the held buttons are stored (the hook at 0x800E1378). The rules themselves are in
 * ftremixdpadmaprules.c.
 *
 * Known difference: the D-pad bits are those of the sample ftRemixDpadControlController returns, so Stick Swap macros
 * read the D-pad generated from the stick, as the asm does.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixdpadmaprules.h"
#include <ft/fighter.h>
#include <gm/gmdef.h>
#include <gm/gmremixpanel.h>
#include <gm/gmremixtoggle.h>
#include <sc/scmanager.h>
#include <sc/scdef.h>
#include <sc/sctypes.h>
#include "remix/remix_training.h"

_Static_assert(nFTCommonStatusDash == FTREMIX_DPAD_MAP_STATUS_DASH, "Action.Dash");
_Static_assert(nFTCommonStatusRun == FTREMIX_DPAD_MAP_STATUS_RUN, "Action.Run");
_Static_assert(nFTCommonStatusKneeBend == FTREMIX_DPAD_MAP_STATUS_JUMP_SQUAT, "Action.JumpSquat");
_Static_assert(nFTCommonStatusGuardKneeBend == FTREMIX_DPAD_MAP_STATUS_SHIELD_JUMP_SQUAT, "Action.ShieldJumpSquat");
_Static_assert(nFTCommonStatusGuardOn == FTREMIX_DPAD_MAP_STATUS_SHIELD_ON, "Action.ShieldOn");
_Static_assert(nFTCommonStatusGuard == FTREMIX_DPAD_MAP_STATUS_SHIELD, "Action.Shield");
_Static_assert(nFTCommonStatusEscapeF == FTREMIX_DPAD_MAP_STATUS_ROLL_F, "Action.RollF");
_Static_assert(nFTCommonStatusEscapeB == FTREMIX_DPAD_MAP_STATUS_ROLL_B, "Action.RollB");
_Static_assert(U_JPAD == FTREMIX_DPAD_MAP_DPAD_UP, "Joypad.DU");
_Static_assert(D_JPAD == FTREMIX_DPAD_MAP_DPAD_DOWN, "Joypad.DD");
_Static_assert(L_JPAD == FTREMIX_DPAD_MAP_DPAD_LEFT, "Joypad.DL");
_Static_assert(R_JPAD == FTREMIX_DPAD_MAP_DPAD_RIGHT, "Joypad.DR");
_Static_assert(A_BUTTON == FTREMIX_DPAD_MAP_BUTTON_A, "Joypad.A");
_Static_assert(B_BUTTON == FTREMIX_DPAD_MAP_BUTTON_B, "Joypad.B");
_Static_assert(U_CBUTTONS == FTREMIX_DPAD_MAP_BUTTON_C_UP, "Joypad.CU");

/**
 * TRUE when the macros run for this fighter. single_button_mode is the Toggles value the press hook checks; the held hook
 * does not check it, so it passes 0.
 */
static sb32 ftRemixDpadMapIsEnabledFor(FTStruct *fp, s32 single_button_mode)
{
    sb32 is_training;
    sb32 is_training_dpad_off;

    if ((fp->player >= GMCOMMON_PLAYERS_MAX) || (gmRemixPanel(DPAD_FUNCTIONS, fp->player) == FTREMIX_DPAD_MAP_OFF))
    {
        return FALSE;
    }
    is_training = (gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode) ? TRUE : FALSE;
    is_training_dpad_off = (remix_training_get(REMIX_TRAINING_DPAD) == REMIX_TRAINING_DPAD_OFF) ? TRUE : FALSE;
    return ftRemixDpadMapIsEnabled(gmRemixPanel(DPAD_FUNCTIONS, fp->player), gmRemixPanel(DPAD_CONTROL, fp->player),
                                   single_button_mode, is_training, is_training_dpad_off) != 0;
}

/**
 * Applies the press macro of the port to the input of this frame: replaces the tap mask, may overwrite the stick and add A
 * to the held buttons. Called for human fighters after the tap mask is computed from the held buttons.
 *
 * @param fp Human fighter; its input still holds the previous frame (button_hold) and its action id.
 * @param button_hold Held buttons of this frame, updated.
 * @param button_tap_mask Newly pressed buttons of this frame, replaced when the macro fires.
 */
void ftRemixDpadMapApplyPress(FTStruct *fp, u16 *button_hold, u16 *button_tap_mask)
{
    FTRemixDpadMapInput input;
    FTRemixDpadMapResult result;

    if (!ftRemixDpadMapIsEnabledFor(fp, gmRemixToggle(single_button_mode)))
    {
        return;
    }
    input.map_mode = gmRemixPanel(DPAD_FUNCTIONS, fp->player);
    input.hold = *button_hold;
    input.hold_prev = fp->input.pl.button_hold;
    input.status_id = fp->status_id;
    ftRemixDpadMapPress(&input, &result);
    if (result.is_applied == 0)
    {
        return;
    }
    *button_tap_mask = (u16)result.tap_buttons;
    *button_hold = (u16)result.hold;
    if (result.is_stick_set != 0)
    {
        fp->input.pl.stick_range.x = (s8)result.stick_x;
        fp->input.pl.stick_range.y = (s8)result.stick_y;
    }
}

/**
 * Returns the held buttons to store for the frame: Special keeps B held and Smash keeps A held (with Charged Smash
 * Attacks on) while the D-pad is held, so a charged smash or a held special keeps going.
 */
u16 ftRemixDpadMapHeldButtons(FTStruct *fp, u16 button_hold)
{
    if (!ftRemixDpadMapIsEnabledFor(fp, 0))
    {
        return button_hold;
    }
    return (u16)ftRemixDpadMapHeld(gmRemixPanel(DPAD_FUNCTIONS, fp->player), (gmRemixToggle(charged_smashes) != 0) ? 1 : 0,
                                   button_hold);
}

#endif
