/**
 * ftremixtwelvecbdefeatedrules.c - defeated pose rules of the 12-Character Battle select, see ftremixtwelvecbdefeatedrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixtwelvecbdefeatedrules.h"

unsigned int ftRemixTwelveCBDefeatedRulesRowAddress(unsigned int array_address, int fkind)
{
    if ((fkind < 0) || (fkind > FTREMIX_TWELVECB_DEFEATED_LAST_FKIND))
    {
        return 0;
    }
    return array_address + ((unsigned int)fkind * FTREMIX_TWELVECB_DEFEATED_ROW_SIZE);
}

int ftRemixTwelveCBDefeatedRulesReplacesMotion(int is_vs_css, int is_defeated, unsigned int animation_file)
{
    return ((is_vs_css != 0) && (is_defeated != 0) && (animation_file != 0)) ? 1 : 0;
}

#endif
