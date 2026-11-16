/**
 * ftremixburyrules.c - pure bury timer, see ftremixburyrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <math.h>

#include "ft/ftremix/ftremixburyrules.h"

int ftRemixBuryRulesTimer(int percent)
{
    float scaled = (float) percent * FTREMIX_BURY_RULES_PERCENT_MUL;

    return (int) rintf(scaled) + FTREMIX_BURY_RULES_BASE_TICS;
}

#endif
