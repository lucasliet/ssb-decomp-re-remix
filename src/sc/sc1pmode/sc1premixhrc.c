/**
 * sc1premixhrc.c - Home-Run Contest rules on the 1P game, see sc1premixhrc.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include "gr/grremix/grremixids.h"
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1premixhrc.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmodename.h"

#define SC1PREMIX_HRC_STOCK_COUNT 0

sb32 sc1PRemixHrcIsActive(void)
{
    if (sc1PRemixModeIs(SC1PREMIX_MODE_HRC) == FALSE)
    {
        return FALSE;
    }
    return (gSCManagerSceneData.spgame_stage == nSC1PGameStageLink) ? TRUE : FALSE;
}

void sc1PRemixHrcSetupBattle(void)
{
    if (sc1PRemixHrcIsActive() == FALSE)
    {
        return;
    }
    gSCManagerBattleState->gkind = nGRRemixStage_HRC;
    gSCManagerBattleState->item_appearance_rate = 0;
    gSCManagerBattleState->time_limit = SCBATTLE_TIMELIMIT_INFINITE;
}

void sc1PRemixHrcSetupPlayer(s32 player)
{
    if (sc1PRemixHrcIsActive() == FALSE)
    {
        return;
    }
    gSCManager1PGameBattleState.players[player].stock_count = SC1PREMIX_HRC_STOCK_COUNT;
}

void sc1PRemixHrcSetupEnemy(s32 player)
{
    if (sc1PRemixHrcIsActive() == FALSE)
    {
        return;
    }
    gSCManagerBattleState->players[player].fkind = REMIX_ID_SANDBAG;
}

#endif
