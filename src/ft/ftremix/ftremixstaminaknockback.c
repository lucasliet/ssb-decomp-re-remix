/**
 * ftremixstaminaknockback.c - Smash Remix Stamina knockback (Stamina.asm stamina_knockback_, stamina_knockback_item_,
 * stamina_knockback_throw_, stamina_knockback_projectile_).
 *
 * While the Stamina rule is active every knockback is computed as if the victim were at
 * GMREMIX_STAMINA_KNOCKBACK_PERCENT. The asm patches four callers (fighter hit, item hit, projectile hit, throw and
 * tornado) but each one only changes the percent that ftParamGetCommonKnockback sums with the recent damage, and the
 * throw patch overwrites that sum for every caller of ftParamGetCommonKnockback. The port changes the sum in one place.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixstamina.h"

s32 ftRemixStaminaKnockbackDamageAdd(s32 percent_damage, s32 recent_damage)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return percent_damage + recent_damage;
    }
    return GMREMIX_STAMINA_KNOCKBACK_PERCENT;
}

#endif
