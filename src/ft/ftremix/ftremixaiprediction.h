#ifndef _FTREMIXAIPREDICTION_H_
#define _FTREMIXAIPREDICTION_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Level 10 attack range and movement prediction hooks of the Remix AI (AI.asm extend_attack_check_range,
 * extend_attack_check_range_2, fix_movement_prediction). */

/** extend_attack_check_range: base of the attack view range (random * 300 + base). 2000 for level 10, else 1200. */
f32 ftRemixAiPredictionAttackViewBase(FTStruct *fp);

/** fix_movement_prediction: TRUE when the candidate input may be considered (running filter, level 10 only). */
sb32 ftRemixAiPredictionCandidateAllowed(FTStruct *fp, s32 input_kind);

/**
 * fix_movement_prediction: writes the predicted X and Y offsets of the candidate (target minus CPU at hit_frame).
 * Only writes for level 10; other levels keep the vanilla values in the outputs.
 */
void ftRemixAiPredictAttackPosition(FTStruct *cpu, FTStruct *target, s32 hit_frame, f32 *predict_x, f32 *predict_y);

#endif

#endif
