/**
 * ftremixsizemovefx.c - Smash Remix body size subsystem, phase (c) of Size.asm: effect gfx of Samus's gun blast, Kirby's
 * Final Cutter and Falcon's kick.
 *
 * Size.asm replaces the render or update routine of these effect descriptors with a wrapper that scales the joints of the
 * gfx by the multiplier of the owner, then runs the vanilla routine. The owner is the fighter stored in the EFStruct
 * (the asm reads it from the projectile special struct, offset 0x0004). An effect with no owner keeps the vanilla scale.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ef/efmanager.h>
#include <ft/fighter.h>
#include <lb/lbcommon.h>
#include <sys/objdisplay.h>

/* Size.asm samus_gun_blast render_routine_ (lui 0x4320): the y offset of the gfx before scaling. */
#define FTREMIX_SIZE_GUN_BLAST_Y 160.0F

/* Size.asm kirby.usp.render_routine_descend_ (0x358637BD, lui 0x4396, 0xC170000D): the 2nd joint offset before scaling. */
#define FTREMIX_SIZE_CUTTER_TRAIL_X 9.999999974752427e-07F
#define FTREMIX_SIZE_CUTTER_TRAIL_Y 300.0F
#define FTREMIX_SIZE_CUTTER_TRAIL_Z (-15.000012397766113F)

/* Size.asm falcon.kick.update_routine_ (0x43A20002): the z offset of the 2nd joint before scaling. */
#define FTREMIX_SIZE_FALCON_KICK_Z 324.00006103515625F

/** Sets the three scale axes of a joint to the multiplier. */
static void ftRemixSizeMoveFxSetScale(DObj *joint, f32 multiplier)
{
    joint->scale.vec.f.x = multiplier;
    joint->scale.vec.f.y = multiplier;
    joint->scale.vec.f.z = multiplier;
}

/** Multiplier of the fighter stored in the effect, or 1.0F with FALSE in has_owner when the effect has none. */
static f32 ftRemixSizeMoveFxGetOwnerMultiplier(GObj *effect_gobj, sb32 *has_owner)
{
    EFStruct *ep = efGetStruct(effect_gobj);

    *has_owner = (ep->fighter_gobj != NULL);

    if (ep->fighter_gobj == NULL)
    {
        return 1.0F;
    }
    return ftRemixSizeGetMultiplier(ftGetStruct(ep->fighter_gobj)->player);
}

/**
 * Size.asm samus_gun_blast render_routine_ (descriptor 0x8012E0B4, vanilla render 0x800CB4B0): scales the 2nd joint and sets
 * the y offset of the top joint to 160.0F times the multiplier, then runs lbCommonDObjScaleXProcDisplay.
 */
void ftRemixSizeFireSparkProcDisplay(GObj *effect_gobj)
{
    DObj *top_joint = DObjGetStruct(effect_gobj);
    sb32 has_owner;
    f32 multiplier = ftRemixSizeMoveFxGetOwnerMultiplier(effect_gobj, &has_owner);

    if (has_owner)
    {
        ftRemixSizeMoveFxSetScale(top_joint->child, multiplier);
        top_joint->translate.vec.f.y = FTREMIX_SIZE_GUN_BLAST_Y * multiplier;
    }
    lbCommonDObjScaleXProcDisplay(effect_gobj);
}

/**
 * Size.asm kirby.usp.render_routine_ (descriptors 0x8012E37C and 0x8012E3A4, vanilla render 0x80014038): scales the 2nd joint
 * of the sword twirl gfx, then runs gcDrawDObjTreeForGObj.
 */
void ftRemixSizeKirbyCutterProcDisplay(GObj *effect_gobj)
{
    sb32 has_owner;
    f32 multiplier = ftRemixSizeMoveFxGetOwnerMultiplier(effect_gobj, &has_owner);

    if (has_owner)
    {
        ftRemixSizeMoveFxSetScale(DObjGetStruct(effect_gobj)->child, multiplier);
    }
    gcDrawDObjTreeForGObj(effect_gobj);
}

/**
 * Size.asm kirby.usp.render_routine_descend_ (descriptor 0x8012E3F4, vanilla render 0x80014768): scales the 2nd joint and its
 * offsets of the descending sword trail, then runs gcDrawDObjTreeDLLinksForGObj.
 */
void ftRemixSizeKirbyCutterTrailProcDisplay(GObj *effect_gobj)
{
    sb32 has_owner;
    f32 multiplier = ftRemixSizeMoveFxGetOwnerMultiplier(effect_gobj, &has_owner);
    DObj *second_joint;

    if (has_owner)
    {
        second_joint = DObjGetStruct(effect_gobj)->child;

        ftRemixSizeMoveFxSetScale(second_joint, multiplier);
        second_joint->translate.vec.f.x = FTREMIX_SIZE_CUTTER_TRAIL_X * multiplier;
        second_joint->translate.vec.f.y = FTREMIX_SIZE_CUTTER_TRAIL_Y * multiplier;
        second_joint->translate.vec.f.z = FTREMIX_SIZE_CUTTER_TRAIL_Z * multiplier;
    }
    gcDrawDObjTreeDLLinksForGObj(effect_gobj);
}

/**
 * Size.asm falcon.kick.update_routine_ (descriptor 0x8012E2C4, vanilla update 0x800FD5D8, also installed for the Falcon clones
 * by captainshared.asm): after the vanilla update, multiplies the scale of the 2nd joint by the multiplier and sets its z
 * offset to 324.00006F times the multiplier. Skipped while the effect is paused (the asm tests the hitlag bit of the
 * projectile struct word at 0x0010, which is is_pause_effect).
 */
void ftRemixSizeFalconKickProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    DObj *second_joint;
    f32 multiplier;

    efManagerNoEjectProcUpdate(effect_gobj);

    if (ep->is_pause_effect || (ep->fighter_gobj == NULL))
    {
        return;
    }
    multiplier = ftRemixSizeGetMultiplier(ftGetStruct(ep->fighter_gobj)->player);
    second_joint = DObjGetStruct(effect_gobj)->child;

    second_joint->scale.vec.f.x *= multiplier;
    second_joint->scale.vec.f.y *= multiplier;
    second_joint->scale.vec.f.z *= multiplier;
    second_joint->translate.vec.f.z = FTREMIX_SIZE_FALCON_KICK_Z * multiplier;
}

#endif
