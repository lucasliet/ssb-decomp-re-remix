/**
 * grremixhazards.c - dispatch of the Remix hazard setups. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <stdint.h>
#include <sys/objtypes.h>

sb32 grRemixHazardsAreOn(void)
{
    return TRUE;
}

sb32 grRemixMovementIsOn(void)
{
    return TRUE;
}

sb32 grRemixHazardsMatchHasStarted(void)
{
    if (gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode)
    {
        return TRUE;
    }

    return gSCManagerBattleState->time_passed != 0;
}

#define GRREMIX_HAZARDS_ITEM_POINTER_OFFSET 0xE0
#define GRREMIX_HAZARDS_ITEM_FILE_BIAS 0x07E8

void* grRemixHazardsItemFileTop(void *stage_header)
{
    u32 item_token = *(u32*) ((uintptr_t) stage_header + GRREMIX_HAZARDS_ITEM_POINTER_OFFSET);

    return (void*) ((uintptr_t) PORT_RESOLVE(item_token) - GRREMIX_HAZARDS_ITEM_FILE_BIAS);
}

sb32 grRemixHazardsMakeGround(s32 gkind, u32 function)
{
    return grRemixHazardsMakeGroundBayDraculaTwilight(gkind, function) ||
           grRemixHazardJapes(gkind, function) ||
           grRemixHazardCorneria(gkind, function) ||
           grRemixHazardGbLand(gkind, function) ||
           grRemixHazardPirate(gkind, function) ||
           grRemixHazardCasino(gkind, function) ||
           grRemixHazardMetallic(gkind, function) ||
           grRemixHazardOnett(gkind, function) ||
           grRemixHazardTurnpike(gkind, function) ||
           grRemixHazardRainbow(gkind, function) ||
           grRemixHazardMmm(gkind, function) ||
           grRemixHazardBbh(gkind, function) ||
           grRemixHazardSpawnedFear(gkind, function) ||
           grRemixHazardRttf(gkind, function) ||
           grRemixHazardTimeTwister(gkind, function) ||
           grRemixHazardScuttleTown(gkind, function) ||
           grRemixHazardPeachCastle2(gkind, function);
}

#endif
