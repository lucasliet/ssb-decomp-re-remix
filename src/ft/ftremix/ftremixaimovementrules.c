#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaimovementrules.h"

/**
 * Fastfall gate of AI.asm prevent_fastfall: all jumps used, an offstage opponent, or a CPU within 2000 units of the
 * ground (the asm compares floor_dist < -2000 and skips when it is not) deny the fastfall. With no opponent the asm jumps
 * to the fastfall at once (`beqz t1, _original`), before the floor test.
 */
int ftRemixAiMovementRulesFastfallAllowed(int jumps_used, int jumps_max, int has_target, int target_floor_line_id,
                                          float floor_dist)
{
    if (jumps_used == jumps_max)
    {
        return 0;
    }
    if (has_target == 0)
    {
        return 1;
    }
    if (target_floor_line_id == -1)
    {
        return 0;
    }
    return floor_dist < FTREMIX_AI_MOVEMENT_FASTFALL_MIN_FLOOR_DIST;
}

/** Shorthop choice of AI.asm shorthop_followup_attack. The thresholds are inclusive, as the asm compares c.le.s. */
int ftRemixAiMovementRulesShorthopInput(float diff, int vanilla_input)
{
    if (diff <= FTREMIX_AI_MOVEMENT_SHORTHOP_VERY_LOW)
    {
        return FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK;
    }
    if (diff <= FTREMIX_AI_MOVEMENT_SHORTHOP_LOW)
    {
        return FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS;
    }
    if (diff <= FTREMIX_AI_MOVEMENT_SHORTHOP_MED)
    {
        return FTREMIX_AI_MOVEMENT_INPUT_FULL_C_JUMP_TOWARDS;
    }
    return vanilla_input;
}

/**
 * Neutral jump table of AI.asm jump_in_neutral. The asm sets the input in the beq delay slots, so the last matching
 * roll wins; the table below keeps that mapping without relying on delay slots.
 */
int ftRemixAiMovementRulesJumpInNeutralInput(int roll)
{
    if (roll <= 2)
    {
        return FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS;
    }
    if (roll == 3)
    {
        return FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_IN_PLACE;
    }
    if ((roll == 4) || (roll == 5))
    {
        return FTREMIX_AI_MOVEMENT_INPUT_FULL_C_JUMP_TOWARDS;
    }
    return FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK;
}

/**
 * Stage clamp of AI.asm clamp_target_pos_to_stage (AI.asm 4903-4960). The asm reads 0x28(s3) and 0x2C(s3) of
 * gMPCollisionBounds, which are current.right and current.left (the vanilla code at 0x80132A20 compares the fighter X with
 * the same words as right and left), although its comments call them left and right. Each `c.le.s` plus `bc1fl` applies the
 * clamp when the compare is true, so the target is pulled inside [left + padding, right - padding]: first the right side,
 * then the left side.
 */
float ftRemixAiMovementRulesClampTargetX(float target_x, float left_edge, float right_edge)
{
    float right_limit = right_edge - FTREMIX_AI_MOVEMENT_LEDGE_PADDING;
    float left_limit = left_edge + FTREMIX_AI_MOVEMENT_LEDGE_PADDING;

    if (right_limit <= target_x)
    {
        target_x = right_limit;
    }
    if (target_x <= left_limit)
    {
        target_x = left_limit;
    }
    return target_x;
}

/** Y clamp of AI.asm clamp_target_pos_to_stage: the padded target Y is replaced by 0 when it is not above 0. */
float ftRemixAiMovementRulesClampTargetY(float target_y)
{
    float padded_y = target_y - FTREMIX_AI_MOVEMENT_JUMP_PADDING;

    if (padded_y <= 0.0F)
    {
        return 0.0F;
    }
    return padded_y;
}

/** detect_offstage_opponents only applies to level 10. */
int ftRemixAiMovementRulesOffstageTargetAllowed(int level)
{
    return level == FTREMIX_AI_MOVEMENT_LEVEL_10;
}

#endif
