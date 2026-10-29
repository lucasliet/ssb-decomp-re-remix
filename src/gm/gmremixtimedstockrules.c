/**
 * gmremixtimedstockrules.c - pure score arithmetic of Timed Stock, see gmremixtimedstockrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtimedstockrules.h"

int gmRemixTimedStockRulesStockScore(int stocks_left, int stock_damage)
{
    if (stocks_left == 0)
    {
        return 0;
    }
    return (stocks_left << GMREMIX_TIMEDSTOCK_STOCK_SHIFT) - stock_damage;
}

int gmRemixTimedStockRulesTugScore(int stock_count, int kos, int falls, int stock_damage)
{
    return ((stock_count + 1) << GMREMIX_TIMEDSTOCK_TUG_STOCK_SHIFT) + ((kos - falls) << GMREMIX_TIMEDSTOCK_STOCK_SHIFT) - stock_damage;
}

int gmRemixTimedStockRulesIsTimeUp(int time_limit, int time_remain, int infinite)
{
    if (time_limit == infinite)
    {
        return 0;
    }
    return (time_remain == 0) ? 1 : 0;
}

#endif
