/**
 * mnremixstagetournament.c - tournament layout of the stage select (src/Stages.asm stage_table_tournament,
 * tournament_hazard_mode_table, get_stage_id_; src/Toggles.asm entry_sss_layout).
 *
 * The layout swaps the 5 page grid of stage_table_normal for a 3 page grid of the tournament stages, switches the
 * variants and the L hazard toggle off, and forces the Hazard Mode toggle to the value of the hovered stage.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "gm/gmremixtoggle.h"
#include "gr/grremix/grremixstage.h"
#include "mn/mnmaps/mnremixstagetournament.h"
#include "mn/mnmaps/mnremixstagetournamentrules.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <port_log.h>

/* The second word of Stages.stage_table_pages is the page count of the tournament grid. */
#define MNREMIX_TOURNAMENT_PAGES_WORD_OFFSET 4

/* Stages.tournament_hazard_mode_table (build/remixasm/remix.sym); the generated remix_symbols.h does not list it yet. */
#define MNREMIX_TOURNAMENT_HAZARD_MODE_TABLE 0x8045B1C0u

static sb32 sMNRemixTournamentIsActive = FALSE;

sb32 mnRemixTournamentLatchLayout(void)
{
    sMNRemixTournamentIsActive = (gmRemixToggle(sss_layout) != 0) ? TRUE : FALSE;

    return sMNRemixTournamentIsActive;
}

sb32 mnRemixTournamentIsActive(void)
{
    return sMNRemixTournamentIsActive;
}

s32 mnRemixTournamentGetStages(s32 *out_gkinds, s32 *out_source_pages, s32 max_count)
{
    s32 pages = (s32)remix_ram_image_read_word(REMIX_SYM_Stages__stage_table_pages + MNREMIX_TOURNAMENT_PAGES_WORD_OFFSET);
    s32 entries = pages * MNREMIX_TOURNAMENT_PAGE_ENTRIES;
    s32 index;
    s32 count = 0;

    if (entries > GRREMIX_SELECT_ENTRIES_MAX)
    {
        entries = GRREMIX_SELECT_ENTRIES_MAX;
    }
    for (index = 0; (index < entries) && (count < max_count); index++)
    {
        s32 gkind = (s32)remix_ram_image_read_byte(REMIX_SYM_Stages__stage_table_tournament + index);

        if (gkind > GRREMIX_KIND_MAX)
        {
            continue;
        }
        out_gkinds[count] = gkind;
        out_source_pages[count] = mnRemixTournamentRulesSourcePage(index);
        count++;
    }
    return count;
}

static void mnRemixTournamentSetHazardMode(s32 mode)
{
    if (gmRemixToggle(hazard_mode) != mode)
    {
        port_log("[mnmaps] tournament layout forces hazard mode %d\n", (int)mode);
        remix_toggle_set(REMIX_SYM_Toggles__entry_hazard_mode, mode);
    }
}

static s32 mnRemixTournamentReadTableMode(s32 gkind)
{
    return (s32)remix_ram_image_read_byte(MNREMIX_TOURNAMENT_HAZARD_MODE_TABLE + gkind);
}

void mnRemixTournamentApplyHazardMode(s32 gkind, s32 source_page)
{
    if ((gkind < 0) || (gkind > GRREMIX_KIND_MAX))
    {
        return;
    }
    mnRemixTournamentSetHazardMode(mnRemixTournamentRulesHazardMode(mnRemixTournamentReadTableMode(gkind), gkind, source_page));
}

void mnRemixTournamentApplyRandomHazardMode(s32 gkind)
{
    if ((gkind < 0) || (gkind > GRREMIX_KIND_MAX))
    {
        return;
    }
    mnRemixTournamentSetHazardMode(mnRemixTournamentReadTableMode(gkind));
}

#endif
