/**
 * gmremixstockmoderules.c - pure Stock Mode rules, see gmremixstockmoderules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixstockmoderules.h"

int gmRemixStockModeRulesLastCount(int stored_count, int match_stocks, int is_tug_of_war)
{
    if (stored_count < 0)
    {
        return match_stocks;
    }
    if (is_tug_of_war != 0)
    {
        return stored_count;
    }
    return (stored_count < match_stocks) ? stored_count : match_stocks;
}

int gmRemixStockModeRulesCountAfterKo(int stock_before, int is_stamina)
{
    if ((is_stamina != 0) && (stock_before == -1))
    {
        return -1;
    }
    return stock_before - 1;
}

int gmRemixStockModeRulesManualCount(int manual_count, int match_stocks)
{
    if (manual_count < 0)
    {
        return match_stocks;
    }
    return (manual_count < match_stocks) ? manual_count : match_stocks;
}

int gmRemixStockModeRulesCssCount(int stored_count, int match_stocks, int is_tug_of_war)
{
    return gmRemixStockModeRulesLastCount(stored_count, match_stocks, is_tug_of_war);
}

int gmRemixStockModeRulesStepManualCount(int manual_count, int direction, int match_stocks)
{
    int next = ((manual_count < 0) ? match_stocks : manual_count) + direction;

    if (next < 0)
    {
        return match_stocks;
    }
    return (next > match_stocks) ? 0 : next;
}

#endif
