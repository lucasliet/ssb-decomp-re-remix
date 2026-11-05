/**
 * sc1ptrainingleftoverrules.c - pure rules of the Remix Training leftovers, see sc1ptrainingleftoverrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1ptrainingleftoverrules.h"

int32_t sc1PTrainingLeftoverRulesNextFrameCount(int32_t prev_action, int32_t action, int32_t prev_hitlag, int32_t hitlag,
                                                int32_t frame_count)
{
    if (prev_action != action)
    {
        return 1;
    }
    if ((hitlag == 0) || (prev_hitlag == 0))
    {
        return frame_count + 1;
    }
    return frame_count;
}

int32_t sc1PTrainingLeftoverRulesIsSharedAction(int32_t action)
{
    return (action < SC1PTRAININGLEFTOVERRULES_SHARED_ACTION_COUNT) ? 1 : 0;
}

int32_t sc1PTrainingLeftoverRulesGetUniqueIndex(int32_t action)
{
    return action - SC1PTRAININGLEFTOVERRULES_SHARED_ACTION_COUNT;
}

int32_t sc1PTrainingLeftoverRulesHoldExitStep(int32_t held_frames, int32_t is_a_held, int32_t *next_held_frames)
{
    int32_t frames;

    if (is_a_held == 0)
    {
        *next_held_frames = 0;
        return 0;
    }
    frames = held_frames + 1;

    if (frames >= SC1PTRAININGLEFTOVERRULES_HOLD_EXIT_FRAMES)
    {
        *next_held_frames = 0;
        return 1;
    }
    *next_held_frames = frames;
    return 0;
}

int32_t sc1PTrainingLeftoverRulesGetHoldBarWidth(int32_t width, int32_t is_a_held)
{
    if (is_a_held == 0)
    {
        return 0;
    }
    if (width >= SC1PTRAININGLEFTOVERRULES_HOLD_BAR_MAX_WIDTH)
    {
        return width;
    }
    return width + SC1PTRAININGLEFTOVERRULES_HOLD_BAR_STEP;
}

int32_t sc1PTrainingLeftoverRulesGetHeldItemEntry(int32_t kind, int32_t first_kind, int32_t count)
{
    if ((kind < first_kind) || (kind >= (first_kind + count)))
    {
        return -1;
    }
    return kind - first_kind;
}

#endif
