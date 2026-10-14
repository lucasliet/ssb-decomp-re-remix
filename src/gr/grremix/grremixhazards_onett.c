/**
 * grremixhazards_onett.c - Remix hazard setup, stage group Onett. See grremixhazards.h.
 *
 * Hazards.onett_setup (.toolchain/pkgs/smashremix-src/src/Hazards.asm, line 6641) spawns the Car.
 * The car itself is ported in it/itremix/itremixcar.c.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "it/itremix/itremixcar.h"
#include "mp/map.h"
#include "remix/remix_symbols.h"

/* asm onett_setup: the ground data pointer (0x80131300) is the Onett header plus 0x14. */
#define GRREMIX_ONETT_GROUND_DATA_OFFSET 0x14

/** Onett header address: the ground data pointer minus GRREMIX_ONETT_GROUND_DATA_OFFSET. */
static void* grRemixOnettGetHeader(void)
{
    return (u8 *) gMPCollisionGroundData - GRREMIX_ONETT_GROUND_DATA_OFFSET;
}

/** Onett setup (Hazards.onett_setup). Spawns the car unless the hazard toggle is off. */
sb32 grRemixHazardOnett(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__onett_setup)
    {
        return FALSE;
    }
    if (grRemixHazardsAreOn())
    {
        itRemixCarHazardSetup(grRemixOnettGetHeader());
    }

    return TRUE;
}

#endif
