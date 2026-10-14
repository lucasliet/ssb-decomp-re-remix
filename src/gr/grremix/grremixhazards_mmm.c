/**
 * grremixhazards_mmm.c - Remix hazard setup, stage group Mmm. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixmmm.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardMmm(s32 gkind, u32 function)
{
    switch (function)
    {
    case REMIX_SYM_Hazards__mmm_setup:
        grRemixMmmMakeGround();
        return TRUE;

    default:
        return FALSE;
    }
}

#endif
