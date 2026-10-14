#ifndef _GRREMIXMETALLICBEELASER_H_
#define _GRREMIXMETALLICBEELASER_H_

/*
 * Robot Bee laser of Metallic Madness, a Remix stage projectile (Hazards.asm robot_bee_laser_*).
 * The Robot Bee item fires it; its descriptor is dWPMetallicBeeLaserWeaponDesc in grremixmetallicbeelaser.c.
 */

#include <ssb_types.h>
#include <sys/objdef.h>

/** Fires the laser from the Robot Bee `bee_gobj` (asm robot_bee_laser_). Returns the laser GObj, or NULL. */
GObj *grRemixMetallicBeeLaserFire(GObj *bee_gobj);

#endif
