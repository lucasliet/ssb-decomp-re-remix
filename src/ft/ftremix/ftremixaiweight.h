#ifndef _FTREMIXAIWEIGHT_H_
#define _FTREMIXAIWEIGHT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Attack option weighting of the Remix lv10 CPU (AI.asm weight_attack_options), called from ftComputerCheckDetectTarget. */

/**
 * TRUE when the lv10 weighting replaces the vanilla range of each candidate attack (AI.asm `lbu 0x13`, level 10 only).
 */
sb32 ftRemixAiWeightAttackOptionsApplies(FTStruct *fp);

/**
 * Chance of one candidate attack of this_fp against target_fp (AI.asm weight_attack_options). Starts at 1.0 and is
 * scaled by the own landing, the opponent hitstun or combo, the slow move and the grab against shield rules. It
 * consumes the same random numbers, in the same order, as the asm.
 */
f32 ftRemixAiWeightAttackOption(FTStruct *this_fp, FTStruct *target_fp, FTComputerAttack *comattack);

#endif

#endif
