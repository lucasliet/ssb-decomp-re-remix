/**
 * grremixhazards_corneria.c - Remix hazard setup, stage group Corneria. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixcorneria.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardCorneria(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__corneria_setup)
    {
        return FALSE;
    }
    grRemixCorneriaSetup();

    return TRUE;
}

#endif
