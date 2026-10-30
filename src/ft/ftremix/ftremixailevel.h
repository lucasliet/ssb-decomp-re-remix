#ifndef _FTREMIXAILEVEL_H_
#define _FTREMIXAILEVEL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Level clamp hooks of the Remix AI (AI.asm clamp_level_9_1 .. clamp_level_9_9). */

/** CPU level as the Remix AI reads it: level 10 is seen as 9 in the formulas that would go negative. */
s32 ftRemixAiClampedLevel(const FTStruct *fp);

/** Walk-stop timer threshold for the CPU (AI.asm clamp_level_9_9). */
s32 ftRemixAiWalkStopThreshold(const FTStruct *fp);

/** Collision size base for the near-opponent check (AI.asm clamp_level_9_7). */
f32 ftRemixAiNearCollisionScale(const FTStruct *fp);

#endif

#endif
