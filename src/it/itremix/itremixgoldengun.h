#ifndef _ITREMIX_GOLDENGUN_H_
#define _ITREMIX_GOLDENGUN_H_

/*
 * Smash Remix Golden Gun item, ported from src/items/GoldenGun.asm.
 * The descriptor is dITRemixGoldenGunItemDesc in itremixgoldengun.c.
 */

#include "it/itremix/itremix.h"

/** Item.GoldenGun.id: Remix item kind of the Golden Gun. */
#define ITREMIX_GOLDENGUN_ID 0x35

/** SPAWN_ITEM of GoldenGun: spawns a golden gun owned by `parent_gobj`, loaded from the common item file. */
GObj* itRemixGoldenGunMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
