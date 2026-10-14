#ifndef _ITREMIXPOISONMUSHROOM_H_
#define _ITREMIXPOISONMUSHROOM_H_

#include "it/itremix/itremix.h"

/* Spawns Poison Mushroom (Remix item 0x2F). Fighter call sites use this signature. */
GObj* itRemixPoisonMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixPoisonMushroomItemDesc;

#endif
