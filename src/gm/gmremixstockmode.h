#ifndef _GMREMIXSTOCKMODE_H_
#define _GMREMIXSTOCKMODE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Match start (StockMode.asm set_initial_stock_count_): stores the initial stock count of `player` (0..3) under the Remix
 * StockMode panel option and returns the stock count its fighter starts with. `match_stocks` is the stock count of the match rules.
 * Call it once per port and match: Last mode stores its count on the first call after a CSS reset.
 */
extern s32 gmRemixStockModeBeginMatch(s32 player, s32 match_stocks);

/* initial_stock_count_table: the initial stock count stored by the last gmRemixStockModeBeginMatch, -1 before any match. */
extern s32 gmRemixStockModeGetInitialCount(s32 player);

/* previous_stock_count_table: the Last mode stock count stored for the port, -1 when unset. */
extern s32 gmRemixStockModeGetLastCount(s32 player);

/* TRUE when the StockMode option of the port's panel is Last. */
extern sb32 gmRemixStockModeIsLast(s32 player);

/* Stores the Last mode stock count of the port (Tug of War writes it when a stock is credited, TugOfWar.asm credit_stock_to_koer_). */
extern void gmRemixStockModeSetLastCount(s32 player, s32 count);

/**
 * Keeps the stock count a port is left with after a KO, which the Last mode reuses in the next match (TwelveCharBattle.asm
 * update_stocks_remaining_, the path outside the 12-Character Battle). Does nothing outside the VS match and in the 12-Character
 * Battle, which keeps its own pools.
 *
 * @param player Port index of the fighter that lost the stock (0..3).
 * @param stock_before Stock count of the port before the KO.
 */
extern void gmRemixStockModeOnKo(s32 player, s32 stock_before);

/* TRUE when the StockMode option of the port's panel is Manual. */
extern sb32 gmRemixStockModeIsManual(s32 player);

/* Forgets the stock count the Last mode stored (the CSS reset of css/StockMode.asm _check_reset). */
extern void gmRemixStockModeResetLast(void);

/* Restores the stored count from before the last Last match (GameEnd.asm salty runback). */
extern void gmRemixStockModeSaltyRunbackLast(void);

#endif

#endif
