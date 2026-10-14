#ifndef _ITREMIX_DEKUNUT_H_
#define _ITREMIX_DEKUNUT_H_

/*
 * Smash Remix Deku Nut item (kind 0x32), ported from src/items/DekuNut.asm.
 * The descriptor is dITRemixDekuNutItemDesc in itremixdekunut.c.
 */

#include "it/itremix/itremix.h"

/** SPAWN_ITEM of Deku Nut: spawns a nut owned by `parent_gobj`, using the common item file. */
GObj* itRemixDekuNutMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
