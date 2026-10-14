#ifndef _ITREMIXLIGHTNING_H_
#define _ITREMIXLIGHTNING_H_

#include "it/itremix/itremix.h"

/* Spawns Lightning (Remix item 0x31). Fighter call sites use this signature. */
GObj* itRemixLightningMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Ends every active Lightning routine and shrink timer. Called from the item clean-up path (see notes, Vanilla patches). */
void itRemixLightningClearActive(void);

extern const ITRemixItemDesc dITRemixLightningItemDesc;

#endif
