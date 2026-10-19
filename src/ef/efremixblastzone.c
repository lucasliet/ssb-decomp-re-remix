/**
 * efremixblastzone.c - BlastZone GFX setting of Smash Remix (Toggles.asm entry_blastzone_gfx).
 *
 * Accessibility.asm blastzone_explode_gfx patches efManagerDeadExplodeMakeEffect: OFF skips the explosion, and REDUCED scales
 * its envelope effect to 0.64 and its star particles to 0.75. DEFAULT leaves the vanilla explosion.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <lb/lbtypes.h>
#include <sys/obj.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"

#define EFREMIX_BLASTZONE_GFX_OFF 1                 /* Toggles.asm entry_blastzone_gfx: OFF */
#define EFREMIX_BLASTZONE_GFX_REDUCED 2             /* Toggles.asm entry_blastzone_gfx: REDUCED */
#define EFREMIX_BLASTZONE_EFFECT_SCALE 0.64F        /* Accessibility.asm: scale = 0.64 (0x3F23D70A) */
#define EFREMIX_BLASTZONE_PARTICLE_SCALE 0.75F      /* Accessibility.asm: scale = 0.75 (0x3F400000) */

/** Returns TRUE when the BlastZone GFX setting is OFF, so the explosion is not made. */
sb32 efRemixBlastZoneGfxIsOff(void)
{
    return gmRemixToggle(blastzone_gfx) == EFREMIX_BLASTZONE_GFX_OFF;
}

/** Scales the envelope effect of the explosion (its root DObj) when the BlastZone GFX setting is REDUCED. */
void efRemixBlastZoneScaleEffect(DObj *dobj)
{
    if (gmRemixToggle(blastzone_gfx) != EFREMIX_BLASTZONE_GFX_REDUCED)
    {
        return;
    }
    dobj->scale.vec.f.x = EFREMIX_BLASTZONE_EFFECT_SCALE;
    dobj->scale.vec.f.y = EFREMIX_BLASTZONE_EFFECT_SCALE;
}

/** Scales a star particle of the explosion when the BlastZone GFX setting is REDUCED. */
void efRemixBlastZoneScaleParticle(LBTransform *xf)
{
    if (gmRemixToggle(blastzone_gfx) != EFREMIX_BLASTZONE_GFX_REDUCED)
    {
        return;
    }
    xf->scale.x = EFREMIX_BLASTZONE_PARTICLE_SCALE;
    xf->scale.y = EFREMIX_BLASTZONE_PARTICLE_SCALE;
}

#endif
