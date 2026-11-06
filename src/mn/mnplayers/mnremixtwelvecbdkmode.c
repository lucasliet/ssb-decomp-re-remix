/**
 * mnremixtwelvecbdkmode.c - DK Mode easter egg of the 12-Character Battle select, see mnremixtwelvecbdkmode.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixdkmode.h"
#include "mn/mnplayers/mnremixtwelvecbcustom.h"
#include "mn/mnplayers/mnremixtwelvecbdkmode.h"
#include "mn/mnplayers/mnremixtwelvecbdkmoderules.h"

void mnRemixTwelveCBDKModeUpdate(void)
{
    int player_one_fkinds[MNREMIX_TCB_DK_MODE_SLOTS];
    int player_two_fkinds[MNREMIX_TCB_DK_MODE_SLOTS];
    int slot;

    for (slot = 0; slot < MNREMIX_TCB_DK_MODE_SLOTS; slot++)
    {
        player_one_fkinds[slot] = mnRemixTwelveCBCustomFkindOf(0, slot);
        player_two_fkinds[slot] = mnRemixTwelveCBCustomFkindOf(1, slot);
    }
    ftRemixDKModeSetMode(mnRemixTwelveCBDKModeSelect(player_one_fkinds, player_two_fkinds));
}

#endif
