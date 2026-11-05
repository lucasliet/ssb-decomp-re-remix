/**
 * mnremixtagteamcss.c - Tag Team queue of the VS character select, see mnremixtagteamcss.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/ftdef.h>
#include "ft/ftremix/sonic/ftremixsonicclassic.h"
#include "gm/gmremixmode.h"
#include "mn/mnplayers/mnremixtagteamcss.h"
#include "mn/mnplayers/mnremixtagteamcssicons.h"
#include "mn/mnplayers/mnremixtagteamcssrules.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_modes.h"
#include "remix/remix_roster_data.h"

#define MNREMIX_TAGTEAM_CSS_PORTS 4

/* TagTeam.character_queues and current_slot; they outlive the CSS like the asm tables, so a later visit shows the same queues. */
static MNRemixTagTeamCssQueue sMNRemixTagTeamCssQueues[MNREMIX_TAGTEAM_CSS_PORTS];
static sb32 sMNRemixTagTeamCssIsInit = FALSE;

static void mnRemixTagTeamCssInit(void)
{
    s32 port;

    if (sMNRemixTagTeamCssIsInit != FALSE)
    {
        return;
    }
    sMNRemixTagTeamCssIsInit = TRUE;

    for (port = 0; port < MNREMIX_TAGTEAM_CSS_PORTS; port++)
    {
        mnRemixTagTeamCssRulesReset(&sMNRemixTagTeamCssQueues[port]);
    }
}

/** Keeps the CVar of the port menu equal to a slot from 1 (the menu names only the fighters of the CSS grid, the rest reads as Random). */
static void mnRemixTagTeamCssMirrorSlot(s32 port, s32 slot)
{
    MNRemixTagTeamCssSlot *queue_slot = &sMNRemixTagTeamCssQueues[port].slots[slot];

    if (slot == 0)
    {
        return;
    }
    queue_slot->mirrored_fkind = remix_modes_tagteam_set_slot(port, slot, queue_slot->fkind);
}

/** A slot the port menu changed since the CSS wrote it takes the fighter of the menu; Random empties it. */
static void mnRemixTagTeamCssTakeMenuSlot(s32 port, s32 slot)
{
    MNRemixTagTeamCssSlot *queue_slot = &sMNRemixTagTeamCssQueues[port].slots[slot];
    s32 menu_fkind = remix_modes_tagteam_slot(port, slot);

    if (menu_fkind == queue_slot->mirrored_fkind)
    {
        return;
    }
    queue_slot->fkind = (menu_fkind >= 0) ? menu_fkind : MNREMIX_TAGTEAM_CSS_NONE;
    queue_slot->costume = 0;
    queue_slot->shade = 0;
    queue_slot->is_classic = 0;
    queue_slot->is_team_costume = 0;
    queue_slot->mirrored_fkind = menu_fkind;
}

sb32 mnRemixTagTeamCssIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_TAG_TEAM);
}

void mnRemixTagTeamCssSetup(MNRemixTagTeamCssPortInfoFunc info_func)
{
    s32 port;
    s32 slot;

    if (mnRemixTagTeamCssIsActive() == FALSE)
    {
        return;
    }
    mnRemixTagTeamCssInit();

    for (port = 0; port < MNREMIX_TAGTEAM_CSS_PORTS; port++)
    {
        for (slot = 1; slot < MNREMIX_TAGTEAM_CSS_SLOTS; slot++)
        {
            mnRemixTagTeamCssTakeMenuSlot(port, slot);
        }
    }
    mnRemixTagTeamCssIconsMake(sMNRemixTagTeamCssQueues, info_func);
}

void mnRemixTagTeamCssWrite(s32 port, s32 fkind, s32 costume, s32 shade, sb32 is_team_costume)
{
    MNRemixTagTeamCssQueue *queue = &sMNRemixTagTeamCssQueues[port];
    sb32 is_classic;
    s32 previous_fkind;

    if (mnRemixTagTeamCssIsActive() == FALSE)
    {
        return;
    }
    mnRemixTagTeamCssInit();

    if ((fkind == nFTKindNull) || (fkind < 0))
    {
        fkind = MNREMIX_TAGTEAM_CSS_NONE;
    }
    is_classic = (fkind == REMIX_ID_SONIC) ? ftRemixSonicIsClassic(port) : FALSE;

    previous_fkind = queue->slots[queue->current_slot].fkind;
    mnRemixTagTeamCssRulesWrite(queue, fkind, costume, shade, is_classic, is_team_costume);

    if (previous_fkind != queue->slots[queue->current_slot].fkind)
    {
        mnRemixTagTeamCssMirrorSlot(port, queue->current_slot);
    }
}

void mnRemixTagTeamCssNextSlot(s32 port, s32 stock_count)
{
    if (mnRemixTagTeamCssIsActive() == FALSE)
    {
        return;
    }
    mnRemixTagTeamCssInit();
    mnRemixTagTeamCssRulesNextSlot(&sMNRemixTagTeamCssQueues[port], stock_count);
}

sb32 mnRemixTagTeamCssStepBack(s32 port)
{
    MNRemixTagTeamCssQueue *queue = &sMNRemixTagTeamCssQueues[port];
    s32 cleared_slot = queue->current_slot;

    if (mnRemixTagTeamCssIsActive() == FALSE)
    {
        return FALSE;
    }
    mnRemixTagTeamCssInit();

    if (mnRemixTagTeamCssRulesStepBack(queue) == 0)
    {
        return FALSE;
    }
    mnRemixTagTeamCssMirrorSlot(port, cleared_slot);

    return TRUE;
}

u32 mnRemixTagTeamCssStocksChanged(s32 stock_count)
{
    u32 went_down = 0;
    s32 port;
    s32 slot;

    if (mnRemixTagTeamCssIsActive() == FALSE)
    {
        return 0;
    }
    mnRemixTagTeamCssInit();

    for (port = 0; port < MNREMIX_TAGTEAM_CSS_PORTS; port++)
    {
        if (mnRemixTagTeamCssRulesClampSlots(&sMNRemixTagTeamCssQueues[port], stock_count) == 0)
        {
            continue;
        }
        went_down |= 1U << port;

        for (slot = 1; slot < MNREMIX_TAGTEAM_CSS_SLOTS; slot++)
        {
            if ((sMNRemixTagTeamCssQueues[port].slots[slot].fkind == MNREMIX_TAGTEAM_CSS_NONE) &&
                (sMNRemixTagTeamCssQueues[port].slots[slot].mirrored_fkind != MNREMIX_TAGTEAM_CSS_NONE))
            {
                mnRemixTagTeamCssMirrorSlot(port, slot);
            }
        }
    }
    return went_down;
}

sb32 mnRemixTagTeamCssAllowsStart(const sb32 *is_in_play, const s32 *stock_counts)
{
    s32 port;

    if (mnRemixTagTeamCssIsActive() == FALSE)
    {
        return TRUE;
    }
    mnRemixTagTeamCssInit();

    for (port = 0; port < MNREMIX_TAGTEAM_CSS_PORTS; port++)
    {
        if ((is_in_play[port] != FALSE) && (mnRemixTagTeamCssRulesIsComplete(&sMNRemixTagTeamCssQueues[port], stock_counts[port]) == 0))
        {
            return FALSE;
        }
    }
    return TRUE;
}

sb32 mnRemixTagTeamCssGetSlot(s32 port, s32 slot, s32 *fkind, s32 *costume, sb32 *is_classic)
{
    const MNRemixTagTeamCssSlot *queue_slot;

    if ((port < 0) || (port >= MNREMIX_TAGTEAM_CSS_PORTS) || (slot < 0) || (slot >= MNREMIX_TAGTEAM_CSS_SLOTS))
    {
        return FALSE;
    }
    mnRemixTagTeamCssInit();

    if (slot > 0)
    {
        mnRemixTagTeamCssTakeMenuSlot(port, slot);
    }
    queue_slot = &sMNRemixTagTeamCssQueues[port].slots[slot];

    if (queue_slot->fkind == MNREMIX_TAGTEAM_CSS_NONE)
    {
        return FALSE;
    }
    *fkind = queue_slot->fkind;
    *costume = (queue_slot->is_team_costume != 0) ? 0 : queue_slot->costume;
    *is_classic = (queue_slot->is_classic != 0) ? TRUE : FALSE;

    return TRUE;
}

#endif
