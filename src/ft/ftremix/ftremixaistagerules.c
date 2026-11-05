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

#endif
