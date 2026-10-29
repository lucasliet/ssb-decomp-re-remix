/**
 * gmremixtimedstock.c - Timed Stock, the Remix stock rule that also ends the match by the timer (TimedStock.asm).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixstockmode.h"
#include "gm/gmremixtimedstockrules.h"
#include "mn/mnvsmode/mnremixvsmode.h"

sb32 gmRemixTimedStockIsActive(void)
{
    return (gSCManagerTransferBattleState.game_rules == SCBATTLE_GAMERULE_TIMED_STOCK) ? TRUE : FALSE;
}

sb32 gmRemixTimedStockIsTimeUp(void)
{
    return (gmRemixTimedStockRulesIsTimeUp(gSCManagerTransferBattleState.time_limit, gSCManagerTransferBattleState.time_remain, SCBATTLE_TIMELIMIT_INFINITE) != 0) ? TRUE : FALSE;
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
        return gmRemixTimedStockRulesTugScore(data->stock_count, data->score, data->falls, data->stock_damage_all);
    }
    stocks_left = gmRemixStockModeCount(player, gSCManagerTransferBattleState.stocks) - data->falls + 1;

    return gmRemixTimedStockRulesStockScore(stocks_left, data->stock_damage_all);
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
