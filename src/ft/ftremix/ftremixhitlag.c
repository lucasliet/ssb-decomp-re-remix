/**
 * ftremixhitlag.c - Smash Remix Japanese.asm hitlag_: the Hitlag toggle sets the hitlag formula.
 *
 * The hook replaces the constant of ftParamGetHitLag (0x800EA1C0) with the one of the toggle; the
 * vanilla constant is kept when the toggle is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_HITLAG_NORMAL 0
#define FTREMIX_HITLAG_JAPANESE 1
#define FTREMIX_HITLAG_MELEE 2
#define FTREMIX_HITLAG_ULTIMATE 3
#define FTREMIX_HITLAG_NONE 4

/* 0.65234375F (0x3F270000) in Japanese.asm, the Ultimate damage multiplier */
#define FTREMIX_HITLAG_ULTIMATE_DAMAGE_MUL 0.65234375F

/**
 * Returns the hitlag tics of a hit for the Hitlag toggle (Japanese.asm). vanilla_add is the constant of the
 * vanilla formula (damage * 1/3 + vanilla_add), kept for the normal and the out-of-range values.
 * Bonus and 1P Game ignore the toggle (gmRemixToggleVS).
 */
s32 ftRemixHitlagTics(s32 damage, f32 hitlag_mul, f32 vanilla_add)
{
    f32 damage_mul = 1.0F / 3.0F;
    f32 hitlag_add = vanilla_add;

    switch (gmRemixToggleVS(hitlag))
    {
        case FTREMIX_HITLAG_JAPANESE:
            hitlag_add = 4.0F;
            break;

        case FTREMIX_HITLAG_MELEE:
            hitlag_add = 3.0F;
            break;

        case FTREMIX_HITLAG_ULTIMATE:
            damage_mul = FTREMIX_HITLAG_ULTIMATE_DAMAGE_MUL;
            hitlag_add = 6.0F;
            break;

        case FTREMIX_HITLAG_NONE:
            damage_mul = 0.0F;
            hitlag_add = 0.0F;
            break;

        default:
            break;
    }
    return (s32) ((damage * damage_mul) + hitlag_add) * hitlag_mul;
}

#endif
