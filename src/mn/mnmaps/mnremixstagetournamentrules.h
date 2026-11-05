#ifndef _MNREMIXSTAGETOURNAMENTRULES_H_
#define _MNREMIXSTAGETOURNAMENTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the tournament layout of the Remix stage select (Stages.asm get_stage_id_, stage_table_tournament), so the
 * headless unit tests (tests/remix) can run them. */

/* Stages.NUM_ICONS: the stages of one page of the Remix grid, the RANDOM square included. */
#define MNREMIX_TOURNAMENT_PAGE_ENTRIES 18

/**
 * Page of the Remix tournament grid (0 based) that holds a stage_table_tournament entry. The same stage can sit on several pages
 * and the page decides its forced hazard mode.
 *
 * @param table_index Index of the entry in stage_table_tournament.
 */
extern int mnRemixTournamentRulesSourcePage(int table_index);

/**
 * Hazard mode the tournament layout forces for a hovered stage (get_stage_id_). Dream Land, Yoshi's Island DL and Saffron City DL
 * sit on two pages with different rules: Dream Land is Hazards OFF on page 3, Yoshi's Island DL is Hazards ON on pages 1 and 2
 * and Saffron City DL is Movement OFF on page 3. Every other stage, and those on their other pages, take the table value.
 *
 * @param table_mode Stages.tournament_hazard_mode_table entry of the stage.
 * @param gkind Stage id.
 * @param source_page Page of the Remix tournament grid the hovered entry sits on.
 * @return Hazards.mode value to write to the Hazard Mode toggle.
 */
extern int mnRemixTournamentRulesHazardMode(int table_mode, int gkind, int source_page);

#endif

#endif
