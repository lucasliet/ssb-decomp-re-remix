/**
 * grremixhazards_turnpike.c - Remix hazard setup, stage group Turnpike. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixturnpike.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardTurnpike(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__toads_turnpike_setup)
    {
        return FALSE;
    }

    grRemixTurnpikeMakeGround();
    return TRUE;
}

#endif
