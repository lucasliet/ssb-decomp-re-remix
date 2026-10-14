#ifndef _ITREMIX_PITFALL_H_
#define _ITREMIX_PITFALL_H_

/*
 * Smash Remix Pitfall item (id 0x34), ported from src/items/Pitfall.asm.
 * The descriptor is dITRemixPitfallItemDesc in itremixpitfall.c.
 */

#include "it/itremix/itremix.h"

/** SPAWN_ITEM of Pitfall: spawns a pitfall from the common item file, with an optional owner. */
GObj* itRemixPitfallMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
