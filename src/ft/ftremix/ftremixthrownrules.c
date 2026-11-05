/**
 * ftremixthrownrules.c - pure rule of bowser.asm thrown_mario_bros_fix_, see ftremixthrownrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixthrownrules.h"
#include "remix/remix_roster_data.h"

int ftRemixThrownRulesKeepsStatusAtAnimEnd(int fkind)
{
    return (fkind == REMIX_ID_BOWSER) || (fkind == REMIX_ID_GBOWSER) || (fkind == REMIX_ID_NBOWSER);
}

#endif
