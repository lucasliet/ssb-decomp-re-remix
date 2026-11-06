#ifndef _GMREMIXSTOCKMODERULES_H_
#define _GMREMIXSTOCKMODERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix Stock Mode panel option (StockMode.asm set_initial_stock_count_), for the headless unit tests (tests/remix). */

/**
 * Stock count of a port in Stock Mode Last. An unset stored count (negative) gives the match count; otherwise the stored
 * count, lowered to the match count unless Tug of War allows higher values.
 *
 * stored_count: previous_stock_count_table of the port. match_stocks: stock setting of the match. is_tug_of_war: nonzero in Tug of War.
 */
extern int gmRemixStockModeRulesLastCount(int stored_count, int match_stocks, int is_tug_of_war);

/**
 * Stock count a port is left with after losing a stock (update_stocks_remaining_): one less, except that a Stamina fighter that is already
 * out of stocks stays at -1 so a second KO does not take it lower.
 *
 * stock_before: stock count of the port before the KO. is_stamina: nonzero in Stamina mode.
 */
extern int gmRemixStockModeRulesCountAfterKo(int stock_before, int is_stamina);

/** Stock count of a port in Stock Mode Manual: the manual count, never above the match count. */
extern int gmRemixStockModeRulesManualCount(int manual_count, int match_stocks);

#endif

#endif
