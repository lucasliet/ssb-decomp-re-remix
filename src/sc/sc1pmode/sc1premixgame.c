/**
 * sc1premixgame.c - stage setup rules of the Remix 1P modes on the 1P game, see sc1premixgame.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include "it/itremix/itremixspawn.h"
#include "sc/sc1pmode/sc1premixgame.h"
#include "sc/sc1pmode/sc1premixgamerules.h"
#include "sc/sc1pmode/sc1premixmode.h"

s32 sc1PRemixGameSpawnFacing(s32 lr)
{
    return sc1PRemixModeSpawnLR(gSCManagerBattleState->gkind, lr);
}

s32 sc1PRemixGameFlashAlpha(s32 table_alpha)
{
    return sc1PRemixModeFlashAlpha(sc1PRemixModeGet(), table_alpha);
}

s32 sc1PRemixGameEnemyTrait(s32 table_trait)
{
    return sc1PRemixModeEnemyTrait(sc1PRemixModeGet(), gSCManagerSceneData.spgame_stage, table_trait);
}

s32 sc1PRemixGameHordeEnemyLevel(s32 level)
{
    return sc1PRemixModeHordeEnemyLevel(sc1PRemixModeGet(), level);
}

/** First human of the battle (the asm walks the ports until the player type is Man); the 1P player when none is found. */
static s32 sc1PRemixGameGetHumanPlayer(void)
{
    s32 player;

    for (player = 0; player < ARRAY_COUNT(gSCManagerBattleState->players); player++)
    {
        if (gSCManagerBattleState->players[player].pkind == nFTPlayerKindMan)
        {
            return player;
        }
    }
    return gSCManagerSceneData.player;
}

s32 sc1PRemixGamePolygonTeamCostume(void)
{
    s32 human = sc1PRemixGameGetHumanPlayer();

    return sc1PRemixModePolygonTeamCostume(gSCManagerBattleState->players[human].costume, gSCManagerBattleState->players[human].fkind, gSCManagerBattleState->gkind);
}

sb32 sc1PRemixGameKeepsEnemyStocks(void)
{
    return (sc1PRemixModeKeepsEnemyStocks(sc1PRemixModeGet()) != 0) ? TRUE : FALSE;
}

void sc1PRemixGameOnStageEnd(sb32 is_stage_won)
{
    if (sc1PRemixModeEndsAllstarRun(sc1PRemixModeGet(), gSCManagerSceneData.spgame_stage, is_stage_won) != 0)
    {
        gSCManagerSceneData.spgame_stage = nSC1PGameStageBoss;
    }
}

sb32 sc1PRemixGameAwardsNoDamage(void)
{
    return (sc1PRemixModeAwardsNoDamage(sc1PRemixModeGet()) != 0) ? TRUE : FALSE;
}

sb32 sc1PRemixGameAwardsBrosCalamity(void)
{
    return (sc1PRemixModeAwardsBrosCalamity(sc1PRemixModeGet()) != 0) ? TRUE : FALSE;
}

void sc1PRemixGameSetupItemToggles(void)
{
    s32 mode = sc1PRemixModeGet();

    gSCManagerBattleState->item_toggles = sc1PRemixModeItemToggles(mode, gSCManagerBattleState->item_toggles);
    gITRemixExtendedToggles = sc1PRemixModeExtendedItemToggles(mode, gITRemixExtendedToggles, ITREMIX_EXTENDED_TOGGLES_DEFAULT);
}

#endif
