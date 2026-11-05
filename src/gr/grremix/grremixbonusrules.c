/**
 * grremixbonusrules.c - pure rules of the Remix bonus stage hazards, see grremixbonusrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixbonusrules.h"
#include "gr/grremix/grremixids.h"

int grRemixBonusRulesGetRollingBomb(int gkind, float *x, float *y, int *first_wait)
{
    switch (gkind)
    {
    case nGRRemixStage_BTT_MARINA:
        *x = 4050.0F;
        *y = 6000.0F;
        *first_wait = GRREMIX_BONUS_BOMB_WAIT_MARINA;
        return 1;

    case nGRRemixStage_BTP_MTWO:
        *x = 4800.0F;
        *y = 4200.0F;
        *first_wait = GRREMIX_BONUS_BOMB_WAIT_DEFAULT;
        return 1;

    case nGRRemixStage_BTP_CRASH:
        *x = 3150.0F;
        *y = 8100.0F;
        *first_wait = GRREMIX_BONUS_BOMB_WAIT_DEFAULT;
        return 1;

    default:
        return 0;
    }
}

float grRemixBonusRulesGetDokanSideThreshold(int gkind)
{
    return (gkind == nGRRemixStage_BTT_PEACH) ? GRREMIX_BONUS_DOKAN_SIDE_THRESHOLD_PEACH : GRREMIX_BONUS_DOKAN_SIDE_THRESHOLD_VANILLA;
}

#endif
