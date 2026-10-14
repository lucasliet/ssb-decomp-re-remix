#ifndef _ITREMIX_GOLDENGUN_H_
#define _ITREMIX_GOLDENGUN_H_

/*
 * Smash Remix Golden Gun item, ported from src/items/GoldenGun.asm.
 * The descriptor is dITRemixGoldenGunItemDesc in itremixgoldengun.c.
 */

#include "it/itremix/itremix.h"

/** SPAWN_ITEM of GoldenGun: spawns a golden gun owned by `parent_gobj`, loaded from the common item file. */
GObj* itRemixGoldenGunMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
