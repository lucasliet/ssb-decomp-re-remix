/**
 * gmremixtimedstock.c - Timed Stock, the Remix stock rule that also ends the match by the timer (TimedStock.asm).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixstockmode.h"
#include "mn/mnvsmode/mnremixvsmode.h"

#define GMREMIX_MODE_SCORE_STOCK_SHIFT 12
#define GMREMIX_MODE_SCORE_TUG_STOCK_SHIFT 24

sb32 gmRemixTimedStockIsActive(void)
{
    return (gSCManagerTransferBattleState.game_rules == SCBATTLE_GAMERULE_TIMED_STOCK) ? TRUE : FALSE;
}

sb32 gmRemixTimedStockIsTimeUp(void)
{
    if (gSCManagerTransferBattleState.time_limit == SCBATTLE_TIMELIMIT_INFINITE)
    {
        return FALSE;
    }
    return (gSCManagerTransferBattleState.time_remain == 0) ? TRUE : FALSE;
}

sb32 gmRemixTimedStockUsesTimeScore(void)
{
    if (gSCManagerTransferBattleState.game_rules == SCBATTLE_GAMERULE_TIME)
    {
        return TRUE;
    }
    return gmRemixTimedStockIsTimeUp();
}

s32 gmRemixTimedStockScore(s32 player)
{
    SCPlayerData *data = &gSCManagerTransferBattleState.players[player];
    s32 stocks_left;

    if (gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR) != FALSE)
    {
        stocks_left = data->stock_count + 1;

        return (stocks_left << GMREMIX_MODE_SCORE_TUG_STOCK_SHIFT) + ((data->score - data->falls) << GMREMIX_MODE_SCORE_STOCK_SHIFT) - data->stock_damage_all;
    }
    stocks_left = gmRemixStockModeCount(player, gSCManagerTransferBattleState.stocks) - data->falls + 1;

    if (stocks_left == 0)
    {
        return 0;
    }
    return (stocks_left << GMREMIX_MODE_SCORE_STOCK_SHIFT) - data->stock_damage_all;
}

sb32 gmRemixTimedStockAllowsSuddenDeath(void)
{
    if (gmRemixModeIs(MNREMIX_VSMODE_KOTH) != FALSE)
    {
        return FALSE;
    }
    return (gSCManagerBattleState->game_rules == SCBATTLE_GAMERULE_TIME) ? TRUE : FALSE;
}

#endif
