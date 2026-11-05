#include "it/itremix/itremixdisplaylinkrules.h"

#define ITREMIX_DISPLAY_LINK_KIND_KLAPTRAP 0x3A
#define ITREMIX_DISPLAY_LINK_KIND_ROBOTBEE 0x3B
#define ITREMIX_DISPLAY_LINK_KIND_CAR 0x3C
#define ITREMIX_DISPLAY_LINK_KIND_CACODEMON 0x3F

int32_t itRemixDisplayLinkRulesForKind(int32_t item_kind)
{
    switch (item_kind)
    {
    case ITREMIX_DISPLAY_LINK_KIND_CAR:
        return ITREMIX_DISPLAY_LINK_CAR;

    case ITREMIX_DISPLAY_LINK_KIND_KLAPTRAP:
    case ITREMIX_DISPLAY_LINK_KIND_ROBOTBEE:
    case ITREMIX_DISPLAY_LINK_KIND_CACODEMON:
        return ITREMIX_DISPLAY_LINK_HAZARD;

    default:
        return ITREMIX_DISPLAY_LINK_DEFAULT;
    }
}
