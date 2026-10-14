/**
 * grremixhazards_casino.c - Remix hazard setup, stage group Casino. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include "remix/remix_symbols.h"
#include <it/item.h>

#define GRREMIX_CASINO_BUMPER_POS_X 0.0F
#define GRREMIX_CASINO_BUMPER_POS_Y 2600.0F /* lui 0x4522 in casino_night_setup (Hazards.asm 4413) */
#define GRREMIX_CASINO_BUMPER_POS_Z 0.0F

/** Spawns the Casino Night stage bumper at its fixed spot (add_item_ item 0x17 is nITKindGBumper). */
static void grRemixCasinoSpawnBumper(void)
{
    Vec3f pos;
    Vec3f vel;

    pos.x = GRREMIX_CASINO_BUMPER_POS_X;
    pos.y = GRREMIX_CASINO_BUMPER_POS_Y;
    pos.z = GRREMIX_CASINO_BUMPER_POS_Z;
    vel.x = 0.0F;
    vel.y = 0.0F;
    vel.z = 0.0F;

    /* REMIX-TODO(stage bumper display list): casino_night_setup (Hazards.asm 4420-4426) points the bumper DObj dl at
       file 0x9C + 0x2D0. The file pointer is stored by add_item_ at N64 MPGroundData+0x80 (Hazards.asm 972-977), which
       has no port field yet, so the bumper keeps its default display list. */
    itManagerMakeItemSetupCommon(NULL, nITKindGBumper, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
}

/**
 * Remix hazard setup of Casino Night Zone. Returns TRUE for its setup address; the bumper is spawned only when hazards
 * are on (Hazards.asm 4405-4408 skip the setup for hazard_mode 1 or 3).
 */
sb32 grRemixHazardCasino(s32 gkind, u32 function)
{
    if (function != REMIX_SYM_Hazards__casino_night_setup)
    {
        return FALSE;
    }
    if (grRemixHazardsAreOn())
    {
        grRemixCasinoSpawnBumper();
    }
    return TRUE;
}

#endif
