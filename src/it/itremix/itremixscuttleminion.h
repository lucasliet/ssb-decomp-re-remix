#ifndef _ITREMIX_SCUTTLEMINION_H_
#define _ITREMIX_SCUTTLEMINION_H_

/*
 * Smash Remix Scuttle Minion item (Scuttle Town), ported from src/items/ScuttleMinion.asm and the minion routines of
 * Hazards.asm. The descriptor is dITRemixScuttleMinionItemDesc in itremixscuttleminion.c.
 */

#include "it/itremix/itremix.h"

#define ITREMIX_SCUTTLEMINION_KIND 0x3E /* Item.ScuttleMinion.id, Remix item ids 0x3A..0x3F */

/** SPAWN_ITEM of Scuttle Minion: makes the minion at `pos` with its hitbox; NULL when no item was made. */
GObj* itRemixScuttleMinionMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
