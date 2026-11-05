/**
 * itremixpickupskip.c - Smash Remix Item.asm skip_item_spawn_gfx_._pickup (0x80172DB8) and skip_item_pickup_sound_
 * (0x80172E10): while the flag is set, itMainSetFighterHold creates no pickup swirl and plays no item-get sound.
 * create_and_assign_item_ sets both flags (it always passes the same value for the two) around the pickup and clears
 * them afterwards, so the port keeps one flag.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "it/itremix/itremixpickupskip.h"

static sb32 sITRemixPickupIsSkipped = FALSE;

void itRemixPickupSkipSet(sb32 is_skip)
{
    sITRemixPickupIsSkipped = is_skip;
}

sb32 itRemixPickupSkipIsActive(void)
{
    return sITRemixPickupIsSkipped;
}

#endif
