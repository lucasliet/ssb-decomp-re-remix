/**
 * grremixhazards_pirate.c - Remix hazard setup, stage group Pirate. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixpiratecannon.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardPirate(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__pirate_land_setup)
    {
        return FALSE;
    }
    if (grRemixHazardsAreOn())
    {
        grRemixPirateMakeGround();
    }
    return TRUE;
}

#endif
