/**
 * ftremixsonicspringrules.c - shared action check of Sonic's spring, see ftremixsonicspringrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/sonic/ftremixsonicspringrules.h"

int ftRemixSonicSpringRulesCommonAction(int action)
{
    if ((action >= FTREMIX_SONIC_SPRING_ACTION_JUMPF) && (action <= FTREMIX_SONIC_SPRING_ACTION_PASS))
    {
        return nFTRemixSonicSpringRulesChangeAction;
    }
    /* SonicSpecial.asm 1749-1751: `sltu` against FallSpecial (no +1, unlike 1744), so the code only matches Tumble,
     * although the comment at 1736 lists "Tumble - FallSpecial"; the port follows the code, pending an owner decision (M14 C3) */
    if (action == FTREMIX_SONIC_SPRING_ACTION_TUMBLE)
    {
        return nFTRemixSonicSpringRulesChangeAction;
    }
    if ((action == FTREMIX_SONIC_SPRING_ACTION_SHIELDBREAK) || (action == FTREMIX_SONIC_SPRING_ACTION_SHIELDBREAKFALL))
    {
        return nFTRemixSonicSpringRulesChangeAction;
    }
    if (action == FTREMIX_SONIC_SPRING_ACTION_INHALEPULLED)
    {
        return nFTRemixSonicSpringRulesIgnore;
    }
    return nFTRemixSonicSpringRulesByCharacter;
}

#endif
