#ifndef _ITREMIXDEKUNUTRULES_H_
#define _ITREMIXDEKUNUTRULES_H_

#include <stdint.h>

/* Pure rules of the Deku Nut hitbox (DekuNut.asm spawn_deku_nut_ and throw_initial_). Ints only, so the headless tests can run them. */

#define ITREMIX_DEKUNUT_RULES_KNOCKBACK_HALFWORD 0x0100

/**
 * Knockback scale after the asm's "sh 0x100, 0x140(item struct)". The scale is a 32-bit word at 0x140, so the big-endian
 * halfword store replaces its high half and keeps the low half.
 *
 * @param knockback_scale Scale before the store (the value of the item attributes, or of an earlier store).
 * @return 0x0100 in the high half, the low half of knockback_scale unchanged.
 */
uint32_t itRemixDekuNutRulesKnockbackScale(uint32_t knockback_scale);

#endif
