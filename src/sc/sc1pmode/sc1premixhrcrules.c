/**
 * sc1premixhrcrules.c - state-free rules of the Home-Run Contest, see sc1premixhrcrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixhrcrules.h"

#define SC1PREMIX_HRC_CAMERA_PAN_DIVISOR 3.0F

void sc1PRemixHrcStateReset(SC1PRemixHrcState *state)
{
    state->warp_distance = 0.0F;
    state->distance = 0;
    state->complete_timer = SC1PREMIX_HRC_COMPLETE_TIMER_START;
}

static void sc1PRemixHrcTrackDistance(SC1PRemixHrcState *state, float sandbag_x)
{
    float units = (sandbag_x - SC1PREMIX_HRC_START_X) + state->warp_distance;

    state->distance = (int)(units / SC1PREMIX_HRC_UNITS_PER_FOOT_TENTH);
}

void sc1PRemixHrcTrack(SC1PRemixHrcState *state, float sandbag_x, float camera_x, SC1PRemixHrcMove *move)
{
    float panned_x;

    move->sandbag_shift = 0.0F;
    move->is_past_start = (sandbag_x >= SC1PREMIX_HRC_START_X) ? 1 : 0;

    if (move->is_past_start == 0)
    {
        move->camera_x = SC1PREMIX_HRC_DEFAULT_CAMERA_X;
        return;
    }
    sc1PRemixHrcTrackDistance(state, sandbag_x);

    panned_x = camera_x + ((sandbag_x - camera_x) / SC1PREMIX_HRC_CAMERA_PAN_DIVISOR);

    if (panned_x < SC1PREMIX_HRC_CAMERA_WARP_THRESHOLD)
    {
        move->camera_x = panned_x;
        return;
    }
    state->warp_distance -= SC1PREMIX_HRC_CAMERA_WARP_DISTANCE;
    move->sandbag_shift = SC1PREMIX_HRC_CAMERA_WARP_DISTANCE;
    move->camera_x = panned_x + SC1PREMIX_HRC_CAMERA_WARP_DISTANCE;
}

static int sc1PRemixHrcInputsVerdict(int has_time)
{
    return (has_time != 0) ? SC1PREMIX_HRC_VERDICT_KEEP_GOING : SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS;
}

int sc1PRemixHrcAttemptVerdict(const SC1PRemixHrcAttempt *attempt)
{
    int has_time = (attempt->time_passed < SC1PREMIX_HRC_TIME_LIMIT_TICS) ? 1 : 0;

    if ((has_time != 0) && (attempt->is_past_start == 0) && (attempt->is_over_platform != 0))
    {
        return SC1PREMIX_HRC_VERDICT_KEEP_GOING;
    }
    if ((has_time == 0) && (attempt->is_human_stuck_in_hold != 0))
    {
        return SC1PREMIX_HRC_VERDICT_END;
    }
    if (attempt->is_sandbag_aerial != 0)
    {
        return sc1PRemixHrcInputsVerdict(has_time);
    }
    if (attempt->is_sandbag_moving != 0)
    {
        return SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS;
    }
    if ((has_time != 0) || (attempt->is_past_start != 0) || (attempt->is_human_resting != 0))
    {
        return SC1PREMIX_HRC_VERDICT_END;
    }
    return SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS;
}

#endif
