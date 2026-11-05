#ifndef _ITREMIXPICKUPSKIP_H_
#define _ITREMIXPICKUPSKIP_H_

/*
 * Smash Remix Item.asm skip_item_spawn_gfx_._pickup and skip_item_pickup_sound_: a flag that makes itMainSetFighterHold
 * skip the pickup swirl and the item-get sound for an item handed to a fighter (Start With, Taunt Item).
 */

#include <ssb_types.h>

/** Sets whether itMainSetFighterHold skips the pickup swirl and the item-get sound (Item.asm create_and_assign_item_). */
void itRemixPickupSkipSet(sb32 is_skip);

/** TRUE while itMainSetFighterHold must skip the pickup swirl and the item-get sound. */
sb32 itRemixPickupSkipIsActive(void);

#endif
