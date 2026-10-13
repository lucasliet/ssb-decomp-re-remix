#ifndef _ITREMIXBOOMERANG_H_
#define _ITREMIXBOOMERANG_H_

#include <ssb_types.h>
#include <sys/objdef.h>
#include <it/itdef.h>
#include "it/itremix/itremix.h"

extern const ITRemixItemDesc dITRemixBoomerangItemDesc;

/* Marina pull: spawns a Boomerang owned by parent_gobj (Boomerang.asm stage_setting_). Returns NULL when no item was made. */
GObj* itRemixBoomerangMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
