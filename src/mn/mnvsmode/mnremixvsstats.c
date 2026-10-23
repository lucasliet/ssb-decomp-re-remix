#include "mn/mnvsmode/mnremixvsstats.h"

#ifdef REGION_REMIX

/* REMIX-TODO(results-text): the stats page is not drawn yet. remix_overlay_show_text is an ImGui toast, not a results
 * page layout, so the VsStats.asm draw routines (draw_port_headers_, draw_underline_, draw_line_, run_collect_) are not
 * ported. REMIX-TODO(results-stats-trackers): Z-cancel, tech and ledge trackers (VsStats.asm stat_trackers) have no hooks. */

static s32 sMNRemixVSStatsSpecialCounts[MN_REMIX_VS_STATS_PORT_COUNT][nMNRemixVSStatsSpecialCount];

void mnRemixVSStatsReset(void)
{
    s32 port;
    s32 special;

    for (port = 0; port < MN_REMIX_VS_STATS_PORT_COUNT; port++)
    {
        for (special = 0; special < nMNRemixVSStatsSpecialCount; special++)
        {
            sMNRemixVSStatsSpecialCounts[port][special] = 0;
        }
    }
}

void mnRemixVSStatsCountSpecial(s32 port, MNRemixVSStatsSpecial special)
{
    sMNRemixVSStatsSpecialCounts[port][special]++;
}

s32 mnRemixVSStatsGetSpecialCount(s32 port, MNRemixVSStatsSpecial special)
{
    return sMNRemixVSStatsSpecialCounts[port][special];
}

static s32 mnRemixVSStatsGetSpecialTotal(s32 port)
{
    s32 special;
    s32 total = 0;

    for (special = 0; special < nMNRemixVSStatsSpecialCount; special++)
    {
        total += sMNRemixVSStatsSpecialCounts[port][special];
    }
    return total;
}

s32 mnRemixVSStatsGetSpecialPercent(s32 port, MNRemixVSStatsSpecial special)
{
    s32 total = mnRemixVSStatsGetSpecialTotal(port);

    if (total == 0)
    {
        return 0;
    }
    return sMNRemixVSStatsSpecialCounts[port][special] * 100 / total;
}

#endif
