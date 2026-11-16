#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixids.h"
#include "ft/ftremix/ftremixaistagerules.h"

static int ftRemixAiStageIsZebesBased(int gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_PLANET_ZEBES:
    case nGRRemixStage_NORFAIR:
    case nGRRemixStage_ZEBES_DL:
    case nGRRemixStage_ZEBES_O:
    case nGRRemixStage_NORFAIR_REMIX:
        return 1;
    }
    return 0;
}

int ftRemixAiStageHasZebesAcid(int gkind, int hazards_on)
{
    return hazards_on && ftRemixAiStageIsZebesBased(gkind);
}

int ftRemixAiStageHasHyruleTornado(int gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_HYRULE_CASTLE:
    case nGRRemixStage_HCASTLE_DL:
    case nGRRemixStage_HCASTLE_O:
    case nGRRemixStage_HCASTLE_REMIX:
        return 1;
    }
    return 0;
}

int ftRemixAiStageFleesDroppingPlatform(int gkind, float joint_y)
{
    switch (gkind)
    {
    case nGRRemixStage_MUSHROOM_KINGDOM:
    case nGRRemixStage_MK_REMIX:
    case nGRRemixStage_BIG_BOOS_HAUNT:
    case nGRRemixStage_YOSHIS_ISLAND_II:
        return joint_y < -100.0F;

    case nGRRemixStage_GREAT_BAY:
        return joint_y < -1000.0F;
    }
    return 0;
}

#endif
