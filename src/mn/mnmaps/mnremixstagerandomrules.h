#ifndef _MNREMIXSTAGERANDOMRULES_H_
#define _MNREMIXSTAGERANDOMRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the list the RANDOM cell of the Remix stage select draws from (Stages.asm swap_stage_ and
 * add_stage_to_random_list_), so the headless unit tests (tests/remix) can run them. */

/* One candidate of the list, in the order swap_stage_ adds them. */
typedef struct MNRemixStageRandomEntry
{
    int stage_id;
    int is_on; /* Value of its Random Stage Toggle. */
    int is_tournament_legal; /* Toggles.profile_defaults_TE of its toggle. */

} MNRemixStageRandomEntry;

/**
 * Builds the random list (add_stage_to_random_list_ for every entry of swap_stage_ `_build_stage_list`). An entry joins when its
 * toggle is on, and on the tournament layout also when it is tournament legal. When that leaves nothing, the pass is repeated
 * with every toggle counted as on (`a2 = TRUE`); the tournament filter still applies.
 *
 * @param entries Candidates, in list order.
 * @param entry_count Number of candidates.
 * @param is_tournament Stage Select Layout is the tournament one.
 * @param out_stage_ids Receives the list, at least entry_count ints.
 * @return Number of stage ids written; 0 when even the second pass finds none.
 */
extern int mnRemixStageRandomRulesBuildList(const MNRemixStageRandomEntry *entries, int entry_count, int is_tournament, int *out_stage_ids);

#endif

#endif
