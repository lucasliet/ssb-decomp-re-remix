#ifndef _ITREMIXCLANBOMB_H_
#define _ITREMIXCLANBOMB_H_

#include <it/item.h>

/** Spawns a Clan Bomb owned by parent_gobj (Marina's pull); returns NULL when the item could not be created. */
GObj* itRemixClanBombMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
