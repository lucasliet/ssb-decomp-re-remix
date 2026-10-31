/**
 * mnremixcpulevelrules.c - pure CPU level rules of the VS CSS, see mnremixcpulevelrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcpulevelrules.h"

int mnRemixCpuLevelCanRaise(int level)
{
    return (level < MNREMIX_CPU_LEVEL_MAX) ? 1 : 0;
}

int mnRemixCpuLevelNeedsTens(int level)
{
    return (level >= MNREMIX_CPU_LEVEL_MAX) ? 1 : 0;
}

int mnRemixCpuLevelUnitsDigit(int level)
{
    return level % 10;
}

int mnRemixHandicapCanRaise(int value)
{
    return (value < MNREMIX_HANDICAP_MAX) ? 1 : 0;
}

int mnRemixHandicapNeedsIcon(int value)
{
    return (value >= MNREMIX_CPU_LEVEL_MAX) ? 1 : 0;
}

int mnRemixHandicapIconIndex(int value)
{
    if (value < MNREMIX_CPU_LEVEL_MAX)
    {
        return -1;
    }
    return value - MNREMIX_CPU_LEVEL_MAX;
}

#endif
