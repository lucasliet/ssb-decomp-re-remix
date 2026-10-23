#ifndef _IFREMIXSTAMINAHUD_H_
#define _IFREMIXSTAMINAHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Smash Remix Stamina HUD (Stamina.asm stamina_display_, stamina_dark_, stamina_dark_fix_2): the percent digits show the
 * HP left and the damage darkening follows the HP ratio. Both functions return their vanilla value when Stamina is off.
 */

/** Value the percent digits show: remaining HP (Stamina HP minus damage, clamped at 0) in Stamina, otherwise the damage. */
s32 ifRemixStaminaDisplayValue(s32 damage);

/** Damage ratio used by the darkening: damage / 300 in vanilla, damage / HP (rounded, times 300, over 300) in Stamina. */
f32 ifRemixStaminaDamageRatio(s32 damage);

#endif

#endif
