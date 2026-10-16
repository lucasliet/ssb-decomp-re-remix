/**
 * ftremixsizeentry.c - Smash Remix body size subsystem, phase (c) of Size.asm: entry animation and Fox Arwing.
 *
 * The entry animation scales the translation of the trans joint for the Mario family and Conker
 * (Size.asm animation_, 0xB856C). The Arwing entry effect scales its wing joint each frame by the
 * multiplier of the fighter that owns it (Size.asm arwing update_routine_). Both are no-ops at 1.0F.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_roster_data.h"
#include <ef/effect.h>
#include <ft/fighter.h>

/** TRUE for the fighter kinds that Size.asm animation_ scales (Mario, Luigi, Dr. Mario, J Mario, Metal, Conker...). */
static sb32 ftRemixSizeEntryIsScaledAnimation(s32 fkind)
{
    switch (fkind)
    {
    case nFTKindMario:
    case nFTKindLuigi:
    case REMIX_ID_DRM:
    case REMIX_ID_DRL:
    case REMIX_ID_JMARIO:
    case REMIX_ID_JLUIGI:
    case REMIX_ID_METAL:
    case REMIX_ID_MLUIGI:
    case REMIX_ID_CONKER:
        return TRUE;

    default:
        return FALSE;
    }
}

/**
 * Scales the translation of the trans joint by the fighter size multiplier, before the entry
 * animation reads it. Called at the top of ftCommonAppearProcPhysics.
 */
void ftRemixSizeEntryAnimationScale(FTStruct *fp)
{
    DObj *transn_joint;
    f32 multiplier;

    if (!ftRemixSizeEntryIsScaledAnimation(fp->fkind))
    {
        return;
    }
    transn_joint = fp->joints[nFTPartsJointTransN];
    multiplier = ftRemixSizeGetMultiplier(fp->player);

    transn_joint->translate.vec.f.x *= multiplier;
    transn_joint->translate.vec.f.y *= multiplier;
    transn_joint->translate.vec.f.z *= multiplier;
}

/**
 * Stores the fighter that owns an Arwing entry effect, so its update can read the fighter size.
 * Called with the effect returned by efManagerFoxEntryArwingMakeEffect.
 */
void ftRemixSizeEntryArwingBind(GObj *effect_gobj, GObj *fighter_gobj)
{
    if (effect_gobj == NULL)
    {
        return;
    }
    efGetStruct(effect_gobj)->fighter_gobj = fighter_gobj;
}

/**
 * Scales the wing joint of an Arwing entry effect by the owning fighter size multiplier.
 * Called every frame from efManagerFoxEntryArwingProcUpdate. Does nothing for effects with no owner.
 */
void ftRemixSizeEntryArwingUpdate(GObj *effect_gobj)
{
    GObj *fighter_gobj = efGetStruct(effect_gobj)->fighter_gobj;
    DObj *wing_joint;
    f32 multiplier;

    if (fighter_gobj == NULL)
    {
        return;
    }
    wing_joint = DObjGetStruct(effect_gobj)->child;
    multiplier = ftRemixSizeGetMultiplier(ftGetStruct(fighter_gobj)->player);

    wing_joint->scale.vec.f.x *= multiplier;
    wing_joint->scale.vec.f.y *= multiplier;
    wing_joint->scale.vec.f.z *= multiplier;
}

#endif
