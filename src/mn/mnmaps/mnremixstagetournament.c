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
    return (s32)remix_ram_image_read_byte(REMIX_SYM_Stages__tournament_hazard_mode_table + gkind);
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
