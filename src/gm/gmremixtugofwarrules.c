/**
 * gmremixtugofwarrules.c - pure credit rules of Tug of War, see gmremixtugofwarrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtugofwarrules.h"

int gmRemixTugOfWarRulesIsValidPort(int port)
{
    return ((port >= 0) && (port < GMREMIX_TUGOFWARRULES_PORTS)) ? 1 : 0;
}

int gmRemixTugOfWarPickCreditPort(int victim_team, int koer_team, int koer_port, int last_hit_port)
{
    if (gmRemixTugOfWarRulesIsValidPort(koer_port) == 0)
    {
        return last_hit_port;
    }
    if (koer_team != victim_team)
    {
        return koer_port;
    }
    return last_hit_port;
}

int gmRemixTugOfWarShouldRevive(int stock_count_before)
{
    return (stock_count_before == -1) ? 1 : 0;
}

#endif
