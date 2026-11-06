/**
 * mnremixvsstaminarules.c - pure rules of the Stamina HP picker, see mnremixvsstaminarules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnvsmode/mnremixvsstaminarules.h"

#define MNREMIX_VSSTAMINA_RULES_SMALL_LIMIT 10
#define MNREMIX_VSSTAMINA_RULES_DOWN_SNAP_FROM 991
#define MNREMIX_VSSTAMINA_RULES_DOWN_SNAP_TO 990
#define MNREMIX_VSSTAMINA_RULES_UP_WRAP_FROM 990
#define MNREMIX_VSSTAMINA_RULES_STICK_STEP 10
#define MNREMIX_VSSTAMINA_RULES_CSS_UP_SNAP_FROM 989
#define MNREMIX_VSSTAMINA_RULES_CSS_UP_SNAP_TO 990
#define MNREMIX_VSSTAMINA_RULES_CSS_LOW_SNAP_FROM 11
#define MNREMIX_VSSTAMINA_RULES_CSS_LOW_SNAP_TO 10

static int mnRemixVSStaminaRulesMenuStepDown(int hp, int is_stick)
{
    if (hp <= MNREMIX_VSSTAMINA_RULES_HP_MIN)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MAX;
    }
    if (is_stick == 0)
    {
        return hp - 1;
    }
    if (hp <= MNREMIX_VSSTAMINA_RULES_SMALL_LIMIT)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MIN;
    }
    if (hp >= MNREMIX_VSSTAMINA_RULES_DOWN_SNAP_FROM)
    {
        return MNREMIX_VSSTAMINA_RULES_DOWN_SNAP_TO;
    }
    return hp - MNREMIX_VSSTAMINA_RULES_STICK_STEP;
}

static int mnRemixVSStaminaRulesMenuStepUp(int hp, int is_stick)
{
    if (hp >= MNREMIX_VSSTAMINA_RULES_HP_MAX)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MIN;
    }
    if (is_stick == 0)
    {
        return hp + 1;
    }
    if (hp >= MNREMIX_VSSTAMINA_RULES_UP_WRAP_FROM)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MIN;
    }
    if (hp < MNREMIX_VSSTAMINA_RULES_SMALL_LIMIT)
    {
        return MNREMIX_VSSTAMINA_RULES_SMALL_LIMIT;
    }
    return hp + MNREMIX_VSSTAMINA_RULES_STICK_STEP;
}

int mnRemixVSStaminaRulesMenuStep(int hp, int is_increase, int is_stick)
{
    if (is_increase != 0)
    {
        return mnRemixVSStaminaRulesMenuStepUp(hp, is_stick);
    }
    return mnRemixVSStaminaRulesMenuStepDown(hp, is_stick);
}

int mnRemixVSStaminaRulesCssStep(int hp, int is_increase)
{
    int next = hp + ((is_increase != 0) ? MNREMIX_VSSTAMINA_RULES_STICK_STEP : -MNREMIX_VSSTAMINA_RULES_STICK_STEP);

    if (next == 0)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MIN;
    }
    if (next == MNREMIX_VSSTAMINA_RULES_CSS_UP_SNAP_FROM)
    {
        return MNREMIX_VSSTAMINA_RULES_CSS_UP_SNAP_TO;
    }
    if (next == MNREMIX_VSSTAMINA_RULES_CSS_LOW_SNAP_FROM)
    {
        return MNREMIX_VSSTAMINA_RULES_CSS_LOW_SNAP_TO;
    }
    if (next == (MNREMIX_VSSTAMINA_RULES_HP_MAX + 1))
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MAX;
    }
    if (next < 0)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MAX;
    }
    if (next > MNREMIX_VSSTAMINA_RULES_HP_MAX)
    {
        return MNREMIX_VSSTAMINA_RULES_HP_MIN;
    }
    return next;
}

#endif
