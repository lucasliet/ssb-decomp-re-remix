/**
 * grremixbarrelrules.c - pure rules of the Remix barrel stages, see grremixbarrelrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixbarrelrules.h"
#include "gr/grremix/grremixids.h"

#define GRREMIX_BARREL_SMASHKETBALL_Y_MODE 2944.0F
#define GRREMIX_BARREL_SMASHKETBALL_Y_VS 1775.0F

int grRemixBarrelRulesGetStageKind(int gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_CONGO_JUNGLE:
    case nGRRemixStage_FALLS:
    case nGRRemixStage_CONGOJ_DL:
    case nGRRemixStage_CONGOJ_O:
        return GRREMIX_BARREL_STAGE_HAZARD;

    case nGRRemixStage_SMASHKETBALL:
        return GRREMIX_BARREL_STAGE_SMASHKETBALL;

    default:
        return GRREMIX_BARREL_STAGE_NONE;
    }
}

int grRemixBarrelRulesSmashketballIsMade(int is_smashketball_mode, int hazards_on)
{
    return (is_smashketball_mode != 0) || (hazards_on != 0);
}

float grRemixBarrelRulesGetSmashketballY(int is_smashketball_mode)
{
    return (is_smashketball_mode != 0) ? GRREMIX_BARREL_SMASHKETBALL_Y_MODE : GRREMIX_BARREL_SMASHKETBALL_Y_VS;
}

#endif
