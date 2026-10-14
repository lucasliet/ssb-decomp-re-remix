#ifndef _ITREMIXDANGO_H_
#define _ITREMIXDANGO_H_

#include "it/itremix/itremix.h"

/* Spawns Dango (Remix item 0x36). Fighter call sites use this signature. */
GObj* itRemixDangoMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixDangoItemDesc;

#endif
