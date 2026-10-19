/**
 * ftremixshieldstun.c - Smash Remix Japanese.asm japanese_shieldstun_: the Shield Stun toggle sets the shieldstun
 * formula of ftCommonGuardSetOffSetStatus (0x80149108).
 *
 * The formula is damage * multiplier + constant. The hook replaces both values, the vanilla ones are kept when the
 * toggle is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_SHIELDSTUN_JAPANESE 1
#define FTREMIX_SHIELDSTUN_MELEE 2
#define FTREMIX_SHIELDSTUN_BRAWL 3
#define FTREMIX_SHIELDSTUN_ULTIMATE 4

/* Values loaded by Japanese.asm; the decimal forms are exact (0x3FE00000, 0x3EE70000, 0x3EB10000, 0x3F3A0000) */
#define FTREMIX_SHIELDSTUN_JAPANESE_MUL 1.75F
#define FTREMIX_SHIELDSTUN_JAPANESE_ADD 3.0F
#define FTREMIX_SHIELDSTUN_MELEE_MUL 0.451171875F
#define FTREMIX_SHIELDSTUN_MELEE_ADD 1.9921875F
#define FTREMIX_SHIELDSTUN_BRAWL_MUL 0.345703125F
#define FTREMIX_SHIELDSTUN_BRAWL_ADD 0.0F
#define FTREMIX_SHIELDSTUN_ULTIMATE_MUL 0.7265625F
#define FTREMIX_SHIELDSTUN_ULTIMATE_ADD 2.0F

/**
 * Returns the shieldstun frames of a shield break for the Shield Stun toggle (Japanese.asm). vanilla_frames is
 * the value of the vanilla formula, returned unchanged for the normal toggle value and for the values the
 * toggle does not define. Bonus and 1P Game ignore the toggle (gmRemixToggleVS).
 */
f32 ftRemixShieldStunFrames(FTStruct *fp, f32 vanilla_frames)
{
    f32 mul;
    f32 add;

    switch (gmRemixToggleVS(shieldstun))
    {
        case FTREMIX_SHIELDSTUN_JAPANESE:
            mul = FTREMIX_SHIELDSTUN_JAPANESE_MUL;
            add = FTREMIX_SHIELDSTUN_JAPANESE_ADD;
            break;

        case FTREMIX_SHIELDSTUN_MELEE:
            mul = FTREMIX_SHIELDSTUN_MELEE_MUL;
            add = FTREMIX_SHIELDSTUN_MELEE_ADD;
            break;

        case FTREMIX_SHIELDSTUN_BRAWL:
            mul = FTREMIX_SHIELDSTUN_BRAWL_MUL;
            add = FTREMIX_SHIELDSTUN_BRAWL_ADD;
            break;

        case FTREMIX_SHIELDSTUN_ULTIMATE:
            mul = FTREMIX_SHIELDSTUN_ULTIMATE_MUL;
            add = FTREMIX_SHIELDSTUN_ULTIMATE_ADD;
            break;

        default:
            return vanilla_frames;
    }
    return (fp->shield_damage * mul) + add;
}

#endif
