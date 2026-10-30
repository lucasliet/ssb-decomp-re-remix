#ifndef _FTREMIXAIPREDICTIONRULES_H_
#define _FTREMIXAIPREDICTIONRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure attack-range and movement-prediction rules of the level 10 Remix AI (AI.asm extend_attack_check_range,
 * extend_attack_check_range_2, fix_movement_prediction), so the headless unit tests (tests/remix) can run them without
 * game state. Numbers in, numbers out. */

#define FTREMIX_AI_PREDICTION_LEVEL_10 10

#define FTREMIX_AI_PREDICTION_VANILLA_VIEW_BASE 1200.0F
#define FTREMIX_AI_PREDICTION_LV10_VIEW_BASE 2000.0F

#define FTREMIX_AI_PREDICTION_INPUT_NSPG 0x09
#define FTREMIX_AI_PREDICTION_INPUT_USPG 0x0D
#define FTREMIX_AI_PREDICTION_INPUT_USMASH 0x19
#define FTREMIX_AI_PREDICTION_INPUT_GRAB 0x1C
#define FTREMIX_AI_PREDICTION_INPUT_DASH_ATTACK 0x47

/**
 * Base of the attack view range used as random * 300 + base. Level 10 uses 2000, other levels keep the vanilla 1200.
 */
extern float ftRemixAiPredictionRulesAttackViewBase(int level);

/**
 * TRUE when a candidate attack may be considered. While the CPU is running (Dash, Run, RunBrake) only grab, upsmash,
 * up special ground, neutral special ground and dash attack are allowed. Outside a run every candidate is allowed.
 */
extern int ftRemixAiPredictionRulesRunningCandidateAllowed(int is_running, int input_kind);

/**
 * CPU X after hit_frame frames, applying ground friction to its speed until the speed crosses zero. A speed of zero
 * keeps pos_x. friction is 0 while airborne.
 */
extern float ftRemixAiPredictionRulesMyX(float pos_x, float vel_x, float friction, int hit_frame);

/** Target X minus CPU X at hit_frame, using the target speed as is. The caller applies the facing direction. */
extern float ftRemixAiPredictionRulesDeltaX(float my_pred_x, float target_x, float target_vel_x, int hit_frame);

/**
 * Y after hit_frame frames of gravity, capped at max_fall (a negative speed), snapped to the ground when the distance
 * to the floor becomes non-negative and the body has a floor line (has_floor). Only used while airborne.
 */
extern float ftRemixAiPredictionRulesFallY(float pos_y, float vel_y, float gravity, float max_fall, float floor_dist,
                                           int has_floor, int hit_frame);

#endif

#endif
