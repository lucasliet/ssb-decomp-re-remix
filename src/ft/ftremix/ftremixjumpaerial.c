/**
 * ftremixjumpaerial.c - Remix double jump patches that vanilla ftCommonJumpAerialSetStatus calls: the Yoshi double jump of
 * Yoshi clones (yoshishared.asm yoshi_dj_fix_1, JYoshi.asm joshi_armor) and the Ness double jump of Character.ness_jump
 * (nessshared.asm ness_jump1_/ness_jump2_). Pure rules live in ftremixjumpaerialrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixjumpaerialrules.h"

/** TRUE when the fighter double jumps with the Yoshi physics (Yoshi, Polygon Yoshi, J Yoshi). */
sb32 ftRemixJumpAerialUsesYoshiPhysics(s32 fkind)
{
    return ftRemixJumpAerialRulesUsesYoshiPhysics(fkind);
}

/** Knockback resist of the Yoshi double jump; J Yoshi has 110 instead of the vanilla value. */
f32 ftRemixJumpAerialYoshiKnockbackResist(s32 fkind, f32 vanilla_resist)
{
    return ftRemixJumpAerialRulesYoshiKnockbackResist(fkind, vanilla_resist);
}

/** TRUE when the fighter double jumps with the Ness physics (Character.ness_jump). */
sb32 ftRemixJumpAerialUsesNessPhysics(s32 fkind)
{
    return ftRemixJumpAerialRulesUsesNessPhysics(fkind);
}

#endif
