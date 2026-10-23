/**
 * gmremixpanel.c - match-side reader of the Remix CSS panel registry (port/remix/remix_player_panel.h).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixpanel.h"
#include "gm/gmremixtoggle.h"

/**
 * Value of a panel option for a port. In the Bonus and 1P Game modes only the human port keeps its values: the asm
 * clears the panel of the CPU ports before those modes (css/X.asm clear_settings_for_1p_).
 */
s32 gmRemixPanelGet(s32 option, s32 player)
{
    if (gmRemixToggleModeIsGuarded() && (gSCManagerBattleState->players[player].pkind != nFTPlayerKindMan))
    {
        return 0;
    }
    return remix_panel_get(option, player);
}

#endif
