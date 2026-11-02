/**
 * sc1ptrainingportrules.c - pure rules of the Training per-port chain, see sc1ptrainingportrules.h.
 */

#include "sc/sc1pmode/sc1ptrainingportrules.h"

int32_t sc1PTrainingPortRulesResolveType(int32_t option, int32_t port, int32_t menu_port, int32_t select_cpu_port)
{
    if (port == menu_port)
    {
        return SC1PTRAININGPORTRULES_TYPE_HUMAN;
    }
    if (option != SC1PTRAININGPORTRULES_TYPE_AUTO)
    {
        return option;
    }
    return (port == select_cpu_port) ? SC1PTRAININGPORTRULES_TYPE_CPU : SC1PTRAININGPORTRULES_TYPE_DISABLED;
}

int32_t sc1PTrainingPortRulesPickDummy(const int32_t *resolved_types, int32_t menu_port)
{
    int32_t port;

    for (port = 0; port < SC1PTRAININGPORTRULES_PORT_COUNT; port++)
    {
        if (resolved_types[port] == SC1PTRAININGPORTRULES_TYPE_CPU)
        {
            return port;
        }
    }
    return menu_port;
}

int32_t sc1PTrainingPortRulesCountType(const int32_t *resolved_types, int32_t type)
{
    int32_t port;
    int32_t count = 0;

    for (port = 0; port < SC1PTRAININGPORTRULES_PORT_COUNT; port++)
    {
        if (resolved_types[port] == type)
        {
            count++;
        }
    }
    return count;
}

int32_t sc1PTrainingPortRulesPick(int32_t option, int32_t auto_value)
{
    return (option <= 0) ? auto_value : (option - 1);
}

int32_t sc1PTrainingPortRulesSpawnPort(int32_t option, int32_t port)
{
    if ((option < 1) || (option > SC1PTRAININGPORTRULES_PORT_COUNT))
    {
        return port;
    }
    return option - 1;
}
