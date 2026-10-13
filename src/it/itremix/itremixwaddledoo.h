#ifndef _ITREMIXWADDLEDOO_H_
#define _ITREMIXWADDLEDOO_H_

#ifdef PORT

#include "it/itremix/itremix.h"

/* Waddle Doo (Item.WaddleDoo.id 0x47) descriptor. */
extern const ITRemixItemDesc dITRemixWaddleDooItemDesc;

/* Item.WaddleDoo.SPAWN_ITEM: WaddleDee.stage_setting_ with Doo's item_info_array. */
extern GObj* itRemixWaddleDooMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Starts the laser attack. Called by WaddleDee and ScuttleMinion ports (asm: Item.WaddleDoo.waddle_doo_attack_initial). */
extern void itRemixWaddleDooAttackInitial(GObj *item_gobj);

#endif

#endif
