#include "it/itremix/itremixdekunutrules.h"

uint32_t itRemixDekuNutRulesKnockbackScale(uint32_t knockback_scale)
{
    return ((uint32_t) ITREMIX_DEKUNUT_RULES_KNOCKBACK_HALFWORD << 16) | (knockback_scale & 0xFFFF);
}
