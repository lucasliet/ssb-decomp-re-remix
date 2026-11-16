#ifndef _ITREMIXCONKERGRENADERULES_H_
#define _ITREMIXCONKERGRENADERULES_H_

#include <stdint.h>

/* Pure rules of Conker's grenade (ConkerSpecial.asm grenade_hurtbox_collision_). No game types, so the headless tests can run them. */

#define ITREMIX_CONKER_GRENADE_RULES_HURT_MIN_DURATION 12

/**
 * Fuse left after the grenade hits a hurtbox or a shield (asm 1766-1778): the speed converted with cvt.w.s, which
 * rounds to nearest (ties to even) under the default FCSR mode, times 1.5 in integers, is taken off the fuse,
 * never below 12 frames.
 *
 * @param lifetime Frames left on the fuse before the hit.
 * @param abs_speed Absolute velocity of the grenade at the hit.
 * @return Frames left on the fuse after the hit.
 */
int32_t itRemixConkerGrenadeRulesHurtFuse(int32_t lifetime, float abs_speed);

#endif
