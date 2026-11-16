#include "it/itremix/itremixconkergrenaderules.h"

#include <math.h>

int32_t itRemixConkerGrenadeRulesHurtFuse(int32_t lifetime, float abs_speed)
{
    int32_t whole_speed = (int32_t) lrintf(abs_speed);
    int32_t fuse = lifetime - (whole_speed + (whole_speed >> 1));

    if (fuse < ITREMIX_CONKER_GRENADE_RULES_HURT_MIN_DURATION)
    {
        return ITREMIX_CONKER_GRENADE_RULES_HURT_MIN_DURATION;
    }
    return fuse;
}
