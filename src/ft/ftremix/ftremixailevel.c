/*
 * ftremixailevel.c - level clamp hooks of the Remix AI (AI.asm clamp_level_9_1 .. clamp_level_9_9). Pure rules live in
 * ftremixailevelrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftremix/ftremixailevel.h"
#include "ft/ftremix/ftremixailevelrules.h"

/** Level read by the CPU formulas that would go negative at level 10 (AI.asm clamp_level_9_1 to _6 and _8). */
s32 ftRemixAiClampedLevel(const FTStruct *fp)
{
    return ftRemixAiLevelClamp(fp->level);
}

/** Walk-stop threshold of the CPU (AI.asm clamp_level_9_9). */
s32 ftRemixAiWalkStopThreshold(const FTStruct *fp)
{
    return ftRemixAiLevelWalkStopThreshold(fp->level);
}

/** Near-opponent collision size base (AI.asm clamp_level_9_7). */
f32 ftRemixAiNearCollisionScale(const FTStruct *fp)
{
    return ftRemixAiLevelCollisionScale(fp->level);
}

#endif
