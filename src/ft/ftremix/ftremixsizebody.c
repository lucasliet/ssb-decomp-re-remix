/**
 * ftremixsizebody.c - Smash Remix body size, phase (b) helpers of Size.asm.
 *
 * Scales the size of attack hitboxes, the damage of attack hitboxes and the passive knockback
 * resist by the per-player size multiplier. Every helper returns its input unchanged while the
 * multiplier is 1.0F, so a fighter of standard size behaves like vanilla.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>

/* Size.asm fix_damage_ (RAM 0x800DF34C): the damage factors are lui-only constants (asm lines 541-542),
 * so the exact values are 0x3F330000 and 0x3FA60000, not the rounded 0.7 and 1.3. */
#define FTREMIX_SIZE_DAMAGE_POISON_FACTOR 0.69921875F
#define FTREMIX_SIZE_DAMAGE_GIANT_FACTOR 1.296875F

/* Size.asm passive_armor_ (RAM 0x800E6324): the passive armor of a fighter above size 1, the same as GDK. */
#define FTREMIX_SIZE_PASSIVE_ARMOR_GIANT 48.0F

/**
 * Hitbox size after the size multiplier, truncated to an integer (Size.asm adjust_hitbox, trunc.w.s).
 * The caller halves the result, as the vanilla hitbox size does.
 */
s32 ftRemixSizeScaleHitboxSize(FTStruct *fp, s32 size)
{
    return (s32)((f32)size * ftRemixSizeGetMultiplier(fp->player));
}

/**
 * Rounds a damage product to the nearest integer with ties to even, as cvt.w.s does in the default rounding mode.
 */
s32 ftRemixRoundDamage(f32 scaled)
{
    f32 fraction;
    s32 whole;

    whole = (s32)scaled;
    fraction = scaled - (f32)whole;

    if ((fraction > 0.5F) || ((fraction == 0.5F) && ((whole & 1) != 0)))
    {
        whole++;
    }
    return whole;
}

/**
 * Damage of an attack hitbox for a fighter of another size (Size.asm fix_damage_, RAM 0x800DF34C).
 * Above size 1 the damage is multiplied by the giant factor, below size 1 by the poison factor, and the
 * product is rounded to the nearest integer with ties to even (cvt.w.s, default rounding mode).
 * The size multiplier itself is only used for the comparison, not as the factor.
 */
s32 ftRemixSizeScaleDamage(FTStruct *fp, s32 damage)
{
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);
    f32 factor;

    if (multiplier == 1.0F)
    {
        return damage;
    }
    factor = (multiplier < 1.0F) ? FTREMIX_SIZE_DAMAGE_POISON_FACTOR : FTREMIX_SIZE_DAMAGE_GIANT_FACTOR;
    return ftRemixRoundDamage((f32)damage * factor);
}

/**
 * Passive knockback resist of a fighter, plus 48 above size 1 (Size.asm passive_armor_, RAM 0x800E6324).
 * The stamina armor fix (Stamina.asm stamina_armor_fix_, called first by the asm) returns no armor for a fighter out of HP.
 */
f32 ftRemixSizePassiveArmor(FTStruct *fp, f32 passive_resist)
{
    if (ftRemixStaminaArmorIsZeroed(fp))
    {
        return 0.0F;
    }
    if (ftRemixSizeGetMultiplier(fp->player) <= 1.0F)
    {
        return passive_resist;
    }
    return passive_resist + FTREMIX_SIZE_PASSIVE_ARMOR_GIANT;
}

#endif
