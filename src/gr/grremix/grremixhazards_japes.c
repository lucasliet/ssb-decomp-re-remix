/**
 * grremixhazards_japes.c - Remix hazard setup, stage group Japes. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixjapes.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardJapes(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__jungle_japes_setup)
    {
        return FALSE;
    }
    grRemixJapesMakeGround();
    return TRUE;
}

#endif
