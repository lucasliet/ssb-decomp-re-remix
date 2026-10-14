/**
 * grremixhazards_metallic.c - Remix hazard setup, stage group Metallic. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixmetallic.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardMetallic(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__metallic_madness_setup)
    {
        return FALSE;
    }
    if (grRemixHazardsAreOn())
    {
        grRemixMetallicSetup();
    }
    return TRUE;
}

#endif
