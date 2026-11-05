/**
 * ftremixnessrules.c - pure rules of the Ness family patches (nessshared.asm and Lucas.asm), see ftremixnessrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixnessrules.h"
#include "remix/remix_roster_data.h"

int ftRemixNessRulesUsesOwnFiles(int power_kind)
{
    return (power_kind == REMIX_ID_JNESS) || (power_kind == REMIX_ID_LUCAS);
}

int ftRemixNessRulesPKFireIsAir(int power_kind, int is_air)
{
    if (power_kind == REMIX_ID_LUCAS)
    {
        return 0;
    }
    return is_air;
}

float ftRemixNessRulesPKFireSpeed(int power_kind, int is_air, float vanilla_speed)
{
    if (!ftRemixNessRulesUsesOwnFiles(power_kind))
    {
        return vanilla_speed;
    }
    return is_air ? FTREMIX_NESS_PKFIRE_SPEED_AIR : FTREMIX_NESS_PKFIRE_SPEED_GROUND;
}

int ftRemixNessRulesPKFireExplodes(int power_kind)
{
    return power_kind == REMIX_ID_LUCAS;
}

int ftRemixNessRulesPKFirePillarUsesOwnFile(int power_kind)
{
    return power_kind == REMIX_ID_JNESS;
}

float ftRemixNessRulesPKFirePillarGravity(int power_kind, float vanilla_gravity)
{
    if (power_kind != REMIX_ID_JNESS)
    {
        return vanilla_gravity;
    }
    return FTREMIX_NESS_PKFIRE_PILLAR_GRAVITY_JNESS;
}

float ftRemixNessRulesPKFirePillarTVel(int power_kind, float vanilla_tvel)
{
    if (power_kind != REMIX_ID_JNESS)
    {
        return vanilla_tvel;
    }
    return FTREMIX_NESS_PKFIRE_PILLAR_TVEL_JNESS;
}

float ftRemixNessRulesPKThunderLandingLag(int fkind, float vanilla_lag)
{
    if (fkind == REMIX_ID_JNESS)
    {
        return FTREMIX_NESS_PKTHUNDER_LANDING_LAG_JNESS;
    }
    if (fkind == REMIX_ID_LUCAS)
    {
        return FTREMIX_NESS_PKTHUNDER_LANDING_LAG_LUCAS;
    }
    return vanilla_lag;
}

float ftRemixNessRulesPKThunderDrift(int fkind, float vanilla_drift)
{
    if (fkind != REMIX_ID_LUCAS)
    {
        return vanilla_drift;
    }
    return FTREMIX_NESS_PKTHUNDER_DRIFT_LUCAS;
}

int ftRemixNessRulesPKThunderEndDelay(int fkind, int vanilla_delay)
{
    if (fkind != REMIX_ID_JNESS)
    {
        return vanilla_delay;
    }
    return FTREMIX_NESS_PKTHUNDER_END_DELAY_JNESS;
}

float ftRemixNessRulesPsiMagnetHealMultiplier(int fkind)
{
    if (fkind == REMIX_ID_LUCAS)
    {
        return FTREMIX_NESS_PSIMAGNET_HEAL_MULTIPLIER_LUCAS;
    }
    return FTREMIX_NESS_PSIMAGNET_HEAL_MULTIPLIER;
}

int ftRemixNessRulesPsiMagnetKeepsFacing(int fkind)
{
    return fkind == REMIX_ID_LUCAS;
}

int ftRemixNessRulesPKThunderHitRefreshDue(int lifetime)
{
    if (lifetime <= 0)
    {
        return 0;
    }
    return (lifetime % FTREMIX_LUCAS_PKTHUNDER_HIT_RATE) == 0;
}

#endif
