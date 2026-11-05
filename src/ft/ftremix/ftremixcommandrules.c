/**
 * ftremixcommandrules.c - pure rule of the animation translation multiplier, see ftremixcommandrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixcommandrules.h"

float ftRemixCommandGetSizeFactor(float size_multiplier)
{
    if (size_multiplier == 1.0F)
    {
        return 1.0F;
    }
    if (size_multiplier <= 1.0F)
    {
        return FTREMIXCOMMAND_POISON_MULT;
    }
    return FTREMIXCOMMAND_SUPER_MULT;
}

#endif
