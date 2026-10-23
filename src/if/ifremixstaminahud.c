/**
 * ifremixstaminahud.c - Stamina HUD for the fighter damage digits (Stamina.asm stamina_display_, stamina_dark_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixstaminahud.h"
#include "gm/gmremixstamina.h"

#define IF_REMIX_STAMINA_DARKNESS_MAX 300.0F

/**
 * Value of the percent digits in Stamina: the HP left after `damage`, never below 0.
 * Outside Stamina returns `damage` unchanged.
 */
s32 ifRemixStaminaDisplayValue(s32 damage)
{
    s32 hitpoints_left;

    if (!gmRemixStaminaIsActive())
    {
        return damage;
    }
    hitpoints_left = gmRemixStaminaGetTotalHP() + 1 - damage;

    if (hitpoints_left < 0)
    {
        return 0;
    }
    return hitpoints_left;
}

/**
 * Damage ratio of the fighter HUD darkening. In Stamina the asm darkens by damage / HP, stored as an integer over 300
 * (cvt.w.s rounds to nearest, emulated with + 0.5). Outside Stamina returns damage / 300, the vanilla value.
 */
f32 ifRemixStaminaDamageRatio(s32 damage)
{
    f32 hitpoints;
    s32 darkness;

    if (!gmRemixStaminaIsActive())
    {
        return damage / IF_REMIX_STAMINA_DARKNESS_MAX;
    }
    hitpoints = gmRemixStaminaGetTotalHP() + 1;
    darkness = (s32) (((damage / hitpoints) * IF_REMIX_STAMINA_DARKNESS_MAX) + 0.5F);

    return darkness / IF_REMIX_STAMINA_DARKNESS_MAX;
}

#endif
