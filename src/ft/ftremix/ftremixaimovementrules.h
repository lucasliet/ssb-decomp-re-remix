#ifndef _FTREMIXAIMOVEMENTRULES_H_
#define _FTREMIXAIMOVEMENTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure movement rules of the level 10 Remix AI (AI.asm fastfall, shorthop_followup_attack, jump_in_neutral,
 * clamp_target_pos_to_stage, detect_offstage_opponents), so the headless unit tests (tests/remix) can run them
 * without game state. Numbers in, numbers out. */

#define FTREMIX_AI_MOVEMENT_LEVEL_10 10

#define FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK 0x01
#define FTREMIX_AI_MOVEMENT_INPUT_FULL_HOP 0x04
#define FTREMIX_AI_MOVEMENT_INPUT_FASTFALL 0x27
#define FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS 0x31
#define FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_IN_PLACE 0x33
#define FTREMIX_AI_MOVEMENT_INPUT_FULL_C_JUMP_TOWARDS 0x34

#define FTREMIX_AI_MOVEMENT_FASTFALL_MIN_FLOOR_DIST -2000.0F
#define FTREMIX_AI_MOVEMENT_LEDGE_PADDING 300.0F
#define FTREMIX_AI_MOVEMENT_JUMP_PADDING 200.0F
#define FTREMIX_AI_MOVEMENT_JUMP_IN_NEUTRAL_ROLL_MAX 200

#define FTREMIX_AI_MOVEMENT_SHORTHOP_VERY_LOW 300.0F
#define FTREMIX_AI_MOVEMENT_SHORTHOP_LOW 600.0F
#define FTREMIX_AI_MOVEMENT_SHORTHOP_MED 900.0F

/**
 * TRUE when a level 10 CPU may press the fastfall input. FALSE when all jumps are used, when the opponent is offstage
 * (floor_line_id -1), or when the CPU is not more than 2000 units above the ground (floor_dist is negative above ground).
 * With no opponent (has_target 0) it is TRUE whatever the floor distance.
 */
extern int ftRemixAiMovementRulesFastfallAllowed(int jumps_used, int jumps_max, int has_target, int target_floor_line_id,
                                                 float floor_dist);

/**
 * Input for a grounded CPU whose target is above. diff is target_y + target_vel_y - my_y. Gaps up to 300 walk, up to
 * 600 short hop towards, up to 900 full C jump towards, larger gaps keep vanilla_input (the full hop).
 */
extern int ftRemixAiMovementRulesShorthopInput(float diff, int vanilla_input);

/**
 * Maps a roll in 0..199 to the neutral input: 0..2 short hop towards, 3 short hop in place, 4..5 full C jump towards.
 * Any other roll returns the point stick input, which means no jump.
 */
extern int ftRemixAiMovementRulesJumpInNeutralInput(int roll);

/**
 * Keeps a target X inside the stage, FTREMIX_AI_MOVEMENT_LEDGE_PADDING away from each edge: a target at or beyond
 * right_edge - padding becomes that value, then a target at or below left_edge + padding becomes that value (the left
 * clamp runs last, so it wins on a stage narrower than twice the padding). Targets between the limits are unchanged.
 */
extern float ftRemixAiMovementRulesClampTargetX(float target_x, float left_edge, float right_edge);

/**
 * Lowers the target Y by FTREMIX_AI_MOVEMENT_JUMP_PADDING and clamps it to 0 when the result is not above 0.
 */
extern float ftRemixAiMovementRulesClampTargetY(float target_y);

/** TRUE for level 10: the CPU may target an opponent who is offstage (detect_offstage_opponents). */
extern int ftRemixAiMovementRulesOffstageTargetAllowed(int level);

#endif

#endif
