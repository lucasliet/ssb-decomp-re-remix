#ifndef _ITREMIXBLUESHELL_H_
#define _ITREMIXBLUESHELL_H_

#include "it/itremix/itremix.h"

/* Spawns Blue Shell (Remix item 0x30). Fighter and item call sites use this signature. */
GObj* itRemixBlueShellMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixBlueShellItemDesc;

#endif
