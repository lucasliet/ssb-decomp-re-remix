#ifndef _ITREMIXFLASHBANG_H_
#define _ITREMIXFLASHBANG_H_

#ifdef PORT

#include <it/item.h>

/*
 * Peppy's Flashbang (Remix item id 0x4A), tossed by PeppyDSP. The fighter passes
 * its right-hand position and a zero velocity. The flashbang takes its owner,
 * team, port and launch power from the fighter. The fifth argument of the Remix
 * spawn is ignored, as the asm does.
 */
GObj* itRemixFlashbangMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif

#endif
