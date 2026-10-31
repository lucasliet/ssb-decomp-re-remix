#ifndef _GMREMIXSTOCKMODE_H_
#define _GMREMIXSTOCKMODE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Stock count a VS fighter starts with under the Remix StockMode panel option (StockMode.asm set_initial_stock_count_).
 * `match_stocks` is the stock count of the match rules, the returned value replaces it for `player` (0..3).
 */
extern s32 gmRemixStockModeCount(s32 player, s32 match_stocks);

/* Forgets the stock count the Last mode stored (the CSS reset of css/StockMode.asm _check_reset). */
extern void gmRemixStockModeResetLast(void);

/* Restores the stored count from before the last Last match (GameEnd.asm salty runback). */
extern void gmRemixStockModeSaltyRunbackLast(void);

#endif

#endif
