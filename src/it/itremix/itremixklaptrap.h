#ifndef _ITREMIX_KLAPTRAP_H_
#define _ITREMIX_KLAPTRAP_H_

/*
 * Smash Remix KlapTrap item (Jungle Japes crocodile), ported from src/items/KlapTrap.asm.
 * The descriptor is dITRemixKlapTrapItemDesc in itremixklaptrap.c.
 */

#include "it/itremix/itremix.h"

#define ITREMIX_KLAPTRAP_ID 0x3A /* Item.KlapTrap.id */

/** SPAWN_ITEM of KlapTrap (asm klaptrap_stage_setting_): creates the item at `pos` with no owner. */
GObj* itRemixKlapTrapMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/** Binds the Japes stage file the item footer is read from, and clears the respawn timer. */
void itRemixKlapTrapSetStageFile(void *stage_file);

/** Blast zone callback (asm klaptrap_blast_zone_): sends the item back to its spawn point. */
void itRemixKlapTrapBlastZone(GObj *item_gobj);

#endif
