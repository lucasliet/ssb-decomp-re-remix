/**
 * mnremixtagteamcssrules.c - pure rules of the Tag Team queue on the VS character select, see mnremixtagteamcssrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixtagteamcssrules.h"

/* The asm queue has 6 slots whatever the stock count; a higher count would write into the queue of the next port. */
static int mnRemixTagTeamCssRulesLastSlot(int stock_count)
{
    if (stock_count < 0)
    {
        return 0;
    }
    return (stock_count >= MNREMIX_TAGTEAM_CSS_SLOTS) ? (MNREMIX_TAGTEAM_CSS_SLOTS - 1) : stock_count;
}

static void mnRemixTagTeamCssRulesClearSlot(MNRemixTagTeamCssSlot *slot)
{
    slot->fkind = MNREMIX_TAGTEAM_CSS_NONE;
    slot->costume = 0;
    slot->shade = 0;
    slot->is_classic = 0;
    slot->is_team_costume = 0;
}

void mnRemixTagTeamCssRulesReset(MNRemixTagTeamCssQueue *queue)
{
    int slot;

    for (slot = 0; slot < MNREMIX_TAGTEAM_CSS_SLOTS; slot++)
    {
        mnRemixTagTeamCssRulesClearSlot(&queue->slots[slot]);
        queue->slots[slot].mirrored_fkind = MNREMIX_TAGTEAM_CSS_NONE;
    }
    queue->current_slot = 0;
}

void mnRemixTagTeamCssRulesWrite(MNRemixTagTeamCssQueue *queue, int fkind, int costume, int shade, int is_classic, int is_team_costume)
{
    MNRemixTagTeamCssSlot *slot = &queue->slots[queue->current_slot];

    if (fkind == MNREMIX_TAGTEAM_CSS_NONE)
    {
        mnRemixTagTeamCssRulesClearSlot(slot);
        return;
    }
    slot->fkind = fkind;
    slot->costume = costume;
    slot->shade = shade;
    slot->is_classic = is_classic;
    slot->is_team_costume = is_team_costume;
}

void mnRemixTagTeamCssRulesNextSlot(MNRemixTagTeamCssQueue *queue, int stock_count)
{
    stock_count = mnRemixTagTeamCssRulesLastSlot(stock_count);

    if (queue->current_slot == stock_count)
    {
        return;
    }
    queue->current_slot = (queue->current_slot < stock_count) ? (queue->current_slot + 1) : stock_count;
}

int mnRemixTagTeamCssRulesStepBack(MNRemixTagTeamCssQueue *queue)
{
    if (queue->current_slot == 0)
    {
        return 0;
    }
    mnRemixTagTeamCssRulesClearSlot(&queue->slots[queue->current_slot]);
    queue->current_slot--;

    return 1;
}

int mnRemixTagTeamCssRulesClampSlots(MNRemixTagTeamCssQueue *queue, int stock_count)
{
    int slot;

    stock_count = mnRemixTagTeamCssRulesLastSlot(stock_count);

    if (queue->current_slot <= stock_count)
    {
        return 0;
    }
    for (slot = stock_count + 1; (slot <= queue->current_slot) && (slot < MNREMIX_TAGTEAM_CSS_SLOTS); slot++)
    {
        mnRemixTagTeamCssRulesClearSlot(&queue->slots[slot]);
    }
    queue->current_slot = stock_count;

    return 1;
}

int mnRemixTagTeamCssRulesIsComplete(const MNRemixTagTeamCssQueue *queue, int stock_count)
{
    return (queue->slots[mnRemixTagTeamCssRulesLastSlot(stock_count)].fkind != MNREMIX_TAGTEAM_CSS_NONE) ? 1 : 0;
}

float mnRemixTagTeamCssRulesIconX(float panel_x, int icon)
{
    return panel_x + (float)MNREMIX_TAGTEAM_CSS_ICON_LEFT_PADDING + (float)(MNREMIX_TAGTEAM_CSS_ICON_STRIDE * icon);
}

int mnRemixTagTeamCssRulesIconCount(int stock_count)
{
    int count = stock_count + 1;

    if (count < 0)
    {
        return 0;
    }
    return (count > MNREMIX_TAGTEAM_CSS_SLOTS) ? MNREMIX_TAGTEAM_CSS_SLOTS : count;
}

#endif
