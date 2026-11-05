#if defined(PORT) && defined(REGION_REMIX)

#include <math.h>
#include "ft/ftremix/ftremixaisharedrules.h"

int ftRemixAiSharedIsMarinaCargoStatus(int status_id)
{
    return (status_id >= FTREMIX_AI_SHARED_MARINA_CARGO_FIRST) && (status_id <= FTREMIX_AI_SHARED_MARINA_CARGO_LAST);
}

/** The asm loads the hat with lb and compares with sltiu, so a negative hat id counts as a Dedede hat. */
static int ftRemixAiSharedHatIsDedede(int hat_id)
{
    return (unsigned int)hat_id >= FTREMIX_AI_SHARED_DEDEDE_HAT_MIN;
}

static int ftRemixAiSharedIsDededeInhaled(int status_id)
{
    return (status_id == FTREMIX_AI_SHARED_DEDEDE_NSP_IDLE_GROUND) || (status_id == FTREMIX_AI_SHARED_DEDEDE_NSP_FALL);
}

static int ftRemixAiSharedIsKirbyDededeHatInhaled(int status_id)
{
    return (status_id == FTREMIX_AI_SHARED_KIRBY_DEDEDE_NSP_IDLE_GROUND) ||
           (status_id == FTREMIX_AI_SHARED_KIRBY_DEDEDE_NSP_FALL);
}

int ftRemixAiSharedInhaleOutcome(int is_kirby_kind, int is_dedede, int hat_id, int status_id)
{
    if (is_dedede)
    {
        return ftRemixAiSharedIsDededeInhaled(status_id) ? FTREMIX_AI_SHARED_INHALE_PRESS_NSP : FTREMIX_AI_SHARED_INHALE_SKIP;
    }
    if (is_kirby_kind == 0)
    {
        return FTREMIX_AI_SHARED_INHALE_SKIP;
    }
    if (ftRemixAiSharedHatIsDedede(hat_id) == 0)
    {
        return FTREMIX_AI_SHARED_INHALE_KIRBY_BLOCK;
    }
    return ftRemixAiSharedIsKirbyDededeHatInhaled(status_id) ? FTREMIX_AI_SHARED_INHALE_PRESS_NSP
                                                             : FTREMIX_AI_SHARED_INHALE_SKIP;
}

int ftRemixAiSharedWalkExitsInDededeUsp(int status_id)
{
    return status_id == FTREMIX_AI_SHARED_DEDEDE_USP_MOVE;
}

int ftRemixAiSharedMarinaMayUseUsp(int jumps_used)
{
    return jumps_used == FTREMIX_AI_SHARED_MARINA_USP_JUMPS_USED;
}

int ftRemixAiSharedPkThunderAimsAtLedge(unsigned int status_total_tics)
{
    return status_total_tics <= FTREMIX_AI_SHARED_PKTHUNDER_MAX_TIME;
}

float ftRemixAiSharedPkThunderAimOffset(float own_x, float own_y, float ledge_x, float ledge_y)
{
    float x_distance = fabsf(ledge_x - own_x);
    float y_delta = (ledge_y - own_y) + FTREMIX_AI_SHARED_PKTHUNDER_LEDGE_Y_OFFSET;

    return FTREMIX_AI_SHARED_PKTHUNDER_MULTIPLIER * atan2f(y_delta, x_distance);
}

#endif
