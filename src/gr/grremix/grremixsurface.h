#ifndef _GRREMIXSURFACE_H_
#define _GRREMIXSURFACE_H_

/*
 * Remix ground knockback surfaces (Surface.asm get_struct_, _bounce and fgm_override_). ftMainGetGroundHitObstacle and
 * ftMainUpdateDamageStatGround call these under REGION_REMIX. The material to knockback table is in
 * grremixsurfacerules.c. The friction and conveyor routines are declared in ft/ftremix/ftremixhooks.h.
 */

#include <ft/fighter.h>
#include <gr/grdef.h>

/**
 * Replaces the body of ftMainGetGroundHitObstacle: finds the ground knockback struct under a fighter, TRUE when the
 * fighter is hit. The bounce surface makes the fighter jump and returns FALSE.
 */
sb32 grRemixSurfaceGetGroundHit(FTStruct *fp, GRAttackColl **p_gr_attack_coll);

/**
 * Surface.asm fgm_override_: when the struct comes from a Remix surface with its own sound, starts the 16 tic floor
 * damage wait, plays the sound and returns TRUE, which makes ftMainUpdateDamageStatGround skip its kind switch.
 */
sb32 grRemixSurfaceApplyFgmOverride(FTStruct *fp, const GRAttackColl *gr_attack_coll);

#endif
