/**
 * grremixhazards_spawnedfear.c - Remix hazard setup, stage group SpawnedFear. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixspawnedfear.h"
#include "remix/remix_symbols.h"

sb32 grRemixHazardSpawnedFear(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__spawned_fear_acid_setup_)
    {
        return FALSE;
    }
    grRemixSpawnedFearSetup();

    return TRUE;
}

#endif
