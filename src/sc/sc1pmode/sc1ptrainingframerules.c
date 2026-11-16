/**
 * sc1ptrainingframerules.c - pure rules of the Training D-pad shortcuts, see sc1ptrainingframerules.h.
 */

#include "sc/sc1pmode/sc1ptrainingframerules.h"

void sc1PTrainingFrameRulesInit(SC1PTrainingFrameState *state)
{
    state->is_frozen = 0;
    state->is_up_latched = 0;
    state->is_right_latched = 0;
}

int32_t sc1PTrainingFrameRulesReadDpad(SC1PTrainingFrameState *state, int32_t dpad_option, int32_t is_left_tap,
                                       int32_t is_right_repeat, int32_t is_up_tap, int32_t is_down_tap)
{
    if ((dpad_option != SC1PTRAININGFRAMERULES_DPAD_OFF) && (is_left_tap != 0))
    {
        return SC1PTRAININGFRAMERULES_ACTION_RESET;
    }
    if ((state->is_up_latched != 0) || (state->is_right_latched != 0))
    {
        return 0;
    }
    if (dpad_option != SC1PTRAININGFRAMERULES_DPAD_ON)
    {
        state->is_frozen = 0;
        return 0;
    }
    state->is_right_latched = (is_right_repeat != 0) ? 1 : 0;

    if (state->is_right_latched != 0)
    {
        return 0;
    }
    state->is_up_latched = (is_up_tap != 0) ? 1 : 0;

    return (is_down_tap != 0) ? SC1PTRAININGFRAMERULES_ACTION_MODEL_DISPLAY : 0;
}

SC1PTrainingFrameKind sc1PTrainingFrameRulesResolve(SC1PTrainingFrameState *state, int32_t is_lag_skip)
{
    SC1PTrainingFrameKind kind;

    if (is_lag_skip != 0)
    {
        return nSC1PTrainingFrameKindSkip;
    }
    if (state->is_up_latched != 0)
    {
        state->is_frozen = (state->is_frozen == 0) ? 1 : 0;
    }
    if (state->is_right_latched != 0)
    {
        state->is_frozen = 1;
        kind = nSC1PTrainingFrameKindAdvance;
    }
    else kind = (state->is_frozen != 0) ? nSC1PTrainingFrameKindFrozen : nSC1PTrainingFrameKindRun;

    state->is_up_latched = 0;
    state->is_right_latched = 0;

    return kind;
}
