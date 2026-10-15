#ifndef _ITREMIX_GOLDENGUN_H_
#define _ITREMIX_GOLDENGUN_H_

/*
 * Smash Remix Golden Gun item, ported from src/items/GoldenGun.asm.
 * The descriptor is dITRemixGoldenGunItemDesc in itremixgoldengun.c.
 */

#include "it/itremix/itremix.h"

/** Item.GoldenGun.id: Remix item kind of the Golden Gun. */
#define ITREMIX_GOLDENGUN_ID 0x35

/** SPAWN_ITEM of GoldenGun: spawns a golden gun owned by `parent_gobj`, loaded from the common item file. */
GObj* itRemixGoldenGunMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/** TRUE when the fighter holds a Golden Gun (GoldenGun.asm projectile_struct_fix: held item id check). */
sb32 itRemixGoldenGunIsHeldBy(GObj *fighter_gobj);

/** GoldenGun.asm golden_bullet_projectile_struct as a WPDesc (patch at 0x80175A6C). */
WPDesc* itRemixGoldenGunGetBulletDesc(void);

/** GoldenGun.asm projectile_properties_fix (patch at 0x80175A8C): damage, knockback, angle and hit sound of the bullet. */
void itRemixGoldenGunSetupBullet(GObj *weapon_gobj);

#endif
