/**
 * grremixhazards_peach2.c - Remix hazard setup, stage group Peach Castle II. See grremixhazards.h.
 * Ported from Hazards.asm peach_castle_II_setup_.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "grremixbanzai.h"
#include "mp/map.h"
#include "remix/remix_symbols.h"

#define GRREMIX_PEACH2_HEADER_TO_GROUND_DATA 0x14 /* asm peach_castle_II_setup_: header = 0x80131300 pointer - 0x14 */

/**
 * peach_castle_II_setup_ (asm). The caller checks grRemixHazardsAreOn() first, as the asm checks
 * Toggles.entry_hazard_mode. Builds the Banzai Bill factory on the stage header.
 */
static void grRemixPeachCastle2Setup(void)
{
    void *header = (u8 *)gMPCollisionGroundData - GRREMIX_PEACH2_HEADER_TO_GROUND_DATA;

    grRemixBanzaiFactoryMake(header);
}

sb32 grRemixHazardPeachCastle2(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__peach_castle_II_setup_)
    {
        return FALSE;
    }
    if (grRemixHazardsAreOn())
    {
        grRemixPeachCastle2Setup();
    }
    return TRUE;
}

#endif
