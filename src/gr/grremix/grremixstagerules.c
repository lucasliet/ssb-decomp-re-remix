/**
 * grremixstagerules.c - pure stage class rules of Stages.asm, see grremixstagerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixstagerules.h"
#include "gr/grremix/grremixids.h"

int grRemixStageRulesEndsWithGameSet(int gkind, unsigned int ground_function)
{
    if (gkind < GRREMIX_STAGERULES_FIRST_NON_BATTLE_ID)
    {
        return 1;
    }
    if ((gkind == nGRRemixStage_HRC) || (gkind == nGRRemixStage_BTT_CRASH) || (gkind == nGRRemixStage_BTP_CRASH))
    {
        return 0;
    }
    return ground_function != 0;
}

int grRemixStageRulesHasSmallExplosion(int gkind, int stage_class)
{
    if (gkind < GRREMIX_STAGERULES_FIRST_NON_BATTLE_ID)
    {
        return 0;
    }
    return stage_class != GRREMIX_STAGERULES_CLASS_BATTLE;
}

#endif
