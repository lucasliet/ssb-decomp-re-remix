#ifndef _GRREMIXPEACHBUMPER_H_
#define _GRREMIXPEACHBUMPER_H_

/*
 * Peach's Castle bumper hitbox of the Remix build (Hazards.asm peach_bumper_hitbox_, RAM 0x8017D710).
 * itGBumperMakeItem calls this under #ifdef REGION_REMIX instead of its vanilla Peach's Castle block.
 */

#include <ssb_types.h>
#include <it/item.h>

/** Sets the bumper hitbox of the item for the current stage: castle knockback on the Peach's Castle stages, casino hitbox on Casino Night Zone. */
void grRemixPeachBumperSetupHitbox(GObj *item_gobj);

#endif
