/**
 * grremixhazards_bbh.c - Remix hazard setup, stage group Bbh. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixbbh.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardBbh(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__bbh_setup_)
    {
        return FALSE;
    }
    grRemixBbhSetup();
    return TRUE;
}

#endif
