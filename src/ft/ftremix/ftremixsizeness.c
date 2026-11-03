/**
 * ftremixsizeness.c - Smash Remix body size for Ness's DSP and USP gfx (Size.asm ness.dsp and ness.usp).
 *
 * The update wrapper runs the vanilla animation update (gcPlayAnimAll, asm 0x8000DF34) and then scales the
 * animated joints by the owner multiplier. The init routine runs once when the gfx is created.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixsizeness.h"
#include <ef/efmanager.h>
#include <ft/fighter.h>
#include <lb/lbtypes.h>
#include <sys/objanim.h>

/** Owner multiplier of the player that created the gfx. */
static f32 ftRemixSizeNessOwnerMultiplier(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);

    return ftRemixSizeGetMultiplier(ftGetStruct(ep->fighter_gobj)->player);
}

/** Multiplies the three scale axes of a joint by the multiplier. */
static void ftRemixSizeNessScaleJoint(DObj *joint, f32 multiplier)
{
    joint->scale.vec.f.x *= multiplier;
    joint->scale.vec.f.y *= multiplier;
    joint->scale.vec.f.z *= multiplier;
}

/**
 * Size.asm ness.dsp _scale (RAM 0x8010260C): scales the 2nd joint of the DSP gfx.
 * The joint offsets (x and y) are scaled only when is_init is TRUE, as the asm does at creation.
 */
static void ftRemixSizeNessDSPScale(GObj *effect_gobj, sb32 is_init)
{
    f32 multiplier = ftRemixSizeNessOwnerMultiplier(effect_gobj);
    DObj *second_joint = DObjGetStruct(effect_gobj)->child;

    ftRemixSizeNessScaleJoint(second_joint, multiplier);
    if (!is_init)
    {
        return;
    }
    second_joint->translate.vec.f.x *= multiplier;
    second_joint->translate.vec.f.y *= multiplier;
}

/**
 * Size.asm ness.usp _scale (RAM 0x80102A3C): scales the 3rd joint of the USP gfx and the y and z offsets of the 2nd joint.
 */
static void ftRemixSizeNessUSPScale(GObj *effect_gobj)
{
    f32 multiplier = ftRemixSizeNessOwnerMultiplier(effect_gobj);
    DObj *second_joint = DObjGetStruct(effect_gobj)->child;
    DObj *third_joint = second_joint->child;

    ftRemixSizeNessScaleJoint(third_joint, multiplier);
    second_joint->translate.vec.f.y *= multiplier;
    second_joint->translate.vec.f.z *= multiplier;
}

/** Size.asm ness.dsp _initialize: scales the DSP gfx once when it is created, offsets included. */
void ftRemixSizeNessDSPInitialize(GObj *effect_gobj)
{
    ftRemixSizeNessDSPScale(effect_gobj, TRUE);
}

/** Size.asm ness.dsp update wrapper: the vanilla animation update, then the DSP scale on every frame. */
void ftRemixSizeNessDSPProcUpdate(GObj *effect_gobj)
{
    gcPlayAnimAll(effect_gobj);
    ftRemixSizeNessDSPScale(effect_gobj, FALSE);
}

/** Size.asm ness.usp _initialize: scales the USP gfx once when it is created. */
void ftRemixSizeNessUSPInitialize(GObj *effect_gobj)
{
    ftRemixSizeNessUSPScale(effect_gobj);
}

/** Size.asm ness.usp update wrapper: the vanilla animation update, then the USP scale on every frame. */
void ftRemixSizeNessUSPProcUpdate(GObj *effect_gobj)
{
    gcPlayAnimAll(effect_gobj);
    ftRemixSizeNessUSPScale(effect_gobj);
}

#endif
