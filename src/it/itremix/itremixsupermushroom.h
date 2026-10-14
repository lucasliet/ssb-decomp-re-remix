#ifndef _ITREMIXSUPERMUSHROOM_H_
#define _ITREMIXSUPERMUSHROOM_H_

#include "it/itremix/itremix.h"

/* Spawns Super Mushroom (Remix item 0x2E). Fighter call sites use this signature. */
GObj* itRemixSuperMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Frees every active mushroom routine and forgets the per-port owners. Meant for the scene-change hook (see notes, Vanilla patches). */
void itRemixSuperMushroomClearStates(void);

extern const ITRemixItemDesc dITRemixSuperMushroomItemDesc;

#endif
