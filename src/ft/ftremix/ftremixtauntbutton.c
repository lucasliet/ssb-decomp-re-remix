/**
 * ftremixtauntbutton.c - Taunt Button setting of Smash Remix (src/Joypad.asm set_taunt_mask_, prevent_taunt_jump_).
 * The panel value moves the taunt of a human fighter from the L trigger to another button. A C button taunt must not
 * also jump, so the C button jump checks of ftCommonKneeBendCheckButtonTap and ftCommonJumpAerialMultiCheckJumpButtonHold
 * call ftRemixTauntButtonAllowsJump.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixpanel.h>
#include <gm/gmremixtoggle.h>
#include <port_log.h>

#define FTREMIX_TAUNT_BUTTON_MAX 8
#define FTREMIX_TAUNT_BUTTON_C_MASK (R_CBUTTONS | L_CBUTTONS | D_CBUTTONS | U_CBUTTONS)

/* Joypad.asm mask_table: the button of panel values 1..8 (value 0 keeps the vanilla taunt button). */
static const u16 sFTRemixTauntButtonMasks[FTREMIX_TAUNT_BUTTON_MAX] = {
    U_CBUTTONS, D_CBUTTONS, L_CBUTTONS, R_CBUTTONS, U_JPAD, D_JPAD, L_JPAD, R_JPAD,
};

/**
 * Returns the taunt button mask of a fighter (asm set_taunt_mask_). Only human fighters take the panel button, and
 * single button mode keeps the vanilla mask. Called from ftManagerMakeFighter in place of the description mask.
 */
u16 ftRemixTauntButtonMaskFor(FTStruct *fp, u16 vanilla_mask)
{
    s32 panel_value;

    if (fp->pkind != nFTPlayerKindMan)
    {
        return vanilla_mask;
    }
    if (gmRemixToggle(single_button_mode) != 0)
    {
        return vanilla_mask;
    }
    panel_value = gmRemixPanel(TAUNT_BUTTON, fp->player);
    if ((panel_value < 0) || (panel_value > FTREMIX_TAUNT_BUTTON_MAX))
    {
        port_log("ftRemixTauntButton: panel value %d is outside 0..%d\n", panel_value, FTREMIX_TAUNT_BUTTON_MAX);
        return vanilla_mask;
    }
    if (panel_value == 0)
    {
        return vanilla_mask;
    }
    return sFTRemixTauntButtonMasks[panel_value - 1];
}

/**
 * Returns TRUE when the C button presses in pressed may jump (asm prevent_taunt_jump_). A press made only of the taunt
 * button of the fighter does not jump. Returns FALSE when no C button is in pressed. pressed is the tap or hold mask
 * that the calling jump check reads.
 */
sb32 ftRemixTauntButtonAllowsJump(FTStruct *fp, u16 pressed)
{
    u16 c_pressed = pressed & FTREMIX_TAUNT_BUTTON_C_MASK;
    u16 taunt_c_pressed = c_pressed & fp->input.button_mask_l;

    return taunt_c_pressed != c_pressed;
}

#endif
