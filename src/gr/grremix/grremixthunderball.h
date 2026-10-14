#ifndef _GRREMIXTHUNDERBALL_H_
#define _GRREMIXTHUNDERBALL_H_

/*
 * Cacodemon Thunderball, the stage projectile of the Spawned Fear hazard
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm thunderball_*). Its descriptor is
 * dWPRemixCacodemonThunderballWeaponDesc in grremixthunderball.c.
 */

#include <ssb_types.h>
#include <sys/obj.h>

#define GRREMIX_THUNDERBALL_KIND 0x2B /* Projectile.id.CACODEMON_THUNDERBALL: 12th add_projectile of Projectile.asm, after 0x20 */

/** Sets the Thunderball file (hitbox and graphics), read from the stage header at 0x130. */
void grRemixThunderballSetFile(void *file);

/** Fires one Thunderball from the Cacodemon item; returns the projectile, or NULL when it could not be made. */
GObj* grRemixThunderballShoot(GObj *cacodemon_gobj);

#endif
