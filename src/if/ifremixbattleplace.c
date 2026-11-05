/**
 * ifremixbattleplace.c - access to the battle place counter for the Remix modes, see ifremixbattleplace.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "if/ifremixbattleplace.h"

extern s32 sIFCommonBattlePlace;

void ifRemixBattlePlaceRestoreRevived(void)
{
    sIFCommonBattlePlace++;
}

#endif
