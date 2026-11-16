#include "it/itremix/itremixbasketballrules.h"

#include <math.h>

int32_t itRemixBasketballRulesBounceHitlag(float speed)
{
    return (int32_t) lrintf(speed * ITREMIX_BASKETBALL_RULES_BOUNCE_HITLAG_SCALE) + 1;
}
