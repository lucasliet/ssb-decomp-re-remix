/**
 * grremixhazards_casino.c - Remix hazard setup, stage group Casino. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixcasino.h"
#include "grremixhazards.h"
#include "remix/remix_symbols.h"
#include <it/item.h>
#include <mp/map.h>
#include <stdint.h>
#include <sys/objtypes.h>

#define GRREMIX_CASINO_BUMPER_POS_X 0.0F
#define GRREMIX_CASINO_BUMPER_POS_Y 2600.0F /* lui 0x4522 in casino_night_setup (Hazards.asm 4413) */
#define GRREMIX_CASINO_BUMPER_POS_Z 0.0F
#define GRREMIX_CASINO_BUMPER_FILE_FIELD 0x80 /* ground data word: pointer to the bumper file (asm add_item_, 0x801313F0) */
#define GRREMIX_CASINO_BUMPER_DL_OFFSET 0x2D0 /* bumper file offset of the casino bumper display list */

/** Spawns the Casino Night stage bumper at its fixed spot (add_item_ item 0x17 is nITKindGBumper). */
static void grRemixCasinoSpawnBumper(void)
{
    Vec3f pos;
    Vec3f vel;
    GObj *bumper_gobj;

    pos.x = GRREMIX_CASINO_BUMPER_POS_X;
    pos.y = GRREMIX_CASINO_BUMPER_POS_Y;
    pos.z = GRREMIX_CASINO_BUMPER_POS_Z;
    vel.x = 0.0F;
    vel.y = 0.0F;
    vel.z = 0.0F;

    bumper_gobj = itManagerMakeItemSetupCommon(NULL, nITKindGBumper, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
    if (bumper_gobj != NULL)
    {
        u32 file_token = *(u32*) ((uintptr_t) gMPCollisionGroundData + GRREMIX_CASINO_BUMPER_FILE_FIELD);

        DObjGetStruct(bumper_gobj)->dl = (Gfx*) ((uintptr_t) PORT_RESOLVE(file_token) + GRREMIX_CASINO_BUMPER_DL_OFFSET);
    }
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
    grRemixCasinoLogBumperWalls();

    if (grRemixHazardsAreOn())
    {
        grRemixCasinoSpawnBumper();
    }
    return TRUE;
}

#endif
