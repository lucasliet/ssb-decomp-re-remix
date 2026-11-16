/**
 * ifremixannouncerrules.c - pure rules of the Pokemon Stadium announcer, see ifremixannouncerrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixannouncerrules.h"

#include <stddef.h>

IFRemixAnnouncement *ifRemixAnnouncerRulesPickStandard(IFRemixAnnouncerTable *table, IFRemixAnnouncerDrawFunc draw)
{
    IFRemixAnnouncement *entry = NULL;
    int attempt;

    for (attempt = 0; attempt < table->pick_count; attempt++)
    {
        entry = &table->entries[draw(table->pick_count)];
        if (entry->timer == 0)
        {
            return entry;
        }
    }
    if ((entry == NULL) || (entry->flag != 0))
    {
        return NULL;
    }
    return entry;
}

/** TRUE when one of the entries the table draws over has no timer left. */
static int ifRemixAnnouncerRulesHasFree(const IFRemixAnnouncerTable *table)
{
    int index;

    for (index = 0; index < table->pick_count; index++)
    {
        if (table->entries[index].timer == 0)
        {
            return 1;
        }
    }
    return 0;
}

IFRemixAnnouncement *ifRemixAnnouncerRulesPickFree(IFRemixAnnouncerTable *table, IFRemixAnnouncerDrawFunc draw)
{
    IFRemixAnnouncement *entry;

    if (ifRemixAnnouncerRulesHasFree(table) == 0)
    {
        return NULL;
    }
    do
    {
        entry = &table->entries[draw(table->pick_count)];
    }
    while (entry->timer != 0);

    return entry;
}

int ifRemixAnnouncerRulesIsSpent(const IFRemixAnnouncerTable *table)
{
    return (table->is_spent_by_first != 0) && (table->entries[0].used != 0);
}

int ifRemixAnnouncerRulesIsLastStock(int stock_count)
{
    return stock_count < 1;
}

int ifRemixAnnouncerRulesChecksSleep(unsigned int global_timer_before, int sleep_requested)
{
    return (global_timer_before != 0) && (sleep_requested != 0);
}

#endif
