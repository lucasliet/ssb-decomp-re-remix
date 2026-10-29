#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaicommandsrules.h"

static int ftRemixAiRulesSideSign(int is_positive)
{
    return is_positive ? 1 : -1;
}

int ftRemixAiRulesStickXFromCode(int code, float player_x, float target_x, int facing_lr)
{
    int away_sign = ftRemixAiRulesSideSign(player_x >= target_x);
    int facing_sign = ftRemixAiRulesSideSign(facing_lr >= 0);

    switch (code)
    {
    case FTREMIX_AI_STICK_AUTOFULL_AWAY:
        return away_sign * FTREMIX_AI_STICK_FULL;

    case FTREMIX_AI_STICK_AUTOHALF_AWAY:
        return away_sign * FTREMIX_AI_STICK_HALF;

    case FTREMIX_AI_STICK_FORWARDS:
        return facing_sign * FTREMIX_AI_STICK_FULL;

    case FTREMIX_AI_STICK_BACKWARDS:
        return -facing_sign * FTREMIX_AI_STICK_FULL;

    default:
        return 0;
    }
}

int ftRemixAiRulesIsTurnAction(int status_id)
{
    return (status_id == FTREMIX_AI_ACTION_TURN) || (status_id == FTREMIX_AI_ACTION_TURN_RUN);
}

#endif
