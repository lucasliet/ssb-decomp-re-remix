/**
 * ftremixsizeentrygfx.c - Smash Remix body size for the entry gfx, phase (c) of Size.asm scope entry.gfx_.
 *
 * The asm hooks the vanilla entry gfx makers (Mario pipe, DK barrel, Samus portal, Link ground wave and shaft, Kirby star,
 * Captain car) and scales the joints of the gfx object by the multiplier of the entering fighter. The port calls these
 * helpers right after the maker (or, for the effects whose update is wrapped, instead of the maker), once per entry, with
 * the entering FTStruct. Every effect accepts a NULL gfx object, which the makers return when no effect slot is free.
 *
 * The Arwing is scaled by ftremixsizeentry.c, the Yoshi egg by ftRemixSizeScaleEntryEgg.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_roster_data.h"
#include <ef/efdef.h>
#include <ef/efmanager.h>
#include <ft/fighter.h>
#include <sys/objanim.h>
#include <sys/objman.h>

/* Size.asm entry.gfx_._blue_falcon (lui 0x4410): the Bowser clown car height before scaling. */
#define FTREMIX_SIZE_ENTRY_CAR_BOWSER_HEIGHT 576.0F

extern EFDesc dEFManagerLinkEntryWaveEffectDesc;
extern EFDesc dEFManagerLinkEntryBeamEffectDesc;

/** Multiplies the three scale axes of a joint by the multiplier. */
static void ftRemixSizeEntryMulJointScale(DObj *joint, f32 multiplier)
{
    joint->scale.vec.f.x *= multiplier;
    joint->scale.vec.f.y *= multiplier;
    joint->scale.vec.f.z *= multiplier;
}

/** Sets the three scale axes of a joint to the multiplier. */
static void ftRemixSizeEntrySetJointScale(DObj *joint, f32 multiplier)
{
    joint->scale.vec.f.x = multiplier;
    joint->scale.vec.f.y = multiplier;
    joint->scale.vec.f.z = multiplier;
}

/** Multiplier of the fighter that owns a wrapped entry gfx, stored by the maker in EFStruct fighter_gobj. */
static f32 ftRemixSizeEntryOwnerMultiplier(GObj *effect_gobj)
{
    return ftRemixSizeGetMultiplier(ftGetStruct(efGetStruct(effect_gobj)->fighter_gobj)->player);
}

/**
 * Update of the wrapped entry gfx, Size.asm entry.gfx_._link_ground_gfx update_routine_: the vanilla update 0x800FD524
 * (efManagerNoStructProcUpdate), then the three scale axes of the top joint times the multiplier, and for Banjo the z
 * offset of the top joint as well. The EFStruct stands for the asm special struct stored in the gfx object.
 */
static void ftRemixSizeEntryGroundGfxProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    FTStruct *fp = ftGetStruct(ep->fighter_gobj);
    DObj *top_joint = DObjGetStruct(effect_gobj);
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);

    gcPlayAnimAll(effect_gobj);

    if (effect_gobj->anim_frame <= 0.0F)
    {
        efManagerSetPrevStructAlloc(ep);
        gcEjectGObj(effect_gobj);

        return;
    }
    if (fp->fkind == REMIX_ID_BANJO)
    {
        top_joint->translate.vec.f.z *= multiplier;
    }
    ftRemixSizeEntryMulJointScale(top_joint, multiplier);
}

/**
 * Update of the Goemon and Ebisumaru entry beam, Size.asm entry.gfx_.scaled_render_routine_: the x and z scale of the top
 * joint times the multiplier after the vanilla update. The asm applies it in the render routine, the port in the update
 * so that a render that runs twice in a frame does not scale twice.
 */
static void ftRemixSizeEntryBeamProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    DObj *top_joint = DObjGetStruct(effect_gobj);
    f32 multiplier = ftRemixSizeEntryOwnerMultiplier(effect_gobj);

    gcPlayAnimAll(effect_gobj);

    if (effect_gobj->anim_frame <= 0.0F)
    {
        efManagerSetPrevStructAlloc(ep);
        gcEjectGObj(effect_gobj);

        return;
    }
    top_joint->scale.vec.f.x *= multiplier;
    top_joint->scale.vec.f.z *= multiplier;
}

/**
 * Makes an entry gfx whose effect desc has an EFStruct and a Remix update, and binds the entering fighter to it.
 * make_effect reads desc, so the desc is changed only for the duration of the call. Returns NULL when no effect is made.
 */
static GObj* ftRemixSizeEntryMakeWrapped(EFDesc *desc, GObj* (*make_effect)(Vec3f*), FTStruct *fp, void (*proc_update)(GObj*))
{
    EFDesc saved = *desc;
    GObj *effect_gobj;

    desc->flags |= EFFECT_FLAG_USERDATA;
    desc->proc_update = proc_update;
    effect_gobj = make_effect(&fp->entry_pos);
    *desc = saved;

    if (effect_gobj == NULL)
    {
        return NULL;
    }
    efGetStruct(effect_gobj)->fighter_gobj = fp->fighter_gobj;

    return effect_gobj;
}

/** Size.asm entry.gfx_._top_joint: the three scale axes of the top joint become the multiplier. Pipe, barrel and portal. */
void ftRemixSizeEntryTopJoint(GObj *effect_gobj, FTStruct *fp)
{
    if (effect_gobj == NULL)
    {
        return;
    }
    ftRemixSizeEntrySetJointScale(DObjGetStruct(effect_gobj), ftRemixSizeGetMultiplier(fp->player));
}

/** Size.asm entry.gfx_._2nd_joint: the three scale axes of the 2nd joint become the multiplier. Kirby star and Banjo shaft. */
void ftRemixSizeEntrySecondJoint(GObj *effect_gobj, FTStruct *fp)
{
    if (effect_gobj == NULL)
    {
        return;
    }
    ftRemixSizeEntrySetJointScale(DObjGetStruct(effect_gobj)->child, ftRemixSizeGetMultiplier(fp->player));
}

/**
 * Size.asm entry.gfx_._link_ground_gfx: makes the Link ground wave with the wrapped update. Banjo does not take the z
 * offset from the entry position (the asm skips the original store), so its top joint keeps the z of the first DObjDesc
 * of the effect desc.
 */
GObj* ftRemixSizeEntryMakeLinkWave(FTStruct *fp)
{
    GObj *effect_gobj = ftRemixSizeEntryMakeWrapped(&dEFManagerLinkEntryWaveEffectDesc, efManagerLinkEntryWaveMakeEffect, fp,
                                                    ftRemixSizeEntryGroundGfxProcUpdate);
    const DObjDesc *top_desc;

    if ((effect_gobj == NULL) || (fp->fkind != REMIX_ID_BANJO))
    {
        return effect_gobj;
    }
    top_desc = (const DObjDesc*)((uintptr_t)*dEFManagerLinkEntryWaveEffectDesc.file_head + dEFManagerLinkEntryWaveEffectDesc.o_dobjsetup);
    DObjGetStruct(effect_gobj)->translate.vec.f.z = top_desc->translate.z;

    return effect_gobj;
}

/** Size.asm entry.gfx_._top_joint_no_y: x and z scale of the top joint become the multiplier, y is left. Link family shaft. */
static void ftRemixSizeEntryTopJointNoY(GObj *effect_gobj, FTStruct *fp)
{
    DObj *top_joint = DObjGetStruct(effect_gobj);
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);

    top_joint->scale.vec.f.x = multiplier;
    top_joint->scale.vec.f.z = multiplier;
}

/**
 * Size.asm entry.gfx_._entry_shaft: makes the Link shaft (beam) and scales it by the fighter. The Link family scales x and z
 * of the top joint, Marina uses the wrapped update of the ground wave, Goemon and Ebisumaru the wrapped beam update, Banjo
 * the 2nd joint, and every other user (Marth, Roy...) the top joint.
 */
GObj* ftRemixSizeEntryMakeLinkBeam(FTStruct *fp)
{
    GObj *effect_gobj;

    switch (fp->fkind)
    {
    case REMIX_ID_MARINA:
        return ftRemixSizeEntryMakeWrapped(&dEFManagerLinkEntryBeamEffectDesc, efManagerLinkEntryBeamMakeEffect, fp,
                                           ftRemixSizeEntryGroundGfxProcUpdate);
    case REMIX_ID_GOEMON:
    case REMIX_ID_EBI:
        return ftRemixSizeEntryMakeWrapped(&dEFManagerLinkEntryBeamEffectDesc, efManagerLinkEntryBeamMakeEffect, fp,
                                           ftRemixSizeEntryBeamProcUpdate);
    default:
        break;
    }
    effect_gobj = efManagerLinkEntryBeamMakeEffect(&fp->entry_pos);

    if (effect_gobj == NULL)
    {
        return NULL;
    }
    switch (fp->fkind)
    {
    case REMIX_ID_LINK:
    case REMIX_ID_JLINK:
    case REMIX_ID_ELINK:
    case REMIX_ID_YLINK:
        ftRemixSizeEntryTopJointNoY(effect_gobj, fp);
        break;

    case REMIX_ID_BANJO:
        ftRemixSizeEntrySecondJoint(effect_gobj, fp);
        break;

    default:
        ftRemixSizeEntryTopJoint(effect_gobj, fp);
        break;
    }
    return effect_gobj;
}

/**
 * Size.asm entry.gfx_._blue_falcon: the Captain car (also the Bowser clown car and the Conker hand). Falcon and J Falcon
 * scale the 2nd joint, Conker the top joint, Bowser lowers the top joint by 576.0F * (1 - multiplier) and scales the 2nd
 * joint. Other fighters keep the vanilla car.
 */
void ftRemixSizeEntryCaptainCar(GObj *effect_gobj, FTStruct *fp)
{
    DObj *top_joint;
    f32 multiplier;

    if (effect_gobj == NULL)
    {
        return;
    }
    top_joint = DObjGetStruct(effect_gobj);
    multiplier = ftRemixSizeGetMultiplier(fp->player);

    switch (fp->fkind)
    {
    case REMIX_ID_CAPTAIN:
    case REMIX_ID_JFALCON:
        ftRemixSizeEntrySetJointScale(top_joint->child, multiplier);
        break;

    case REMIX_ID_CONKER:
        ftRemixSizeEntrySetJointScale(top_joint, multiplier);
        break;

    case REMIX_ID_BOWSER:
        top_joint->translate.vec.f.y += FTREMIX_SIZE_ENTRY_CAR_BOWSER_HEIGHT - (FTREMIX_SIZE_ENTRY_CAR_BOWSER_HEIGHT * multiplier);
        ftRemixSizeEntrySetJointScale(top_joint->child, multiplier);
        break;

    default:
        break;
    }
}

#endif
