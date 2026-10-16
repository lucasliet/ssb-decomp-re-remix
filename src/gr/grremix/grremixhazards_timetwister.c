/**
 * grremixhazards_timetwister.c - Remix hazard setup, stage group Time Twister. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixtimetwister.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardTimeTwister(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__time_twister_setup_)
    {
        return FALSE;
    }

    grRemixTimeTwisterSetup();
    return TRUE;
}

#endif
