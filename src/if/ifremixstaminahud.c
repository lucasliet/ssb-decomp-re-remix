/**
 * ifremixstaminahud.c - Stamina HUD for the fighter damage digits (Stamina.asm stamina_display_, stamina_dark_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixstaminahud.h"
#include "gm/gmremixstamina.h"
#include "gm/gmremixstaminarules.h"

#define IF_REMIX_STAMINA_DARKNESS_MAX 300.0F

/**
 * Value of the percent digits in Stamina: the HP left after `damage`, never below 0.
 * Outside Stamina returns `damage` unchanged.
 */
s32 ifRemixStaminaDisplayValue(s32 damage)
{
    if (!gmRemixStaminaIsActive())
    {
        return damage;
    }
    return gmRemixStaminaRulesHPLeft(gmRemixStaminaGetTotalHP(), damage);
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
    hitpoints = gmRemixStaminaRulesMaxHP(gmRemixStaminaGetTotalHP());
    darkness = (s32) (((damage / hitpoints) * IF_REMIX_STAMINA_DARKNESS_MAX) + 0.5F);

    return darkness / IF_REMIX_STAMINA_DARKNESS_MAX;
}

#endif
