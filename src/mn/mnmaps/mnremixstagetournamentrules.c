/**
 * mnremixstagetournamentrules.c - pure rules of the tournament layout of the stage select, see mnremixstagetournamentrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixids.h"
#include "mn/mnmaps/mnremixstagepanelrules.h"
#include "mn/mnmaps/mnremixstagetournamentrules.h"

/* get_stage_id_ `sltiu t1, t3, 2`: pages 1 and 2 of the grid against page 3. */
#define MNREMIX_TOURNAMENT_LAST_PAGE_INDEX 2

int mnRemixTournamentRulesSourcePage(int table_index)
{
    return table_index / MNREMIX_TOURNAMENT_PAGE_ENTRIES;
}

int mnRemixTournamentRulesHazardMode(int table_mode, int gkind, int source_page)
{
    int is_last_page = (source_page >= MNREMIX_TOURNAMENT_LAST_PAGE_INDEX) ? 1 : 0;

    if ((gkind == nGRRemixStage_DREAM_LAND) && (is_last_page != 0))
    {
        return MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON;
    }
    if ((gkind == nGRRemixStage_YOSHI_ISLAND_DL) && (is_last_page == 0))
    {
        return MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON;
    }
    if ((gkind == nGRRemixStage_SAFFRON_DL) && (is_last_page != 0))
    {
        return MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF;
    }
    return table_mode;
}

#endif
