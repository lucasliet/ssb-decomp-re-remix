/**
 * gmremixtoggle.c - mode guard of the Remix settings (Toggles.single_player_guard, Global.GAMEMODE BONUS and CLASSIC).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixtoggle.h"

sb32 gmRemixToggleModeIsGuarded(void)
{
    s32 game_type;

    if (gSCManagerBattleState == NULL)
    {
        return FALSE;
    }
    game_type = gSCManagerBattleState->game_type;

    return (game_type == nSCBattleGameTypeBonus) || (game_type == nSCBattleGameType1PGame);
}

#endif
