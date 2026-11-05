/**
 * ftremixaimodesrules.c - pure rules of the CPU hooks of the Remix game modes, see ftremixaimodesrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaimodesrules.h"

float ftRemixAiModesBallSeekOffset(int team, int home_team)
{
    if (team == home_team)
    {
        return -FTREMIX_AI_MODES_BALL_SEEK_OFFSET;
    }
    return FTREMIX_AI_MODES_BALL_SEEK_OFFSET;
}

float ftRemixAiModesSquaredDistance(float from_x, float from_y, float to_x, float to_y)
{
    float distance_x = from_x - to_x;
    float distance_y = from_y - to_y;

    return (distance_x * distance_x) + (distance_y * distance_y);
}

float ftRemixAiModesHillTargetY(float marker_y)
{
    return marker_y - FTREMIX_AI_MODES_HILL_MARKER_DROP;
}

#endif
