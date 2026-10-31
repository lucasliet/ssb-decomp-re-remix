#include "mn/mnvsmode/mnremixvsstats.h"
#include "mn/mnvsmode/mnremixvsstatsrules.h"

#ifdef REGION_REMIX

static s32 sMNRemixVSStatsSpecialCounts[MN_REMIX_VS_STATS_PORT_COUNT][nMNRemixVSStatsSpecialCount];
static s32 sMNRemixVSStatsTrackerCounts[MN_REMIX_VS_STATS_PORT_COUNT][nMNRemixVSStatsTrackerCount];
static s32 sMNRemixVSStatsHighestPercents[MN_REMIX_VS_STATS_PORT_COUNT];

void mnRemixVSStatsReset(void)
{
    s32 port;
    s32 special;
    s32 tracker;

    for (port = 0; port < MN_REMIX_VS_STATS_PORT_COUNT; port++)
    {
        sMNRemixVSStatsHighestPercents[port] = 0;

        for (special = 0; special < nMNRemixVSStatsSpecialCount; special++)
        {
            sMNRemixVSStatsSpecialCounts[port][special] = 0;
        }
        for (tracker = 0; tracker < nMNRemixVSStatsTrackerCount; tracker++)
        {
            sMNRemixVSStatsTrackerCounts[port][tracker] = 0;
        }
    }
}

void mnRemixVSStatsCountTracker(s32 port, MNRemixVSStatsTracker tracker)
{
    sMNRemixVSStatsTrackerCounts[port][tracker]++;
}

s32 mnRemixVSStatsGetTracker(s32 port, MNRemixVSStatsTracker tracker)
{
    return sMNRemixVSStatsTrackerCounts[port][tracker];
}

s32 mnRemixVSStatsGetTrackerPercent(s32 port, MNRemixVSStatsTracker success, MNRemixVSStatsTracker miss)
{
    s32 part = sMNRemixVSStatsTrackerCounts[port][success];
    s32 total = part + sMNRemixVSStatsTrackerCounts[port][miss];

    return mnRemixVSStatsRulesPercent(part, total);
}

void mnRemixVSStatsRecordPercent(s32 port, s32 percent)
{
    sMNRemixVSStatsHighestPercents[port] = mnRemixVSStatsRulesHighest(sMNRemixVSStatsHighestPercents[port], percent);
}

s32 mnRemixVSStatsGetHighestPercent(s32 port)
{
    return sMNRemixVSStatsHighestPercents[port];
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
