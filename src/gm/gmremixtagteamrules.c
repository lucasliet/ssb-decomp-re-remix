/**
 * gmremixtagteamrules.c - pure queue rules of Tag Team, see gmremixtagteamrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtagteamrules.h"

/* Fighters of the Remix CSS grid a Random slot can pick, in Remix id (the same list as the port menu). */
static const unsigned char dGMRemixTagTeamRulesRandomFighters[] =
{
    0x3F, 0x20, 0x04, 0x00, 0x02, 0x05, 0x03, 0x07, 0x1E, 0x3B, 0x40, 0x1F, 0x0B, 0x06, 0x08,
    0x01, 0x09, 0x0A, 0x1D, 0x3E, 0x41, 0x48, 0x21, 0x49, 0x34, 0x37, 0x38, 0x39, 0x3A, 0x44
};

int gmRemixTagTeamRulesGetSlot(int stocks_set, int stocks_left, int queue_max)
{
    int slot = stocks_set - stocks_left;

    if (slot < 0)
    {
        slot = 0;
    }
    return (slot < queue_max) ? slot : (queue_max - 1);
}

int gmRemixTagTeamRulesIconSlot(int base_slot, int icon_count, int position, int queue_max)
{
    int slot = base_slot + (icon_count - 1 - position);

    if (slot < 0)
    {
        slot = 0;
    }
    return (slot < queue_max) ? slot : (queue_max - 1);
}

int gmRemixTagTeamRulesNeedsSwap(int queue_fkind, int current_fkind)
{
    return (queue_fkind != current_fkind) ? 1 : 0;
}

int gmRemixTagTeamRulesRandomCount(void)
{
    return (int)(sizeof(dGMRemixTagTeamRulesRandomFighters) / sizeof(dGMRemixTagTeamRulesRandomFighters[0]));
}

int gmRemixTagTeamRulesRandomFighter(int random_index)
{
    int count = gmRemixTagTeamRulesRandomCount();

    if (random_index < 0)
    {
        random_index = 0;
    }
    if (random_index >= count)
    {
        random_index = count - 1;
    }
    return dGMRemixTagTeamRulesRandomFighters[random_index];
}

#endif
