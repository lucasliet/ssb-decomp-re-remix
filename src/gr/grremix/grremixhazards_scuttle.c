/**
 * grremixhazards_scuttle.c - Remix hazard setup, stage group Scuttle Town. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixscuttle.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardScuttleTown(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__scuttle_town_setup_)
    {
        return FALSE;
    }

    grRemixScuttleTownSetup();
    return TRUE;
}

#endif
