/**
 * grremixhazards.c - dispatch of the Remix hazard setups. See grremixhazards.h.
 */

#ifdef PORT

#include "grremixhazards.h"

sb32 grRemixHazardsAreOn(void)
{
    return TRUE;
}

sb32 grRemixMovementIsOn(void)
{
    return TRUE;
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
           grRemixHazardRttf(gkind, function);
}

#endif
