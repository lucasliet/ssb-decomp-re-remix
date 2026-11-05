/**
 * ftremixeffects.c - fighter effect patches that vanilla effect and capture code calls, see ftremixeffects.h.
 *
 * Remix swaps the animation structs of the vanilla effects (EFDesc) for copies that read the clone's own file and, for
 * some, other offsets inside it. The port keeps the vanilla EFDesc and hands efManagerMakeEffect a static copy.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixeffects.h"
#include "ft/ftremix/ftremixeffectsrules.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixnessshared.h"
#include "ft/ftremix/ftremixpikashared.h"
#include <ef/effect.h>
#include <lb/library.h>
#include <wp/weapon.h>
#include "remix/remix_roster_data.h"

void ftRemixDededeInhaleWindOffset(LBTransform *xf, GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 offset_x = ftRemixEffectsRulesInhaleWindOffsetX(fp->fkind) - FTREMIX_EFFECTS_INHALE_WIND_X_KIRBY;

    xf->translate.x += fp->lr * offset_x * ftRemixSizeGetMultiplier(fp->player);
}

s32 ftRemixDededeCaptureWaitBreakout(FTStruct *captured_fp, s32 vanilla_wait)
{
    FTStruct *absorber_fp;

    if (captured_fp->capture_gobj == NULL)
    {
        return vanilla_wait;
    }
    absorber_fp = ftGetStruct(captured_fp->capture_gobj);

    return ftRemixEffectsRulesInhaleBreakoutWait(absorber_fp->fkind, captured_fp->percent_damage, vanilla_wait);
}

/** The file slot `slot` of a fighter when it is loaded, else NULL. */
static void** ftRemixEffectsLoadedSlot(void **slot)
{
    if ((slot == NULL) || (*slot == NULL))
    {
        return NULL;
    }
    return slot;
}

EFDesc* ftRemixReflectorDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixReflectorDesc;
    void **file;

    if (!ftRemixEffectsRulesReflectorUsesOwnFile(fp->fkind))
    {
        return vanilla_desc;
    }
    file = ftRemixEffectsLoadedSlot(fp->data->p_file_special2);

    if (file == NULL)
    {
        return vanilla_desc;
    }
    sRemixReflectorDesc = *vanilla_desc;
    sRemixReflectorDesc.file_head = file;
    sRemixReflectorDesc.o_dobjsetup = ftRemixEffectsRulesReflectorDObjSetup(fp->fkind, vanilla_desc->o_dobjsetup);
    sRemixReflectorDesc.o_anim_joint = ftRemixEffectsRulesReflectorFirstAnimJoint(fp->fkind, vanilla_desc->o_anim_joint);

    return &sRemixReflectorDesc;
}

AObjEvent32** ftRemixReflectorAnimJoint(GObj *effect_gobj, s32 anim_id, AObjEvent32 **vanilla_anim_joint)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    FTStruct *fp;
    void **file;

    if (ep->fighter_gobj == NULL)
    {
        return vanilla_anim_joint;
    }
    fp = ftGetStruct(ep->fighter_gobj);

    if (!ftRemixEffectsRulesReflectorUsesOwnFile(fp->fkind))
    {
        return vanilla_anim_joint;
    }
    file = ftRemixEffectsLoadedSlot(fp->data->p_file_special2);

    if (file == NULL)
    {
        return vanilla_anim_joint;
    }
    return lbRelocGetFileData(AObjEvent32**, *file, ftRemixEffectsRulesReflectorAnimJoint(fp->fkind, anim_id, 0));
}

/** `vanilla_desc` with the model file (Remix file 4) of `kind` (J Ness, Lucas), or `vanilla_desc` itself when `kind` has none. */
static EFDesc* ftRemixNessModelDesc(s32 kind, EFDesc *vanilla_desc, EFDesc *remix_desc)
{
    void **file = ftRemixNessSharedModelFile(kind, vanilla_desc->file_head);

    if (file == vanilla_desc->file_head)
    {
        return vanilla_desc;
    }
    *remix_desc = *vanilla_desc;
    remix_desc->file_head = file;

    return remix_desc;
}

/** Sets the offsets of the Lucas PK Thunder trail gfx (pkthunder_anim_struct2_lucas, pkthunder_anim_struct3_lucas) in `desc`. */
static void ftRemixLucasSetTrailOffsets(EFDesc *desc)
{
    desc->o_dobjsetup = FTREMIX_EFFECTS_LUCAS_PKTHUNDER_TRAIL_O_DOBJSETUP;
    desc->o_mobjsub = 0;
    desc->o_anim_joint = 0;
    desc->o_matanim_joint = 0;
}

EFDesc* ftRemixNessPKThunderWaveDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixWaveDesc;
    EFDesc *desc = ftRemixNessModelDesc(fp->fkind, vanilla_desc, &sRemixWaveDesc);

    if ((desc == vanilla_desc) || (fp->fkind != REMIX_ID_LUCAS))
    {
        return desc;
    }
    desc->o_dobjsetup = FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_DOBJSETUP;
    desc->o_mobjsub = FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_MOBJSUB;
    desc->o_anim_joint = FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_ANIM_JOINT;
    desc->o_matanim_joint = FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_MATANIM_JOINT;

    return desc;
}

EFDesc* ftRemixNessPKThunderTrailDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixTrailDesc;
    EFDesc *desc = ftRemixNessModelDesc(fp->fkind, vanilla_desc, &sRemixTrailDesc);

    if ((desc != vanilla_desc) && (fp->fkind == REMIX_ID_LUCAS))
    {
        ftRemixLucasSetTrailOffsets(desc);
    }
    return desc;
}

s32 ftRemixWeaponOwnerKind(GObj *weapon_gobj)
{
    GObj *owner_gobj = wpGetStruct(weapon_gobj)->owner_gobj;

    if (owner_gobj == NULL)
    {
        return -1;
    }
    return ftGetStruct(owner_gobj)->fkind;
}

/** Fighter kind that fired the PK Thunder whose reflected head is `head_gobj`, or -1 (nessshared.asm reads it from the projectile struct). */
static s32 ftRemixNessReflectHeadFiredBy(GObj *head_gobj)
{
    GObj *parent_gobj = wpGetStruct(head_gobj)->weapon_vars.pkthunder.parent_gobj;

    if (parent_gobj == NULL)
    {
        return -1;
    }
    return ftGetStruct(parent_gobj)->fkind;
}

EFDesc* ftRemixNessPKReflectTrailDesc(GObj *head_gobj, EFDesc *vanilla_desc)
{
    static EFDesc sRemixReflectTrailDesc;
    s32 kind = ftRemixNessReflectHeadFiredBy(head_gobj);
    EFDesc *desc = ftRemixNessModelDesc(kind, vanilla_desc, &sRemixReflectTrailDesc);

    if ((desc != vanilla_desc) && (kind == REMIX_ID_LUCAS))
    {
        ftRemixLucasSetTrailOffsets(desc);
    }
    return desc;
}

EFDesc* ftRemixNessPsiMagnetDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixPsiMagnetDesc;
    void **file;

    if (!ftRemixEffectsRulesPsiMagnetUsesFile6(fp->fkind))
    {
        return vanilla_desc;
    }
    file = ftRemixEffectsLoadedSlot(fp->data->p_file_special1);

    if (file == NULL)
    {
        return vanilla_desc;
    }
    sRemixPsiMagnetDesc = *vanilla_desc;
    sRemixPsiMagnetDesc.file_head = file;

    return &sRemixPsiMagnetDesc;
}

void ftRemixLucasPsiMagnetPlaceGfx(GObj *effect_gobj, FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_LUCAS)
    {
        return;
    }
    DObjGetStruct(effect_gobj)->child->translate.vec.f.x = FTREMIX_EFFECTS_LUCAS_PSIMAGNET_GFX_X * fp->lr;
}

sb32 ftRemixLucasPKThunderGfxKeepsRotation(FTStruct *fp)
{
    return fp->fkind == REMIX_ID_LUCAS;
}

EFDesc* ftRemixPikaThunderTrailDesc(s32 owner_fkind, EFDesc *vanilla_desc)
{
    static EFDesc sRemixThunderTrailDesc;
    void **file = ftRemixPikaSharedModelFile(owner_fkind, vanilla_desc->file_head);

    if (file == vanilla_desc->file_head)
    {
        return vanilla_desc;
    }
    sRemixThunderTrailDesc = *vanilla_desc;
    sRemixThunderTrailDesc.file_head = file;

    return &sRemixThunderTrailDesc;
}

EFDesc* ftRemixPikaThunderJoltDesc(GObj *jolt_gobj, EFDesc *vanilla_desc)
{
    static EFDesc sRemixThunderJoltDesc;
    void **file = ftRemixPikaSharedJoltEffectFile(ftRemixPikaSharedThunderJoltPowerKind(wpGetStruct(jolt_gobj)), vanilla_desc->file_head);

    if (file == vanilla_desc->file_head)
    {
        return vanilla_desc;
    }
    sRemixThunderJoltDesc = *vanilla_desc;
    sRemixThunderJoltDesc.file_head = file;

    return &sRemixThunderJoltDesc;
}

#endif
