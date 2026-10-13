#ifndef _ITREMIXGORDO_H_
#define _ITREMIXGORDO_H_

#ifdef PORT

#include "it/itremix/itremix.h"

/* Gordo (Item.Gordo.id 0x48) descriptor. Spawn is itRemixGordoMakeItem. */
extern const ITRemixItemDesc dITRemixGordoItemDesc;

/* Spawns a Gordo owned by parent_gobj (Dedede's down special). Fighter call sites are wired by another worker. */
extern GObj* itRemixGordoMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif

#endif
