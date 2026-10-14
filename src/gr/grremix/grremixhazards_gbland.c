/**
 * grremixhazards_gbland.c - Remix hazard setup, stage group GbLand. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixgbland.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardGbLand(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__gbland_setup)
    {
        return FALSE;
    }
    grRemixGBLandSetup();
    return TRUE;
}

#endif
