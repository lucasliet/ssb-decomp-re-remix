/**
 * efremixpuffsing.c - Jigglypuff sing effect of Smash Remix (Size.asm `scope jigglypuff`, Toggles.asm entry_puff_sing_anim).
 *
 * Size.asm replaces the update routine of dEFManagerPurinSingEffectDesc (descriptor 0x8012E314 + 0x10) with a wrapper that
 * runs the vanilla update, forces the matrix kind of the 2nd joint to 0x2E (rotation and scale respected) and, unless the
 * Jigglypuff Sing GFX Anims setting is on, resets the scale of that joint to 1.0. The Gameshark check of the asm (byte
 * 0x8010212B, the 0x46 kind written by efManagerPurinSingMakeEffect) has no equivalent in the port: it always holds 0x46.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ef/efremixpuffsing.h"
#include <ef/effect.h>
#include <ef/efmanager.h>
#include <gm/gmremixtoggle.h>
#include <sys/obj.h>
#include <sys/objanim.h>
#include <sys/objman.h>

/* asm: lli t8, 0x2E, the routine index that respects rotation and scale (nGCMatrixKind46) */
#define EFREMIX_PURIN_SING_MATRIX_KIND 0x2E

/** Sets the matrix kind of the first matrix of the joint, as the asm does with the byte at 0x0004 of the struct at 0x005C. */
static void efRemixPurinSingForceMatrixKind(DObj *joint)
{
    if (joint->xobjs_num == 0)
    {
        return;
    }
    joint->xobjs[0]->kind = EFREMIX_PURIN_SING_MATRIX_KIND;
}

/** Resets the scale of the joint to 1.0 on the three axes (asm: 0x3F800000 stored at 0x40, 0x44 and 0x48). */
static void efRemixPurinSingResetScale(DObj *joint)
{
    joint->scale.vec.f.x = 1.0F;
    joint->scale.vec.f.y = 1.0F;
    joint->scale.vec.f.z = 1.0F;
}

/**
 * Update proc of the Jigglypuff sing effect (asm update_routine_, vanilla update 0x800FD568).
 * Runs efManagerHaveStructProcUpdate inline, so the effect is not touched after the vanilla update ejects it, then forces
 * the matrix kind of the 2nd joint and resets its scale unless the Sing GFX Anims setting is on.
 *
 * @param effect_gobj Sing effect GObj
 */
void efRemixPurinSingProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    DObj *second_joint;

    if (!(ep->is_pause_effect))
    {
        gcPlayAnimAll(effect_gobj);

        if (effect_gobj->anim_frame <= 0.0F)
        {
            efManagerSetPrevStructAlloc(ep);
            gcEjectGObj(effect_gobj);
            return;
        }
    }
    second_joint = DObjGetStruct(effect_gobj)->child;

    efRemixPurinSingForceMatrixKind(second_joint);

    if (gmRemixToggle(puff_sing_anim) != 0)
    {
        return;
    }
    efRemixPurinSingResetScale(second_joint);
}

#endif
