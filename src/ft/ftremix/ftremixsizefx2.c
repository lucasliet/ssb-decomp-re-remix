/**
 * ftremixsizefx2.c - Smash Remix body size subsystem, phase (c) of Size.asm: effect scale of Link's Up Special.
 *
 * The Remix wrappers rewrite the scale of the weapon and the effect on every render. The port sets them once,
 * when the object is spawned, which gives the same result while the owner's multiplier does not change.
 * A multiplier of 1.0F returns before any write, so the vanilla objects are not touched.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <sys/objdisplay.h>

/* Size.asm link.adjust_usp_gfx_size_ (lui 0x4396 = 300.0F): the effect 2nd joint height before scaling. */
#define FTREMIX_SIZE_LINK_EFFECT_Y 300.0F

/**
 * Scales the Up Special weapon top joint by the owner multiplier (Size.asm link.adjust_usp_projectile_size_).
 */
void ftRemixSizeScaleLinkWeapon(FTStruct *fp, GObj *weapon_gobj)
{
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);
    DObj *top_joint;

    if (multiplier == 1.0F)
    {
        return;
    }
    top_joint = DObjGetStruct(weapon_gobj);
    top_joint->scale.vec.f.x = multiplier;
    top_joint->scale.vec.f.y = multiplier;
    top_joint->scale.vec.f.z = multiplier;
}

/**
 * Scales the Up Special effect 2nd joint and sets its height to 300.0F times the owner multiplier
 * (Size.asm link.adjust_usp_gfx_size_). effect_gobj must not be NULL.
 */
void ftRemixSizeScaleLinkEffect(FTStruct *fp, GObj *effect_gobj)
{
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);
    DObj *second_joint;

    if (multiplier == 1.0F)
    {
        return;
    }
    second_joint = DObjGetStruct(effect_gobj)->child;
    second_joint->scale.vec.f.x = multiplier;
    second_joint->scale.vec.f.y = multiplier;
    second_joint->scale.vec.f.z = multiplier;
    second_joint->translate.vec.f.y = FTREMIX_SIZE_LINK_EFFECT_Y * multiplier;
}

#endif
