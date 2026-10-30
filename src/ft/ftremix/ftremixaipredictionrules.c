#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaipredictionrules.h"

/** Level 10 widens the attack view range; the other levels keep the vanilla base. */
float ftRemixAiPredictionRulesAttackViewBase(int level)
{
    if (level == FTREMIX_AI_PREDICTION_LEVEL_10)
    {
        return FTREMIX_AI_PREDICTION_LV10_VIEW_BASE;
    }
    return FTREMIX_AI_PREDICTION_VANILLA_VIEW_BASE;
}

/** Input filter of AI.asm fix_movement_prediction (check_skip_move). */
int ftRemixAiPredictionRulesRunningCandidateAllowed(int is_running, int input_kind)
{
    if (!is_running)
    {
        return 1;
    }
    return (input_kind == FTREMIX_AI_PREDICTION_INPUT_GRAB) || (input_kind == FTREMIX_AI_PREDICTION_INPUT_USMASH) ||
           (input_kind == FTREMIX_AI_PREDICTION_INPUT_USPG) || (input_kind == FTREMIX_AI_PREDICTION_INPUT_NSPG) ||
           (input_kind == FTREMIX_AI_PREDICTION_INPUT_DASH_ATTACK);
}

/** One frame of ground friction: the speed moves towards zero and is not allowed to cross it. */
static float ftRemixAiPredictionRulesFrictionStep(float vel_x, float friction)
{
    if (vel_x > 0.0F)
    {
        return vel_x - friction;
    }
    return vel_x + friction;
}

/** TRUE when the speed has reached or crossed zero, which ends the friction loop. */
static int ftRemixAiPredictionRulesSpeedStopped(float vel_x)
{
    return vel_x <= 0.0F;
}

float ftRemixAiPredictionRulesMyX(float pos_x, float vel_x, float friction, int hit_frame)
{
    int frame;

    if (vel_x == 0.0F)
    {
        return pos_x;
    }
    for (frame = 0; frame < hit_frame; frame++)
    {
        pos_x += vel_x;

        if (vel_x > 0.0F)
        {
            vel_x = ftRemixAiPredictionRulesFrictionStep(vel_x, friction);
            if (ftRemixAiPredictionRulesSpeedStopped(vel_x))
            {
                break;
            }
            continue;
        }
        vel_x = ftRemixAiPredictionRulesFrictionStep(vel_x, friction);
        if (vel_x >= 0.0F)
        {
            break;
        }
    }
    return pos_x;
}

float ftRemixAiPredictionRulesDeltaX(float my_pred_x, float target_x, float target_vel_x, int hit_frame)
{
    float target_pred_x = target_x + (target_vel_x * hit_frame);

    return target_pred_x - my_pred_x;
}

float ftRemixAiPredictionRulesFallY(float pos_y, float vel_y, float gravity, float max_fall, float floor_dist,
                                    int has_floor, int hit_frame)
{
    float ground_y = pos_y + floor_dist;
    int frame;

    for (frame = 0; frame < hit_frame; frame++)
    {
        pos_y += vel_y;
        floor_dist -= vel_y;

        if (has_floor && (floor_dist >= 0.0F))
        {
            return ground_y;
        }
        vel_y -= gravity;
        if (vel_y <= max_fall)
        {
            vel_y = max_fall;
        }
    }
    return pos_y;
}

#endif
