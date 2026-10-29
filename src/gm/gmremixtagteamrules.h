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
 * TRUE (1) when the queue character of the slot is another fighter than the current one, so the fighter must be swapped.
 *
 * queue_fkind: fkind of the queue slot. current_fkind: fkind of the fighter that lost the stock.
 */
extern int gmRemixTagTeamRulesNeedsSwap(int queue_fkind, int current_fkind);

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
