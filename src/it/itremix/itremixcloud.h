#ifndef _ITREMIXCLOUD_H_
#define _ITREMIXCLOUD_H_

#include <it/item.h>
#include <ft/fighter.h>

/* Spawn routine for Item.Cloud (0x49). Goemon's USP calls it with the vanilla item spawn ABI. */
GObj* itRemixCloudMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
