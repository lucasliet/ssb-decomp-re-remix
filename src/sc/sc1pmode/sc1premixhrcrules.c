/**
 * sc1premixhrcrules.c - state-free rules of the Home-Run Contest, see sc1premixhrcrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixhrcrules.h"

#define SC1PREMIX_HRC_CAMERA_PAN_DIVISOR 3.0F
#define SC1PREMIX_HRC_COUNTER_SINK_PER_TIC 0.5F
#define SC1PREMIX_HRC_COUNTER_GROWTH_PER_TIC 0.01F
#define SC1PREMIX_HRC_COUNTER_HALF_GROWN 0.725F
#define SC1PREMIX_HRC_COUNTER_TAIL_HALF 12.5F

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

void sc1PRemixHrcCounterAt(int complete_tics, int is_completing, float number_width, SC1PRemixHrcCounterLayout *layout)
{
    float final_right_edge;
    float step;
    int steps;

    layout->counter_x = SC1PREMIX_HRC_COUNTER_X;
    layout->counter_y = SC1PREMIX_HRC_COUNTER_Y;
    layout->counter_scale = SC1PREMIX_HRC_COUNTER_SCALE;
    layout->label_x = SC1PREMIX_HRC_LABEL_X;
    layout->label_y = SC1PREMIX_HRC_LABEL_Y;
    layout->label_scale = SC1PREMIX_HRC_LABEL_SCALE;

    if (is_completing == 0)
    {
        return;
    }
    if (complete_tics > SC1PREMIX_HRC_COMPLETE_TIMER_START)
    {
        complete_tics = SC1PREMIX_HRC_COMPLETE_TIMER_START;
    }
    if (complete_tics < 0)
    {
        complete_tics = 0;
    }
    steps = complete_tics + 1;
    final_right_edge = SC1PREMIX_HRC_COUNTER_CENTER_X + (SC1PREMIX_HRC_COUNTER_HALF_GROWN * number_width) - SC1PREMIX_HRC_COUNTER_TAIL_HALF;
    step = (SC1PREMIX_HRC_COUNTER_X - final_right_edge) / (float)(SC1PREMIX_HRC_COMPLETE_TIMER_START + 1);

    layout->counter_x -= step * steps;
    layout->label_x -= step * steps;
    layout->counter_y += SC1PREMIX_HRC_COUNTER_SINK_PER_TIC * steps;
    layout->label_y += SC1PREMIX_HRC_COUNTER_SINK_PER_TIC * steps;
    layout->counter_scale += SC1PREMIX_HRC_COUNTER_GROWTH_PER_TIC * steps;
    layout->label_scale += SC1PREMIX_HRC_COUNTER_GROWTH_PER_TIC * steps;
}

float sc1PRemixHrcCloudSpeed(float sandbag_dx, int is_past_start)
{
    if (is_past_start == 0)
    {
        return 0.0F;
    }
    return sandbag_dx * SC1PREMIX_HRC_CLOUD_SPEED_FACTOR;
}

float sc1PRemixHrcCloudStep(float x, float speed)
{
    x += speed;

    if (x <= SC1PREMIX_HRC_CLOUD_WARP_THRESHOLD)
    {
        x += SC1PREMIX_HRC_CLOUD_WARP_DISTANCE;
    }
    return x;
}

int sc1PRemixHrcRecordVerdict(unsigned int previous, unsigned int distance)
{
    if (previous == 0)
    {
        return (distance != 0) ? SC1PREMIX_HRC_RECORD_FIRST : SC1PREMIX_HRC_RECORD_KEEP;
    }
    return (distance > previous) ? SC1PREMIX_HRC_RECORD_BEATEN : SC1PREMIX_HRC_RECORD_KEEP;
}

#endif
