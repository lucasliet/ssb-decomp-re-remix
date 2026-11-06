#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <math.h>
#include "ft/ftremix/ftremixairecoverydriftrules.h"

int ftRemixAiDriftGateOpens(int level_ten, int is_fall_special, int jumps_used, int jumps_max, int objective_is_recover,
                            int floor_line_id)
{
    if (level_ten == 0)
    {
        return 0;
    }
    if ((is_fall_special != 0) || (jumps_used >= jumps_max))
    {
        return 1;
    }
    return (objective_is_recover != 0) && (floor_line_id == -1);
}

int ftRemixAiDriftEdgeIsOnScreen(float edge_x, float edge_y, float top, float bottom, float right, float left)
{
    if (edge_y >= (top - FTREMIX_AI_DRIFT_BLASTZONE_PADDING))
    {
        return 0;
    }
    if (edge_y <= (bottom + FTREMIX_AI_DRIFT_BLASTZONE_PADDING))
    {
        return 0;
    }
    return ((left + FTREMIX_AI_DRIFT_BLASTZONE_PADDING) <= edge_x) && (edge_x <= (right - FTREMIX_AI_DRIFT_BLASTZONE_PADDING));
}

float ftRemixAiDriftLedgeDistance(float own_x, float own_y, float edge_x, float edge_y)
{
    float x_distance = own_x - edge_x;
    float y_distance = own_y - edge_y;

    if (own_y < edge_y)
    {
        y_distance *= FTREMIX_AI_DRIFT_BELOW_DISTANCE_SCALE;
    }
    return sqrtf((x_distance * x_distance) + (y_distance * y_distance));
}

/* Inside the stage bounds the drift is skipped for a fighter well above both ledges, unless it uses its up special or is in
 * special fall. Outside the bounds it always runs. */
static int ftRemixAiDriftSkipsInnerGap(const FTRemixAiDriftState *state)
{
    if ((state->x < state->bounds_left) || (state->bounds_right < state->x))
    {
        return 0;
    }
    if ((state->special_flag_set != 0) || (state->is_fall_special != 0))
    {
        return 0;
    }
    return ((state->left_y + FTREMIX_AI_DRIFT_TOO_HIGH_Y) < state->y) ||
           ((state->right_y + FTREMIX_AI_DRIFT_TOO_HIGH_Y) < state->y);
}

/* Below the ledge. With the up special in use it only points toward the ledge side; otherwise a fighter facing a grabbable
 * ledge aims at its grab box, and every other case aims outside the ledge so the fighter is not pineappled. */
static void ftRemixAiDriftBelowLedge(const FTRemixAiDriftState *state, int special_flag_set, int is_facing_ledge, int grabbable,
                                     float scale, FTRemixAiDriftResult *result)
{
    if (special_flag_set != 0)
    {
        if (is_facing_ledge == 0)
        {
            result->target_x = state->x - (FTREMIX_AI_DRIFT_USP_AWAY_X * scale);
        }
        return;
    }
    if ((grabbable != 0) && (state->is_slow_jumper == 0) && (is_facing_ledge != 0))
    {
        result->target_y -= state->grab_y;
        result->target_x += state->grab_x * scale;
        return;
    }
    result->target_x += FTREMIX_AI_DRIFT_LEDGE_PADDING * scale;
}

/* Above the ledge. The up special and every case that is not facing a grabbable ledge aim inside the ledge; the facing case
 * lowers the target by the grab box. */
static void ftRemixAiDriftAboveLedge(const FTRemixAiDriftState *state, int special_flag_set, int is_facing_ledge, int grabbable,
                                     float scale, FTRemixAiDriftResult *result)
{
    if ((special_flag_set == 0) && (grabbable != 0) && (is_facing_ledge != 0))
    {
        result->target_y -= state->grab_y;
        return;
    }
    result->target_x -= FTREMIX_AI_DRIFT_LEDGE_PADDING * scale;
}

void ftRemixAiDriftComputeTarget(const FTRemixAiDriftState *state, FTRemixAiDriftResult *result)
{
    float biased_x;
    int goes_right;
    int ledge_direction;
    int grabbable;
    int is_facing_ledge;
    int special_flag_set = state->special_flag_set;
    float scale;
    float probe_y = state->y;

    result->skipped = 0;
    result->clears_special_flag = 0;

    if (ftRemixAiDriftSkipsInnerGap(state) != 0)
    {
        result->skipped = 1;
        return;
    }
    biased_x = state->x + (FTREMIX_AI_DRIFT_FACING_BIAS * (float)state->lr);
    goes_right = fabsf(biased_x - state->right_x) < fabsf(biased_x - state->left_x);

    /* "Left" is the left edge of a floor line, which a fighter off its left side faces with lr 1. */
    scale = goes_right ? 1.0F : -1.0F;
    ledge_direction = goes_right ? -1 : 1;
    grabbable = goes_right ? state->right_grabbable : state->left_grabbable;
    result->target_x = goes_right ? state->right_x : state->left_x;
    result->target_y = goes_right ? state->right_y : state->left_y;

    if (state->resets_special_flag != 0)
    {
        special_flag_set = 0;
        result->clears_special_flag = 1;
    }
    is_facing_ledge = state->lr == ledge_direction;

    if ((is_facing_ledge != 0) && (grabbable != 0))
    {
        probe_y += state->grab_y;
    }
    if (result->target_y < probe_y)
    {
        ftRemixAiDriftAboveLedge(state, special_flag_set, is_facing_ledge, grabbable, scale, result);
        return;
    }
    ftRemixAiDriftBelowLedge(state, special_flag_set, is_facing_ledge, grabbable, scale, result);
}

#endif
