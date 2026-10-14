/**
 * grremixhazards_rttf.c - Remix hazard setup, stage group Rttf. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "remix/remix_symbols.h"
#include <gr/grbonus/grbonus3.h>

sb32 grRemixHazardRttf(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__remix_rttf_setup_)
    {
        return FALSE;
    }

    grBonus3MakeGround();

    return TRUE;
}

#endif
