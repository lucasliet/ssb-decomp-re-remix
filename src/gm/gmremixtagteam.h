#ifndef _GMREMIXTAGTEAM_H_
#define _GMREMIXTAGTEAM_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Smash Remix Tag Team (TagTeam.asm): each port plays a queue of characters, one per stock, instead of one fighter.
 * Slot 0 is the character picked in the CSS, slots 1 to 5 come from the port menu (Remix > Tag Team, Random by default).
 * The queue is as long as the stock count plus one. When a fighter loses a stock the next character of its queue takes
 * its place (load_next_char_). Port deviations: the stock icons of the HUD keep the first character and a stolen stock
 * in teams does not move a queue slot (REMIX-TODO(tagteam-icons), REMIX-TODO(tagteam-steal)).
 */

#define GMREMIX_TAGTEAM_QUEUE_MAX 6

/** TRUE during a Tag Team match. */
extern sb32 gmRemixTagTeamIsActive(void);

/** Match start: builds the queue of every port (random slots are resolved here). */
extern void gmRemixTagTeamSetup(void);

/**
 * TagTeam.asm load_next_char_: called when a fighter that lost a stock is about to respawn. When the next character of its
 * queue is another one, the fighter is destroyed and the new one takes its port, items and projectiles.
 *
 * @param fighter_gobj Fighter that lost the stock.
 * @return The fighter that respawns (the new one, or fighter_gobj when the character does not change).
 */
extern GObj* gmRemixTagTeamLoadNextFighter(GObj *fighter_gobj);

#endif

#endif
