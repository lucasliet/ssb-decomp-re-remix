#ifndef _ITREMIXCLOUD_H_
#define _ITREMIXCLOUD_H_

#include <it/item.h>
#include <ft/fighter.h>

/* Spawn routine for Item.Cloud (0x49). Goemon's USP calls it with the vanilla item spawn ABI. */
GObj* itRemixCloudMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Cloud.asm enable_flash: TRUE for kinds whose GObj flags the item main loop must keep. */
sb32 itRemixKeepsFlashFlags(s32 kind);

#endif
