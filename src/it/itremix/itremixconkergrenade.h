#ifndef _ITREMIXCONKERGRENADE_H_
#define _ITREMIXCONKERGRENADE_H_

#ifdef PORT

#include <it/item.h>

/*
 * Conker's grenade (vanilla item id 0x15, so no ITRemixItemDesc is exported).
 * Spawned by ConkerDSP when a grenade is tossed. The fighter passes its power
 * (0x0B20 in the Remix asm) and the throw position; the grenade takes its owner,
 * team, port and facing from the fighter.
 */
GObj *itRemixConkerGrenadeMakeItem(GObj *parent_gobj, Vec3f *pos, s32 power);

#endif

#endif
