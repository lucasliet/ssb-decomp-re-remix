#ifndef _ITREMIXSHURIKEN_H_
#define _ITREMIXSHURIKEN_H_

#include <ssb_types.h>
#include <sys/objdef.h>
#include <it/itdef.h>
#include "it/itremix/itremix.h"

extern const ITRemixItemDesc dITRemixShurikenItemDesc;

/** Spawns the Shuriken owned by `parent_gobj` (Remix Item.Shuriken SPAWN_ITEM). Returns NULL when no item was made. */
GObj* itRemixShurikenMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
