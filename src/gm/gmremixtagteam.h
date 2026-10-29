#ifndef _GMREMIXTAGTEAM_H_
#define _GMREMIXTAGTEAM_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

/*
 * Smash Remix Tag Team (TagTeam.asm): each port plays a queue of characters, one per stock, instead of one fighter.
 * Slot 0 is the character picked in the CSS, slots 1 to 5 come from the port menu (Remix > Tag Team, Random by default).
 * The queue is as long as the stock count plus one. When a fighter loses a stock the next character of its queue takes
 * its place (load_next_char_). The HUD lists the queue in reverse, one icon per character left (gmremixtagteamicons.c), and a
 * stock stolen in teams carries its character to the thief. Port deviation: the Stock Mode LAST offset of the slot is not applied.
 */

#define GMREMIX_TAGTEAM_QUEUE_MAX 6

/** TRUE during a Tag Team match. */
extern sb32 gmRemixTagTeamIsActive(void);

/** Match start: builds the queue of every port (random slots are resolved here). */
extern void gmRemixTagTeamSetup(void);

/**
 * TagTeam.asm show_all_icons_: gives the stock icon at a position of the HUD the icon of its queue character.
 *
 * @param sobj Sprite object of the icon.
 * @param port Port of the HUD.
 * @param position Icon, 0 for the leftmost.
 * @param icon_count Icons on the HUD.
 * @return TRUE when the icon was set, FALSE outside Tag Team or when the character has no icon.
 */
extern sb32 gmRemixTagTeamApplyStockIcon(SObj *sobj, s32 port, s32 position, s32 icon_count);

/**
 * TagTeam.asm handle_stock_stealing_: records the stock a port steals from a teammate (the thief plays its character) and
 * gives the flying stock icon that character.
 *
 * @param thief Port that takes the stock.
 * @param stolen_port Port the stock comes from.
 * @param sobj Sprite object of the flying icon.
 * @return TRUE when the icon was set, FALSE outside Tag Team or when the character has no icon.
 */
extern sb32 gmRemixTagTeamStealStock(s32 thief, s32 stolen_port, SObj *sobj);

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
