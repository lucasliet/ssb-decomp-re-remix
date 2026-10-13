#ifndef _ITREMIX_GEM_H_
#define _ITREMIX_GEM_H_

/*
 * Smash Remix Gem item (Marina's clanpot), ported from src/items/Gem.asm.
 * The descriptor is dITRemixGemItemDesc in itremixgem.c.
 */

#include "it/itremix/itremix.h"

/** SPAWN_ITEM of Gem: spawns a gem owned by `parent_gobj`, bound to the owner's item file. */
GObj* itRemixGemMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
