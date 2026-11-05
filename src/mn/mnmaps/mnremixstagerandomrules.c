/**
 * mnremixstagerandomrules.c - pure rules of the RANDOM cell list, see mnremixstagerandomrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnmaps/mnremixstagerandomrules.h"

static int mnRemixStageRandomRulesIsAdded(const MNRemixStageRandomEntry *entry, int is_tournament, int uses_all)
{
    if ((entry->is_on == 0) && (uses_all == 0))
    {
        return 0;
    }
    if ((is_tournament != 0) && (entry->is_tournament_legal == 0))
    {
        return 0;
    }
    return 1;
}

static int mnRemixStageRandomRulesCollect(const MNRemixStageRandomEntry *entries, int entry_count, int is_tournament, int uses_all, int *out_stage_ids)
{
    int index;
    int count = 0;

    for (index = 0; index < entry_count; index++)
    {
        if (mnRemixStageRandomRulesIsAdded(&entries[index], is_tournament, uses_all) != 0)
        {
            out_stage_ids[count++] = entries[index].stage_id;
        }
    }
    return count;
}

int mnRemixStageRandomRulesBuildList(const MNRemixStageRandomEntry *entries, int entry_count, int is_tournament, int *out_stage_ids)
{
    int count = mnRemixStageRandomRulesCollect(entries, entry_count, is_tournament, 0, out_stage_ids);

    if (count != 0)
    {
        return count;
    }
    return mnRemixStageRandomRulesCollect(entries, entry_count, is_tournament, 1, out_stage_ids);
}

#endif
