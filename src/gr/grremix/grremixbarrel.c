/**
 * grremixbarrel.c - ground obstacle capacity of the Remix barrel stages, see grremixbarrel.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gr/grremix/grremixbarrel.h"
#include "gr/grremix/grremixids.h"

s32 grRemixBarrelGetObstacleCapacity(void)
{
    if (gSCManagerBattleState->gkind == nGRRemixStage_BTP_BANJO)
    {
        return GRREMIX_BARREL_OBSTACLES_BANJO;
    }
    return GRREMIX_BARREL_OBSTACLES_VANILLA;
}

#endif
