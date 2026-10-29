/**
 * gmremixstaminarules.c - pure rules of Stamina, see gmremixstaminarules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixstaminarules.h"

int gmRemixStaminaRulesTotalHP(int hp)
{
    return hp - 1;
}

int gmRemixStaminaRulesIsOverTotal(int percent, int total_hp)
{
    return (percent > total_hp) ? 1 : 0;
}

int gmRemixStaminaRulesDamageToFinish(int total_hp, int percent)
{
    return total_hp + 1 - percent;
}

int gmRemixStaminaRulesHPLeft(int total_hp, int damage)
{
    int hitpoints_left = total_hp + 1 - damage;

    if (hitpoints_left < 0)
    {
        return 0;
    }
    return hitpoints_left;
}

#endif
