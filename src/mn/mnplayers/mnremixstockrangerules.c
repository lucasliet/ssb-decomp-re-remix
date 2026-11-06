/**
 * mnremixstockrangerules.c - pure rules of the stock picker arrows, see mnremixstockrangerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixstockrangerules.h"

#define MNREMIX_STOCK_RANGE_TAG_TEAM_MIN 1
#define MNREMIX_STOCK_RANGE_TAG_TEAM_MAX 5
#define MNREMIX_STOCK_RANGE_TUG_OF_WAR_MIN 0
#define MNREMIX_STOCK_RANGE_TUG_OF_WAR_MAX 24
#define MNREMIX_STOCK_RANGE_DEFAULT_MIN 0

static int mnRemixStockRangeMin(int range)
{
    switch (range)
    {
    case MNREMIX_STOCK_RANGE_TAG_TEAM:
        return MNREMIX_STOCK_RANGE_TAG_TEAM_MIN;

    case MNREMIX_STOCK_RANGE_TUG_OF_WAR:
        return MNREMIX_STOCK_RANGE_TUG_OF_WAR_MIN;

    default:
        return MNREMIX_STOCK_RANGE_DEFAULT_MIN;
    }
}

static int mnRemixStockRangeMax(int range, int default_max)
{
    switch (range)
    {
    case MNREMIX_STOCK_RANGE_TAG_TEAM:
        return MNREMIX_STOCK_RANGE_TAG_TEAM_MAX;

    case MNREMIX_STOCK_RANGE_TUG_OF_WAR:
        return MNREMIX_STOCK_RANGE_TUG_OF_WAR_MAX;

    default:
        return default_max;
    }
}

int mnRemixStockRangeNext(int range, int value, int default_max)
{
    if ((value + 1) > mnRemixStockRangeMax(range, default_max))
    {
        return mnRemixStockRangeMin(range);
    }
    return value + 1;
}

int mnRemixStockRangePrev(int range, int value, int default_max)
{
    if ((value - 1) < mnRemixStockRangeMin(range))
    {
        return mnRemixStockRangeMax(range, default_max);
    }
    return value - 1;
}

#endif
