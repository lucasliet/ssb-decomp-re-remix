#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixailevelrules.h"

/** Clamps any level above FTREMIX_AI_LEVEL_CLAMP_MAX down to it. */
int ftRemixAiLevelClamp(int level)
{
    if (level > FTREMIX_AI_LEVEL_CLAMP_MAX)
    {
        return FTREMIX_AI_LEVEL_CLAMP_MAX;
    }
    return level;
}

/** The threshold shrinks by 2 per level and never goes below 0. */
int ftRemixAiLevelWalkStopThreshold(int level)
{
    int threshold = (-level * 2) + 18;

    if (threshold < 0)
    {
        return 0;
    }
    return threshold;
}

/** Only level 10 underestimates the collision size. */
float ftRemixAiLevelCollisionScale(int level)
{
    if (level >= FTREMIX_AI_LEVEL_CLAMP_MAX + 1)
    {
        return FTREMIX_AI_LEVEL_COLL_SIZE_LV10;
    }
    return FTREMIX_AI_LEVEL_COLL_SIZE_DEFAULT;
}

#endif
