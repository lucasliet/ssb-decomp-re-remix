#ifndef _ITREMIXBASKETBALL_H_
#define _ITREMIXBASKETBALL_H_

#include "it/itremix/itremix.h"

/* Spawns the Smashketball basketball (Remix item 0x40). Fighter and mode call sites use this signature. */
GObj* itRemixBasketballMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixBasketballItemDesc;

#endif
