#ifndef _ITREMIXBASKETBALLRULES_H_
#define _ITREMIXBASKETBALLRULES_H_

#include <stdint.h>

/* Pure rules of the Basketball bounce (Basketball.asm bounce_patch_). No game types, so the headless tests can run them. */

#define ITREMIX_BASKETBALL_RULES_BOUNCE_HITLAG_SCALE 0.030029296875F /* 0x3CF60000 */

/**
 * Hitlag frames of a bounce (asm _calculate_hitlag): speed * 0.03 converted with cvt.w.s, which rounds to nearest
 * (ties to even) under the default FCSR mode, plus 1.
 *
 * @param speed Absolute velocity of the ball at the bounce.
 * @return Frames the ball stays frozen.
 */
int32_t itRemixBasketballRulesBounceHitlag(float speed);

#endif
