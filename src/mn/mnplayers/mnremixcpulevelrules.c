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

#endif
