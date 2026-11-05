#ifndef _MNREMIXTAGTEAMCSSRULES_H_
#define _MNREMIXTAGTEAMCSSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Tag Team queue on the VS character select (TagTeam.asm css scope: update_slot_, move_to_next_slot_,
 * clear_slot_and_go_to_previous_slot_, adjust_current_slot_on_stock_count_change_, prevent_start_with_empty_slots_), so the
 * headless unit tests (tests/remix) can run them. The stock count here is the 0-based value of the stock picker: a queue
 * has stock count + 1 slots and the last one is slot stock count.
 */

#define MNREMIX_TAGTEAM_CSS_SLOTS 6
#define MNREMIX_TAGTEAM_CSS_NONE (-1)

/** First icon x of a panel above its left edge (setup_: panel x + 5) and the distance between two icons (10). */
#define MNREMIX_TAGTEAM_CSS_ICON_LEFT_PADDING 5
#define MNREMIX_TAGTEAM_CSS_ICON_STRIDE 10

/** Top of the icon row (setup_: y 117). */
#define MNREMIX_TAGTEAM_CSS_ICON_Y 117

/** One queue slot of the CSS: the fighter (NONE when empty), its costume and shade, Classic Sonic, and the slot costume kind. */
typedef struct MNRemixTagTeamCssSlot
{
    int fkind;
    int costume;
    int shade;
    int is_classic;
    int is_team_costume;
    int mirrored_fkind;
} MNRemixTagTeamCssSlot;

/** The queue of one port and the slot the next pick goes to (TagTeam.character_queues and current_slot). */
typedef struct MNRemixTagTeamCssQueue
{
    MNRemixTagTeamCssSlot slots[MNREMIX_TAGTEAM_CSS_SLOTS];
    int current_slot;
} MNRemixTagTeamCssQueue;

/** Empties every slot and goes back to slot 0. */
extern void mnRemixTagTeamCssRulesReset(MNRemixTagTeamCssQueue *queue);

/**
 * Writes the fighter of the panel into the current slot (update_slot_). An empty fighter writes an empty slot.
 *
 * @param queue Queue of the port.
 * @param fkind Fighter of the panel, MNREMIX_TAGTEAM_CSS_NONE when there is none.
 * @param costume Costume of the panel.
 * @param shade Shade of the panel.
 * @param is_classic Classic Sonic flag, only read for Sonic by the caller.
 * @param is_team_costume Nonzero when the costume is a team costume (team battle).
 */
extern void mnRemixTagTeamCssRulesWrite(MNRemixTagTeamCssQueue *queue, int fkind, int costume, int shade, int is_classic, int is_team_costume);

/**
 * Moves to the next slot, stopping on the last one (move_to_next_slot_). A slot past the last one comes back to it.
 *
 * @param queue Queue of the port.
 * @param stock_count 0-based stock count of the port.
 */
extern void mnRemixTagTeamCssRulesNextSlot(MNRemixTagTeamCssQueue *queue, int stock_count);

/**
 * B with a token in hand (clear_slot_and_go_to_previous_slot_): the slot being picked is emptied and the previous one becomes
 * the current slot. Nothing happens on the first slot.
 *
 * @param queue Queue of the port.
 * @return Nonzero when a slot was emptied.
 */
extern int mnRemixTagTeamCssRulesStepBack(MNRemixTagTeamCssQueue *queue);

/**
 * The stock count changed (adjust_current_slot_on_stock_count_change_): a current slot past it comes down to it and the slots
 * above it are emptied.
 *
 * @param queue Queue of the port.
 * @param stock_count New 0-based stock count.
 * @return Nonzero when the current slot went down.
 */
extern int mnRemixTagTeamCssRulesClampSlots(MNRemixTagTeamCssQueue *queue, int stock_count);

/**
 * Start rule (prevent_start_with_empty_slots_): the last slot of the queue is filled, which means all of them are.
 *
 * @param queue Queue of the port.
 * @param stock_count 0-based stock count of the port.
 * @return Nonzero when the queue is complete.
 */
extern int mnRemixTagTeamCssRulesIsComplete(const MNRemixTagTeamCssQueue *queue, int stock_count);

/**
 * X of an icon of the row of a panel (setup_).
 *
 * @param panel_x Left edge of the panel.
 * @param icon Icon, 0 for the first.
 */
extern float mnRemixTagTeamCssRulesIconX(float panel_x, int icon);

/** Number of icons of a port: the 0-based stock count plus one, at most MNREMIX_TAGTEAM_CSS_SLOTS (slot_icon_update_routine_). */
extern int mnRemixTagTeamCssRulesIconCount(int stock_count);

#endif

#endif
