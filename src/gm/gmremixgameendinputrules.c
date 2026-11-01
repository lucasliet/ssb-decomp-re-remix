/**
 * gmremixgameendinputrules.c - exact button match of the VS match end, see gmremixgameendinputrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgameendinputrules.h"

int gmRemixGameEndInputMatchesExactly(unsigned int held, unsigned int mask)
{
    return ((mask != 0) && (held == mask)) ? 1 : 0;
}

#endif
