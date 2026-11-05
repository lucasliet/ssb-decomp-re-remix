#include "gr/grremix/grremixplatformrules.h"
#include "gr/grremix/grremixids.h"

static int32_t grRemixPlatformRulesMovesAsHazard(int32_t gkind)
{
    return (gkind == nGRRemixStage_ONETT) || (gkind == nGRRemixStage_TALTAL) || (gkind == nGRRemixStage_TALTAL_REMIX);
}

int32_t grRemixPlatformRulesMovementIsOn(int32_t gkind, int32_t hazard_type, int32_t hazards_on, int32_t movement_on)
{
    if (grRemixPlatformRulesMovesAsHazard(gkind))
    {
        return hazards_on;
    }
    if ((hazard_type & GRREMIX_PLATFORM_HAZARD_TYPE_MOVEMENT) == 0)
    {
        return 1;
    }
    return movement_on;
}
