/**
 * gmremixgroupdisplayrules.c - pure flag rules of the Remix display groups, see gmremixgroupdisplayrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgroupdisplayrules.h"

uint32_t gmRemixGroupDisplayRulesSetFlags(uint32_t hide)
{
    return hide;
}

uint32_t gmRemixGroupDisplayRulesTempFlags(uint32_t flags, uint32_t hide)
{
    if (hide == 0)
    {
        return flags >> 16;
    }
    return (flags << 16) | hide;
}

#endif
