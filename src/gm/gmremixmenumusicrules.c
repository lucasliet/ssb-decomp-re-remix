/**
 * gmremixmenumusicrules.c - pure rules of the Menu Music trap easter egg, see gmremixmenumusicrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmenumusicrules.h"

int gmRemixMenuMusicRulesCountTrapPress(int count)
{
    int presses = count + 1;

    if (presses == 0)
    {
        presses = 1;
    }
    if (presses >= GMREMIX_MENU_MUSIC_TRAP_PRESSES)
    {
        return GMREMIX_MENU_MUSIC_TRAP_ACTIVE;
    }
    return presses;
}

int gmRemixMenuMusicRulesTrapIsActive(int count)
{
    return count < 0;
}

#endif
