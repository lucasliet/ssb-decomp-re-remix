#ifndef _GRREMIXPIRATECANNONBALL_H_
#define _GRREMIXPIRATECANNONBALL_H_

/*
 * Pirate Land cannonball (Hazards.asm cannonball_stage_setting_, cannonball_main_,
 * cannonball_explosion_ and the projectile data tables). The cannon fires it; it is a
 * stage-owned weapon, so it uses its own WPDesc.
 */

#include <ssb_types.h>
#include <sys/objtypes.h>

#ifdef PORT

/** Spawns a cannonball at spawn_pos. anim_track is the joint animation track for the barrel position. */
GObj* grRemixPirateCannonballMake(Vec3f *spawn_pos, void *hitbox_file, AObjEvent32 *anim_track);

#endif

#endif
