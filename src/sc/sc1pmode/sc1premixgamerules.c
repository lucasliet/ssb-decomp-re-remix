/**
 * sc1premixgamerules.c - stage setup rules of the Remix 1P modes, see sc1premixgamerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixids.h"
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1premixgamerules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

int sc1PRemixModeSpawnLR(int gkind, int lr)
{
    return (gkind == nGRRemixStage_REMIX_RTTF) ? -1 : lr;
}

int sc1PRemixModeFlashAlpha(int mode, int table_alpha)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_BONUS3:
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
        return SC1PREMIX_GAME_HORDE_FLASH_ALPHA;

    default:
        return table_alpha;
    }
}

int sc1PRemixModeEnemyTrait(int mode, int stage, int table_trait)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_BONUS3:
        return SC1PREMIX_GAME_TRAIT_BONUS3;

    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
        return SC1PREMIX_GAME_TRAIT_POLYGON_TEAM;

    case SC1PREMIX_MODE_ALLSTAR:
        return (stage == SC1PREMIX_GAME_STAGE_ALLSTAR_TRIPLE) ? SC1PREMIX_GAME_TRAIT_DEFAULT : table_trait;

    default:
        return table_trait;
    }
}

int sc1PRemixModeHordeEnemyLevel(int mode, int level)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_MULTIMAN:
        return SC1PREMIX_GAME_MULTIMAN_LEVEL;

    case SC1PREMIX_MODE_CRUEL:
        return SC1PREMIX_GAME_CRUEL_LEVEL;

    default:
        return level;
    }
}

static int sc1PRemixIsPolygonFighter(int fkind)
{
    if (fkind >= REMIX_ID_NWARIO)
    {
        return 1;
    }
    return ((fkind >= REMIX_ID_NMARIO) && (fkind <= REMIX_ID_NNESS)) ? 1 : 0;
}

static int sc1PRemixIsPolygonStage(int gkind)
{
    if (gkind < nGRRemixStage_DUEL_ZONE)
    {
        return 0;
    }
    if ((gkind == nGRRemixStage_BATTLEFIELD) || (gkind == nGRRemixStage_REMIX_RTTF))
    {
        return 1;
    }
    return (gkind <= nGRRemixStage_RACE_TO_THE_FINISH) ? 1 : 0;
}

int sc1PRemixModePolygonTeamCostume(int human_costume, int human_fkind, int gkind)
{
    if (human_costume != 0)
    {
        return 0;
    }
    if (sc1PRemixIsPolygonFighter(human_fkind) == 0)
    {
        return 0;
    }
    return sc1PRemixIsPolygonStage(gkind);
}

int sc1PRemixModeKeepsEnemyStocks(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_BONUS3:
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
    case SC1PREMIX_MODE_HRC:
        return 1;

    default:
        return 0;
    }
}

int sc1PRemixModeEndsAllstarRun(int mode, int stage, int is_stage_won)
{
    if (mode != SC1PREMIX_MODE_ALLSTAR)
    {
        return 0;
    }
    return ((stage == SC1PREMIX_GAME_STAGE_ALLSTAR_FINAL) && (is_stage_won != 0)) ? 1 : 0;
}

int sc1PRemixModeAwardsNoDamage(int mode)
{
    return (mode != SC1PREMIX_MODE_ALLSTAR) ? 1 : 0;
}

int sc1PRemixModeAwardsBrosCalamity(int mode)
{
    return (mode == SC1PREMIX_MODE_STANDARD) ? 1 : 0;
}

unsigned int sc1PRemixModeItemToggles(int mode, unsigned int item_toggles)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_REMIX_1P:
        return item_toggles;

    case SC1PREMIX_MODE_ALLSTAR:
        return item_toggles & SC1PREMIX_GAME_ALLSTAR_ITEM_MASK;

    default:
        return item_toggles & SC1PREMIX_GAME_VANILLA_ITEM_MASK;
    }
}

unsigned int sc1PRemixModeExtendedItemToggles(int mode, unsigned int current, unsigned int default_toggles)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_REMIX_1P:
        return current;

    case SC1PREMIX_MODE_ALLSTAR:
        return default_toggles;

    default:
        return 0;
    }
}

#endif
