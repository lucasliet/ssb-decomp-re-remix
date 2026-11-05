#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixdpadcontrolrules.h"

int ftRemixDpadControlIsStandardMode(int mode)
{
    return (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP) || (mode == FTREMIX_DPAD_CONTROL_STICKLESS);
}

int ftRemixDpadControlRunUpAction(int mode, unsigned int button_tap, unsigned int button_hold)
{
    if (!ftRemixDpadControlIsStandardMode(mode))
    {
        return nFTRemixDpadControlRunUpVanilla;
    }
    if (!(button_tap & FTREMIX_DPAD_CONTROL_BUTTON_B) && (button_hold & FTREMIX_DPAD_CONTROL_BUTTON_B))
    {
        return nFTRemixDpadControlRunUpSpecial;
    }
    if (!(button_tap & FTREMIX_DPAD_CONTROL_BUTTON_A) && (button_hold & FTREMIX_DPAD_CONTROL_BUTTON_A))
    {
        return nFTRemixDpadControlRunUpSmash;
    }
    return nFTRemixDpadControlRunUpNone;
}

int ftRemixDpadControlSkipsDashAttack(int mode, int stick_y)
{
    return ftRemixDpadControlIsStandardMode(mode) && (stick_y > 0);
}

unsigned int ftRemixDpadControlShieldMask(int mode, int is_taunt_button_custom)
{
    if (mode == FTREMIX_DPAD_CONTROL_OFF)
    {
        return FTREMIX_DPAD_CONTROL_BUTTON_Z;
    }
    return is_taunt_button_custom ? (FTREMIX_DPAD_CONTROL_BUTTON_L | FTREMIX_DPAD_CONTROL_BUTTON_Z) : FTREMIX_DPAD_CONTROL_BUTTON_L;
}

unsigned int ftRemixDpadControlTauntMask(int mode, int is_taunt_button_custom, unsigned int taunt_mask)
{
    if ((mode == FTREMIX_DPAD_CONTROL_OFF) || is_taunt_button_custom)
    {
        return taunt_mask;
    }
    return FTREMIX_DPAD_CONTROL_BUTTON_Z;
}

#endif
