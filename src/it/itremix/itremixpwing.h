#ifndef _ITREMIXPWING_H_
#define _ITREMIXPWING_H_

#include "it/itremix/itremix.h"

/* Spawns Pwing (Remix item 0x37). Fighter call sites use this signature. */
GObj* itRemixPwingMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Ends every player's active Pwing. Called from the scene-end path that Remix runs as Pwing.clear_active_pwings_. */
void itRemixPwingClearActive(void);

extern const ITRemixItemDesc dITRemixPwingItemDesc;

#endif
