#ifndef _ITREMIXBLUESHELL_H_
#define _ITREMIXBLUESHELL_H_

#include "it/itremix/itremix.h"

/* Spawns Blue Shell (Remix item 0x30). Fighter and item call sites use this signature. */
GObj* itRemixBlueShellMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

extern const ITRemixItemDesc dITRemixBlueShellItemDesc;

/* Shared Blue Shell hit effect (BlueShell.asm hit_gfx_). Flashbang calls it with its own item object. */
void itRemixBlueShellHitGfx(GObj *item_gobj);

/* Fighter in first place of the match (BlueShell.asm get_player_in_first_). NULL when that port has no fighter. */
GObj* itRemixBlueShellGetPlayerInFirst(void);

#endif
