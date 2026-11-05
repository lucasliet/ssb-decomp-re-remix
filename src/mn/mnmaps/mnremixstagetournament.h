#ifndef _MNREMIXSTAGETOURNAMENT_H_
#define _MNREMIXSTAGETOURNAMENT_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Reads the Stage Select Layout toggle (Toggles.entry_sss_layout) for the screen that is opening. The layout cannot change
 * while the screen is open, as in Remix where the setting is changed from the settings menu.
 *
 * @return TRUE when the tournament layout is on.
 */
extern sb32 mnRemixTournamentLatchLayout(void);

/** TRUE when the screen opened with the tournament layout. */
extern sb32 mnRemixTournamentIsActive(void);

/**
 * Forces the Hazard Mode toggle to the value the tournament layout gives the hovered stage (get_stage_id_). Does nothing for
 * a stage without a table entry.
 *
 * @param gkind Hovered stage id.
 * @param source_page Page of the tournament grid (0 based) the cursor is on.
 */
extern void mnRemixTournamentApplyHazardMode(s32 gkind, s32 source_page);

/**
 * Forces the Hazard Mode toggle for the stage the RANDOM square picked (Stages.asm swap_stage_._end_random, which looks the
 * stage up without its page).
 *
 * @param gkind Stage id that was drawn.
 */
extern void mnRemixTournamentApplyRandomHazardMode(s32 gkind);

#endif

#endif
