#ifndef _GMREMIXTAGTEAMRULES_H_
#define _GMREMIXTAGTEAMRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure queue rules of Tag Team (TagTeam.asm start_match_setup_, load_next_char_), so the headless unit tests (tests/remix) can run them. */

/**
 * Queue slot of a port's current stock: the stocks set minus the stocks left, clamped to 0 to queue_max - 1.
 *
 * stocks_set: stock setting of the match. stocks_left: stock count of the fighter of the port.
 * queue_max: length of the queue, GMREMIX_TAGTEAM_QUEUE_MAX in the caller.
 */
extern int gmRemixTagTeamRulesGetSlot(int stocks_set, int stocks_left, int queue_max);

/**
 * Stocks of the queue a Stock Mode Last port skips (TagTeam.asm use_character_queues_on_load_ and load_next_char_: the stock
 * setting minus the stock count the port plays with). 0 when the mode is not Last. A negative difference (the stock setting
 * lowered below the stored count) gives 0; the asm would index before the queue there.
 *
 * is_last: nonzero when the port's Stock Mode is Last. global_stocks: stock setting of the match. base_stocks: stored previous
 * stock count (match start) or initial stock count (swap).
 */
extern int gmRemixTagTeamRulesSkippedStocks(int is_last, int global_stocks, int base_stocks);

/**
 * Queue slot of a port's fighter on a swap (TagTeam.asm load_next_char_): initial stock count minus the stocks left minus the
 * stocks stolen from the port, plus the skipped stocks of Stock Mode Last. Clamped to 0 to queue_max - 1.
 */
extern int gmRemixTagTeamRulesQueueIndex(int initial_stocks, int stocks_left, int stolen_count, int skipped_stocks, int queue_max);

/**
 * Shade of a queue slot in a team battle (TagTeam.asm start_match_setup_ _teams_loop): one more for each earlier port whose
 * same slot holds the same fighter on the same team.
 *
 * previous_fkinds, previous_teams: fighter and team of the same slot in every earlier port, previous_count entries.
 */
extern int gmRemixTagTeamRulesShade(int fkind, int team, const int *previous_fkinds, const int *previous_teams, int previous_count);

/**
 * Queue slot a stock icon shows (TagTeam.asm show_all_icons_): the HUD lists the queue in reverse, the rightmost icon is the
 * current fighter (base_slot) and the leftmost one is the last character of the queue. Clamped to 0 to queue_max - 1.
 *
 * base_slot: queue slot of the current fighter. icon_count: icons on the HUD. position: icon, 0 for the leftmost.
 * queue_max: length of the queue, GMREMIX_TAGTEAM_QUEUE_MAX in the caller.
 */
extern int gmRemixTagTeamRulesIconSlot(int base_slot, int icon_count, int position, int queue_max);

/**
 * TRUE (1) when the fighter must be replaced by the queue slot: another fighter, or the same fighter with another costume or
 * shade (the asm only repaints that case, TagTeam.asm load_next_char_ _update_costume; the port builds the fighter again).
 *
 * Classic Sonic is part of the slot (TagTeam.asm load_next_char_ "if is_classic changed, then we will have to load"): the same
 * Sonic with the other model also needs a swap.
 *
 * queue_*: fkind, costume, shade and Classic Sonic flag of the queue slot. current_*: the same of the fighter that lost the stock
 * (the flag is 0 for any fighter that is not Sonic).
 */
extern int gmRemixTagTeamRulesNeedsSwap(int queue_fkind, int queue_costume, int queue_shade, int queue_is_classic, int current_fkind, int current_costume, int current_shade, int current_is_classic);

/** Number of fighters a Random slot can pick. */
extern int gmRemixTagTeamRulesRandomCount(void);

/**
 * Fighter of a Random slot for a random index, the index is clamped to the Random list (below 0 gives the first fighter,
 * past the end gives the last one).
 *
 * random_index: index from 0 to gmRemixTagTeamRulesRandomCount() - 1, usually from syUtilsRandIntRange.
 *
 * Returns the fkind of the fighter.
 */
extern int gmRemixTagTeamRulesRandomFighter(int random_index);

#endif

#endif
