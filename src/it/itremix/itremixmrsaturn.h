#ifndef _ITREMIXMRSATURN_H_
#define _ITREMIXMRSATURN_H_

#include "it/itremix/itremix.h"

/* Spawns Mr. Saturn (Remix item 0x38). Fighter call sites use this signature. */
GObj* itRemixMrSaturnMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixMrSaturnItemDesc;

#endif
