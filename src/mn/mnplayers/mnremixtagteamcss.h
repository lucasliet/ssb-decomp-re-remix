#ifndef _MNREMIXTAGTEAMCSS_H_
#define _MNREMIXTAGTEAMCSS_H_

#include <ssb_types.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Tag Team on the VS character select (TagTeam.asm css scope): each port builds its queue of characters on the CSS, one
 * slot per stock plus one. The queue of a port is filled slot by slot with the fighter of its panel (a pick, a costume
 * change, the preview of a held token); taking the token again goes to the next slot, B with the token in hand empties the
 * slot and goes back one. A row of icons above each panel shows the slots: the stock icon of the fighter, or a dot while empty.
 * The match reads the queue through mnRemixTagTeamCssGetSlot; the slots from 1 are also kept in the CVars of the port menu
 * (Remix > Tag Team) so both read the same data.
 */

/** What the icon row needs to know about a panel, filled by the CSS (mnplayersvs.c) each frame. */
typedef struct MNRemixTagTeamCssPortInfo
{
    sb32 is_in_play;
    s32 stock_count;
    f32 panel_x;
    s32 team;
    sb32 is_team_battle;
    GObj *fighter_gobj;
} MNRemixTagTeamCssPortInfo;

typedef void (*MNRemixTagTeamCssPortInfoFunc)(s32 port, MNRemixTagTeamCssPortInfo *info);

/** TRUE when the picked mode is Tag Team. */
extern sb32 mnRemixTagTeamCssIsActive(void);

/**
 * Makes the icon row of the four panels (setup_) and takes the queue of the previous visit, keeping what the port menu changed.
 * Does nothing outside Tag Team.
 *
 * @param info_func Gives the state of a panel.
 */
extern void mnRemixTagTeamCssSetup(MNRemixTagTeamCssPortInfoFunc info_func);

/**
 * update_slot_: writes the fighter of a panel into the current slot of its port. Call it when the panel shows another fighter,
 * when the token is placed and when the costume changes.
 *
 * @param port Port of the panel.
 * @param fkind Fighter of the panel, nFTKindNull when there is none.
 * @param costume Costume of the panel.
 * @param shade Shade of the panel.
 * @param is_team_costume TRUE in a team battle.
 */
extern void mnRemixTagTeamCssWrite(s32 port, s32 fkind, s32 costume, s32 shade, sb32 is_team_costume);

/** move_to_next_slot_on_pickup: the token of the port was taken (not recalled), so the next pick goes to the next slot. */
extern void mnRemixTagTeamCssNextSlot(s32 port, s32 stock_count);

/**
 * clear_slot_and_go_to_previous_slot_: B with the token in hand.
 *
 * @return TRUE when a slot was emptied (the caller plays the confirm sound and writes the fighter of the panel again).
 */
extern sb32 mnRemixTagTeamCssStepBack(s32 port);

/**
 * adjust_current_slot_on_stock_count_change_: clamps the queue of every port to a new stock count.
 *
 * @param stock_count New 0-based stock count.
 * @return Bit mask of the ports whose current slot went down.
 */
extern u32 mnRemixTagTeamCssStocksChanged(s32 stock_count);

/**
 * prevent_start_with_empty_slots_: every panel in play must have the last slot of its queue filled.
 *
 * @param is_in_play TRUE for each of the four ports that is not N/A.
 * @param stock_counts 0-based stock count of each port.
 * @return TRUE when the match may start.
 */
extern sb32 mnRemixTagTeamCssAllowsStart(const sb32 *is_in_play, const s32 *stock_counts);

/**
 * Queue slot of a port for the match.
 *
 * @param port Port, 0..3.
 * @param slot Slot, 0..5.
 * @param fkind Fighter of the slot, set only when the slot is filled.
 * @param costume Costume of the slot, 0 for a team costume.
 * @param is_classic Classic Sonic flag.
 * @return TRUE when the CSS filled the slot, FALSE when the match must fill it (port menu, Random).
 */
extern sb32 mnRemixTagTeamCssGetSlot(s32 port, s32 slot, s32 *fkind, s32 *costume, sb32 *is_classic);

#endif

#endif
