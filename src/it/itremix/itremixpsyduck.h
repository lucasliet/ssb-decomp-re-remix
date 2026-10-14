#ifndef _ITREMIXPSYDUCK_H_
#define _ITREMIXPSYDUCK_H_

#include "it/itremix/itremix.h"

/* Spawns Psyduck (Remix item 0x41). Fighter and Poke Ball call sites use this signature. */
GObj* itRemixPsyduckMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixPsyduckItemDesc;

#endif
