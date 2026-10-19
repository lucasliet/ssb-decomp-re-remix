/**
 * ftremixdi.c - Smash Remix DI.asm: the DI toggle sets the smash DI range multiplier.
 *
 * apply_di_modifiers (0x801408DC) replaces the vanilla load of the multiplier in
 * ftCommonDamageCommonProcLagUpdate (ftcommondamage.c) and calls into this file.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_DI_JAPANESE 1
#define FTREMIX_DI_ULTIMATE 2

/* 0x3FC00000 in DI.asm (Japanese coding) */
#define FTREMIX_DI_JAPANESE_MUL 1.5F

/* 0x3F190000 in DI.asm (Ultimate coding, weaker (S)DI); the low 16 bits are zero, so the value is exact */
#define FTREMIX_DI_ULTIMATE_MUL 0.59765625F

/**
 * Returns the smash DI range multiplier for the DI toggle (DI.asm). Normal DI, and any value the toggle
 * does not define, keep vanilla_mul. Bonus and 1P Game ignore the toggle (gmRemixToggleVS).
 */
f32 ftRemixDamageSmashDIRangeMul(f32 vanilla_mul)
{
    switch (gmRemixToggleVS(di))
    {
        case FTREMIX_DI_JAPANESE:
            return FTREMIX_DI_JAPANESE_MUL;

        case FTREMIX_DI_ULTIMATE:
            return FTREMIX_DI_ULTIMATE_MUL;

        default:
            return vanilla_mul;
    }
}

#endif
