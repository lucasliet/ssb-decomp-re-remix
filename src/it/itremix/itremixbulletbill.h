#ifndef _ITREMIX_BULLETBILL_H_
#define _ITREMIX_BULLETBILL_H_

/*
 * Smash Remix Bullet Bill item (Banzai Bill, Peach's Castle II), ported from the Banzai Bill routines of
 * Hazards.asm (banzai_bill_stage_setting_, banzai_bill_main_, banzai_bill_destroy_, banzai_bill_blast_zone_).
 * The descriptor is dITRemixBulletBillItemDesc in itremixbulletbill.c.
 */

#include "it/itremix/itremix.h"

#define ITREMIX_BULLETBILL_KIND 0x3D /* Remix item id 0x3D, Item.BulletBill.id */

/** SPAWN_ITEM of Bullet Bill: creates the item at `pos` with the stage file set by itRemixBulletBillSetStageFile. */
GObj* itRemixBulletBillMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/** Binds the stage header the item footer (0xE0) and the animation tracks are read from, and clears the life timer. */
void itRemixBulletBillSetStageFile(void *stage_header);

/** Starts the joint and material animation tracks of a spawned bullet (asm 0x8000BD1C and 0x8000BD54). */
void itRemixBulletBillStartAnimation(GObj *item_gobj);

#endif
