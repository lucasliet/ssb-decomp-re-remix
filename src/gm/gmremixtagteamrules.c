/**
 * gmremixtagteamrules.c - pure queue rules of Tag Team, see gmremixtagteamrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtagteamrules.h"

int gmRemixTagTeamRulesGetSlot(int stocks_set, int stocks_left, int queue_max)
{
    int slot = stocks_set - stocks_left;

    if (slot < 0)
    {
        slot = 0;
    }
    return (slot < queue_max) ? slot : (queue_max - 1);
}

int gmRemixTagTeamRulesSkippedStocks(int is_last, int global_stocks, int base_stocks)
{
    int skipped = global_stocks - base_stocks;

    if ((is_last == 0) || (skipped < 0))
    {
        return 0;
    }
    return skipped;
}

int gmRemixTagTeamRulesQueueIndex(int initial_stocks, int stocks_left, int stolen_count, int skipped_stocks, int queue_max)
{
    return gmRemixTagTeamRulesGetSlot(initial_stocks + skipped_stocks, stocks_left + stolen_count, queue_max);
}

int gmRemixTagTeamRulesShade(int fkind, int team, const int *previous_fkinds, const int *previous_teams, int previous_count)
{
    int shade = 0;
    int i;

    for (i = 0; i < previous_count; i++)
    {
        if ((previous_fkinds[i] == fkind) && (previous_teams[i] == team))
        {
            shade++;
        }
    }
    return shade;
}

int gmRemixTagTeamRulesIconSlot(int base_slot, int icon_count, int position, int queue_max)
{
    int slot = base_slot + (icon_count - 1 - position);

    if (slot < 0)
    {
        slot = 0;
    }
    return (slot < queue_max) ? slot : (queue_max - 1);
}

int gmRemixTagTeamRulesNeedsSwap(int queue_fkind, int queue_costume, int queue_shade, int queue_is_classic, int current_fkind, int current_costume, int current_shade, int current_is_classic)
{
    if (queue_fkind != current_fkind)
    {
        return 1;
    }
    if ((queue_is_classic != 0) != (current_is_classic != 0))
    {
        return 1;
    }
    return ((queue_costume != current_costume) || (queue_shade != current_shade)) ? 1 : 0;
}

#endif
