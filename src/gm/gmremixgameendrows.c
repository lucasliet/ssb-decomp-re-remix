/**
 * gmremixgameendrows.c - reads the King of the Hill and Smashketball scores for the extra results row, see gmremixgameendrows.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgameendrows.h"
#include "gm/gmremixkoth.h"
#include "gm/gmremixmode.h"
#include "gm/gmremixsmashketballstate.h"
#include "mn/mnvsmode/mnremixvsmode.h"

sb32 gmRemixGameEndRowsHasRow(void)
{
    return (gmRemixModeIs(MNREMIX_VSMODE_KOTH) || gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL)) ? TRUE : FALSE;
}

s32 gmRemixGameEndRowsGetValue(s32 port)
{
    if (gmRemixModeIs(MNREMIX_VSMODE_KOTH))
    {
        return gmRemixKothGetScore(port);
    }
    if (gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL))
    {
        return gGMRemixSmashketball.scores[port];
    }
    return 0;
}

#endif
