/**
 * ftremixsizeweight.c - Smash Remix body size subsystem, phase (b): fighter weight (Size.asm adjust_player_weight_1..8).
 *
 * The knockback formulas read the attribute weight of the fighter. Remix scales that weight by the size multiplier
 * of the fighter, so a large fighter is knocked back less. The multiplier is 1.0F by default, so the weight is
 * unchanged until a source sets it.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>

/**
 * Returns vanilla_weight scaled by the size of fp: vanilla_weight * (1 + FTREMIX_SIZE_COEF_WEIGHT * (size - 1)).
 * Callers pass fp->attr->weight, the value the vanilla knockback formula reads.
 */
f32 ftRemixSizeWeight(FTStruct *fp, f32 vanilla_weight)
{
    return vanilla_weight * ftRemixSizeAttrMultiplier(fp->player, FTREMIX_SIZE_COEF_WEIGHT);
}

#endif
