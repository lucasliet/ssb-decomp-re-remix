/**
 * itremixheartpersist.c - SinglePlayerModes.asm heart_persist (0x8016F5BC): in the All-Star Rest Area (stage type 0) the
 * asm reads a fixed timer of 0x5785 for every heart instead of the item's own, so the fade and the destruction never start.
 * The port keeps the timer from counting down, which has the same effect.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "it/itremix/itremixheartpersist.h"
#include "it/item.h"
#include "sc/sc1pmode/sc1premixallstar.h"

sb32 itRemixHeartIsDespawnBlocked(GObj *item_gobj)
{
    if (itGetStruct(item_gobj)->kind != nITKindHeart)
    {
        return FALSE;
    }
    return sc1PRemixAllstarIsRestArea();
}

#endif
