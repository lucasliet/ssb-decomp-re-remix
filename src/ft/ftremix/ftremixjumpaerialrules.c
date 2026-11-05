/**
 * ftremixjumpaerialrules.c - pure rules of the Remix double jump patches, see ftremixjumpaerialrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixjumpaerialrules.h"
#include "remix/remix_roster_data.h"

int ftRemixJumpAerialRulesUsesYoshiPhysics(int fkind)
{
    return (fkind == REMIX_ID_YOSHI) || (fkind == REMIX_ID_NYOSHI) || (fkind == REMIX_ID_JYOSHI);
}

float ftRemixJumpAerialRulesYoshiKnockbackResist(int fkind, float vanilla_resist)
{
    return (fkind == REMIX_ID_JYOSHI) ? FTREMIX_JUMPAERIAL_J_YOSHI_KNOCKBACK_RESIST : vanilla_resist;
}

int ftRemixJumpAerialRulesUsesNessPhysics(int fkind)
{
    switch (fkind)
    {
    case REMIX_ID_NESS:
    case REMIX_ID_NNESS:
    case REMIX_ID_JNESS:
    case REMIX_ID_LUCAS:
    case REMIX_ID_NLUCAS:
    case REMIX_ID_MTWO:
    case REMIX_ID_NMTWO:
    case REMIX_ID_PEACH:
    case REMIX_ID_NPEACH:
        return 1;

    default:
        return 0;
    }
}

#endif
