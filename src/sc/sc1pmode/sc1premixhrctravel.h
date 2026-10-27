#ifndef _SC1PREMIXHRCTRAVEL_H_
#define _SC1PREMIXHRCTRAVEL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/objdef.h>

/*
 * Projectiles and the player of the Home-Run Contest that go far down the field (HRC.asm update_projectile_and_hmn_positions_): each
 * one is placed relative to the warped camera. The rules are in sc1premixhrctravelrules.h.
 */

/** Forgets every object followed so far; called at the start of an attempt. */
extern void sc1PRemixHrcTravelReset(void);

/**
 * Places the player and every projectile for this frame.
 *
 * @param human_gobj Fighter of the player.
 * @param camera_x X the camera looks at, after this frame's warp.
 * @param warp_distance Total the camera has warped back.
 */
extern void sc1PRemixHrcTravelUpdate(GObj *human_gobj, float camera_x, float warp_distance);

#endif

#endif
