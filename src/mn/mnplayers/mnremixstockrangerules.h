#ifndef _MNREMIXSTOCKRANGERULES_H_
#define _MNREMIXSTOCKRANGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the stock picker arrows of the Remix VS character select (TwelveCharBattle.asm handle_stock_count_change_). Values
 * are the 0-based stock counts the CSS keeps: the picker shows the value plus one. Tag Team allows 2 to 6 stocks, Tug of War 1 to
 * 25 and every other mode 1 to 99.
 */

#define MNREMIX_STOCK_RANGE_DEFAULT 0
#define MNREMIX_STOCK_RANGE_TAG_TEAM 1
#define MNREMIX_STOCK_RANGE_TUG_OF_WAR 2

/**
 * Value after the right arrow: one more, or the lowest value of the range after its highest.
 *
 * @param range MNREMIX_STOCK_RANGE_*.
 * @param value Current 0-based stock value.
 * @param default_max Highest value of the default range (98, or fewer where the screen limits it).
 */
extern int mnRemixStockRangeNext(int range, int value, int default_max);

/**
 * Value after the left arrow: one less, or the highest value of the range below its lowest.
 *
 * @param range MNREMIX_STOCK_RANGE_*.
 * @param value Current 0-based stock value.
 * @param default_max Highest value of the default range (98, or fewer where the screen limits it).
 */
extern int mnRemixStockRangePrev(int range, int value, int default_max);

#endif

#endif
