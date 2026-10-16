/**
 * grremixhardcoding.c - Mushroom Kingdom Remix offset selection for the vanilla Inishie and Pakkun makers.
 * See grremixhardcoding.h.
 */

#ifdef PORT

#include "grremixhardcoding.h"
#include "grremixids.h"
#include <sc/scene.h>

intptr_t grRemixSelectMKOffset(intptr_t vanilla_offset, intptr_t remix_offset)
{
    if (gSCManagerBattleState->gkind == nGRRemixStage_MK_REMIX)
    {
        return remix_offset;
    }
    return vanilla_offset;
}

#endif
