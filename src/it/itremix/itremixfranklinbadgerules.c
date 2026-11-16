#include "it/itremix/itremixfranklinbadgerules.h"

ITRemixFranklinBadgeMaintain itRemixFranklinBadgeRulesGetMaintain(int32_t owns_hitbox, uint16_t flags)
{
    if (owns_hitbox != 0)
    {
        return nITRemixFranklinBadgeMaintainReflect;
    }
    if (flags == ITREMIX_FRANKLINBADGE_RULES_FLAGS_ABSORB)
    {
        return nITRemixFranklinBadgeMaintainNone;
    }
    if (flags == ITREMIX_FRANKLINBADGE_RULES_FLAGS_NESS)
    {
        return nITRemixFranklinBadgeMaintainDropReflect;
    }
    if ((flags & ITREMIX_FRANKLINBADGE_RULES_FLAG_REFLECT) != 0)
    {
        return nITRemixFranklinBadgeMaintainNone;
    }
    return nITRemixFranklinBadgeMaintainRestore;
}

int32_t itRemixFranklinBadgeRulesKeepsReflectAtExpiry(ITRemixFranklinBadgeMaintain maintain, uint16_t flags)
{
    if (maintain == nITRemixFranklinBadgeMaintainDropReflect)
    {
        return 1;
    }
    if (maintain != nITRemixFranklinBadgeMaintainNone)
    {
        return 0;
    }
    return (flags != ITREMIX_FRANKLINBADGE_RULES_FLAGS_ABSORB) ? 1 : 0;
}
