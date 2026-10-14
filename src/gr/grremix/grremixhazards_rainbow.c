/**
 * grremixhazards_rainbow.c - Remix hazard setup, stage group Rainbow. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixrainbow.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardRainbow(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__rainbow_road_setup)
    {
        return FALSE;
    }
    grRemixRainbowSetup();
    return TRUE;
}

#endif
