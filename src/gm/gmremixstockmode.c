/**
 * gmremixstockmode.c - VS stock count of the Remix StockMode panel option (StockMode.asm set_initial_stock_count_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmode.h"
#include "gm/gmremixpanel.h"
#include "gm/gmremixstamina.h"
#include "gm/gmremixstockmode.h"
#include "gm/gmremixstockmoderules.h"
#include "gm/gmremixtwelvecb.h"
#include "mn/mnvsmode/mnremixvsmode.h"

#define GMREMIX_STOCK_MODE_LAST 1
#define GMREMIX_STOCK_MODE_MANUAL 2
#define GMREMIX_STOCK_MODE_PORTS 4
#define GMREMIX_STOCK_COUNT_UNSET (-1)

/* previous_stock_count_table and initial_previous_stock_count_table of StockMode.asm, per port. Both start unset (-1). */
static s32 sStockModeLastCount[GMREMIX_STOCK_MODE_PORTS] =
{
    GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET
};
static s32 sStockModeInitialLastCount[GMREMIX_STOCK_MODE_PORTS] =
{
    GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET
};

/* initial_stock_count_table of StockMode.asm: the stock count each port started the match with, for every Stock Mode. */
static s32 sStockModeInitialCount[GMREMIX_STOCK_MODE_PORTS] =
{
    GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET, GMREMIX_STOCK_COUNT_UNSET
};

/** Hooks for the CSS reset and the salty runback. Call them from the port; see docs/remix/modes_base.md (Stock Mode). */
void gmRemixStockModeResetLast(void);
void gmRemixStockModeSaltyRunbackLast(void);
sb32 gmRemixStockModeIsManual(s32 player);

/**
 * Stock count of a fighter in Last mode (StockMode.asm set_initial_stock_count_, _use_last). The first match after a CSS
 * reset stores its own count; later matches reuse the stored count, lowered to the match count unless Tug of War allows
 * higher values.
 *
 * @param player Port index of the fighter (0..3).
 * @param match_stocks Stock count of the match rules.
 * @return The stock count of the fighter.
 */
static s32 gmRemixStockModeLastCount(s32 player, s32 match_stocks)
{
    s32 last_count = sStockModeLastCount[player];

    sStockModeInitialLastCount[player] = last_count;
    if (last_count < 0)
    {
        sStockModeLastCount[player] = match_stocks;
    }
    return gmRemixStockModeRulesLastCount(last_count, match_stocks, gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR));
}

/**
 * Clears the Last stock counts of every port, as the CSS Stock option does on a CSS reset (StockMode.asm onchange_handler,
 * _check_reset). The next match stores its own count again.
 */
void gmRemixStockModeResetLast(void)
{
    s32 player;

    for (player = 0; player < GMREMIX_STOCK_MODE_PORTS; player++)
    {
        sStockModeLastCount[player] = GMREMIX_STOCK_COUNT_UNSET;
    }
}

/**
 * Restores the Last stock counts from the start of the last match, for a salty runback (GameEnd.asm _success, which
 * copies initial_previous_stock_count_table back to previous_stock_count_table).
 */
void gmRemixStockModeSaltyRunbackLast(void)
{
    s32 player;

    for (player = 0; player < GMREMIX_STOCK_MODE_PORTS; player++)
    {
        sStockModeLastCount[player] = sStockModeInitialLastCount[player];
    }
}

sb32 gmRemixStockModeIsManual(s32 player)
{
    return (gmRemixPanel(STOCK_MODE, player) == GMREMIX_STOCK_MODE_MANUAL) ? TRUE : FALSE;
}

/**
 * Initial stock count of a port (StockMode.asm set_initial_stock_count_): the 12-Character Battle pool, the Last or Manual
 * count, or the match count.
 *
 * @param player Port index of the fighter (0..3).
 * @param match_stocks Stock count of the match rules (gSCManagerBattleState->stocks).
 * @return The initial stock count of the port.
 */
static s32 gmRemixStockModeComputeInitial(s32 player, s32 match_stocks)
{
    s32 manual_count;

    if (gmRemixTwelveCBIsActive() != FALSE)
    {
        return gmRemixTwelveCBGetStocks(player, match_stocks);
    }
    if (gmRemixPanel(STOCK_MODE, player) == GMREMIX_STOCK_MODE_LAST)
    {
        return gmRemixStockModeLastCount(player, match_stocks);
    }
    if (gmRemixPanel(STOCK_MODE, player) != GMREMIX_STOCK_MODE_MANUAL)
    {
        return match_stocks;
    }
    manual_count = gmRemixPanel(STOCK_COUNT, player);
    return gmRemixStockModeRulesManualCount(manual_count, match_stocks);
}

/**
 * Match start: stores the initial stock count of a port (initial_stock_count_table) and returns the stock count its fighter
 * starts with. Stamina gives one stock (Stamina.asm stamina_stock, which runs after set_initial_stock_count_, so the stored
 * initial count is still the Stock Mode one). Calls once per port and match: Last mode stores its count here.
 *
 * @param player Port index of the fighter (0..3).
 * @param match_stocks Stock count of the match rules (gSCManagerBattleState->stocks).
 * @return The stock count of the fighter.
 */
s32 gmRemixStockModeBeginMatch(s32 player, s32 match_stocks)
{
    s32 initial_count = gmRemixStockModeComputeInitial(player, match_stocks);

    sStockModeInitialCount[player] = initial_count;
    if (gmRemixStaminaIsActive() != FALSE)
    {
        return 0;
    }
    return initial_count;
}

s32 gmRemixStockModeGetInitialCount(s32 player)
{
    return sStockModeInitialCount[player];
}

s32 gmRemixStockModeGetLastCount(s32 player)
{
    return sStockModeLastCount[player];
}

sb32 gmRemixStockModeIsLast(s32 player)
{
    return (gmRemixPanel(STOCK_MODE, player) == GMREMIX_STOCK_MODE_LAST) ? TRUE : FALSE;
}

void gmRemixStockModeSetLastCount(s32 player, s32 count)
{
    sStockModeLastCount[player] = count;
}

#endif
