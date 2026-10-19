/**
 * ifremixhud.c - Disable HUD setting (Toggles.asm entry_disable_hud) for the battle interface.
 * Camera.asm disable_hud_ and Training.asm disable_hud_ read the same toggle value; ALL is the bit 0x0002.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixhud.h"
#include "gm/gmremixtoggle.h"

#define IF_REMIX_HUD_DISABLE_ALL 0x0002

/** TRUE when the Disable HUD toggle is All, so the whole battle interface (damage, timer, stock icons) stays hidden. */
sb32 ifRemixHudDisabledAll(void)
{
    return (gmRemixToggle(disable_hud) & IF_REMIX_HUD_DISABLE_ALL) != 0;
}

#endif
