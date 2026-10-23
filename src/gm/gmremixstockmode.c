/**
 * gmremixstockmode.c - VS stock count of the Remix StockMode panel option (StockMode.asm set_initial_stock_count_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixpanel.h"
#include "gm/gmremixstockmode.h"

#define GMREMIX_STOCK_MODE_MANUAL 2

/* REMIX-TODO(StockMode): the Last mode (1) is not ported. The asm reuses the count the CSS stock counter last stored
 * (CharacterSelect.asm 5627, GameEnd.asm 138), and the port has no CSS stock counter, so Last uses the match count. */

/**
 * Stock count a VS fighter starts with. The Manual count only lowers the match count, as set_initial_stock_count_ keeps the
 * smaller of the two.
 *
 * @param player Port index of the fighter (0..3).
 * @param match_stocks Stock count of the match rules (gSCManagerBattleState->stocks).
 * @return The stock count of the fighter.
 */
s32 gmRemixStockModeCount(s32 player, s32 match_stocks)
{
    s32 manual_count;

    if (gmRemixPanel(STOCK_MODE, player) != GMREMIX_STOCK_MODE_MANUAL)
    {
        return match_stocks;
    }
    manual_count = gmRemixPanel(STOCK_COUNT, player);
    return (manual_count < match_stocks) ? manual_count : match_stocks;
}

#endif
