/**
 * ftremixnessshared.c - Smash Remix nessshared.asm (scope NessShared): J Ness and Lucas (and a Kirby that copied them)
 * reuse the Ness code paths. Remix swaps the weapon, item and gfx structures for copies whose file pointer is the clone's
 * own file; the port keeps the vanilla WPDesc and ITDesc and swaps the file pointer of a copy, so the weapon attributes
 * come from the clone's loaded file instead of the (possibly not loaded) Ness globals. The values are in
 * ftremixnessrules.h.
 *
 * Remix file n is the FTData slot of that number (docs/remix/fighters.md): file 1 p_file_main, file 4 p_file_model, file 7
 * p_file_special2. For J Ness and Lucas the PK Fire file is file 7; it is Ness's special1 in the vanilla FTData order.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixnessshared.h"
#include "ft/ftremix/ftremixnessrules.h"
#include "ft/ftremix/ftremix.h"
#include "gm/gmremixtoggle.h"
#include <ef/efmanager.h>
#include "ft/ftchar/ftness/ftness.h"
#include "fighter_registry.h"
#include <reloc_data.h>

typedef enum FTRemixNessFile
{
    nFTRemixNessFileMain,
    nFTRemixNessFileModel,
    nFTRemixNessFilePKFire

} FTRemixNessFile;

/** PK Fire weapon words beyond the vanilla WPStruct: the weapon has no vars of its own (asm projectile word 0x78). */
typedef struct WPRemixNessPKFireVars
{
    s32 power_kind;

} WPRemixNessPKFireVars;

/** PK Fire pillar item words beyond the vanilla ITStruct: `xf` is ITStruct.item_vars.pkfire.xf (asm item word 0x354). */
typedef struct ITRemixNessPKFireVars
{
    LBTransform *xf;
    s32 power_kind;

} ITRemixNessPKFireVars;

_Static_assert(sizeof(WPRemixNessPKFireVars) <= sizeof(((WPStruct*)0)->weapon_vars), "WPRemixNessPKFireVars must fit in WPStruct.weapon_vars");
_Static_assert(sizeof(ITRemixNessPKFireVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixNessPKFireVars must fit in ITStruct.item_vars");

s32 ftRemixNessSharedPowerKind(FTStruct *fp)
{
    if ((fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return fp->passive_vars.kirby.copy_id;
    }
    return fp->fkind;
}

/** The loaded file `which` of `kind`, or `vanilla_file` if `kind` is not a clone with its own files or it is not loaded. */
static void **ftRemixNessSharedGetFile(s32 kind, void **vanilla_file, FTRemixNessFile which)
{
    FTData *data;
    void **file;

    if (!ftRemixNessRulesUsesOwnFiles(kind))
    {
        return vanilla_file;
    }
    data = port_fighter_data(kind);

    if (data == NULL)
    {
        return vanilla_file;
    }
    switch (which)
    {
    case nFTRemixNessFileMain:
        file = data->p_file_main;
        break;

    case nFTRemixNessFileModel:
        file = data->p_file_model;
        break;

    default:
        file = data->p_file_special2;
        break;
    }
    if ((file == NULL) || (*file == NULL))
    {
        return vanilla_file;
    }
    return file;
}

void **ftRemixNessSharedModelFile(s32 kind, void **vanilla_file)
{
    return ftRemixNessSharedGetFile(kind, vanilla_file, nFTRemixNessFileModel);
}

void ftRemixNessSharedPKFireLaunch(FTStruct *fp, FTRemixNessPKFireLaunch *launch)
{
    s32 power_kind = ftRemixNessSharedPowerKind(fp);

    launch->is_air = ftRemixNessRulesPKFireIsAir(power_kind, launch->is_air);
    launch->speed_air = ftRemixNessRulesPKFireSpeed(power_kind, TRUE, launch->speed_air);
    launch->speed_ground = ftRemixNessRulesPKFireSpeed(power_kind, FALSE, launch->speed_ground);
}

void ftRemixNessSharedPKFireWeaponDesc(FTStruct *fp, WPDesc *desc)
{
    desc->p_weapon = ftRemixNessSharedGetFile(ftRemixNessSharedPowerKind(fp), desc->p_weapon, nFTRemixNessFilePKFire);
}

void ftRemixNessSharedPKFireSetKind(WPStruct *wp, FTStruct *fp)
{
    ((WPRemixNessPKFireVars*)&wp->weapon_vars)->power_kind = ftRemixNessSharedPowerKind(fp);
}

/** Power kind stored in a PK Fire weapon by ftRemixNessSharedPKFireSetKind. */
static s32 ftRemixNessSharedPKFireGetKind(WPStruct *wp)
{
    return ((WPRemixNessPKFireVars*)&wp->weapon_vars)->power_kind;
}

sb32 ftRemixNessSharedPKFireExplode(WPStruct *wp, Vec3f *pos)
{
    if (!ftRemixNessRulesPKFireExplodes(ftRemixNessSharedPKFireGetKind(wp)))
    {
        return FALSE;
    }
    efManagerSparkleWhiteMakeEffect(pos);

    return TRUE;
}

void ftRemixNessSharedPKFireItemDesc(WPStruct *wp, ITDesc *desc)
{
    s32 power_kind = ftRemixNessSharedPKFireGetKind(wp);

    if (!ftRemixNessRulesPKFirePillarUsesOwnFile(power_kind))
    {
        return;
    }
    desc->p_file = ftRemixNessSharedGetFile(power_kind, desc->p_file, nFTRemixNessFilePKFire);
}

void ftRemixNessSharedPKFireItemSetKind(ITStruct *ip, WPStruct *wp)
{
    ((ITRemixNessPKFireVars*)&ip->item_vars)->power_kind = ftRemixNessSharedPKFireGetKind(wp);
}

void ftRemixNessSharedPKFirePillarPhysics(ITStruct *ip, f32 *gravity, f32 *tvel)
{
    s32 power_kind = ((ITRemixNessPKFireVars*)&ip->item_vars)->power_kind;

    *gravity = ftRemixNessRulesPKFirePillarGravity(power_kind, *gravity);
    *tvel = ftRemixNessRulesPKFirePillarTVel(power_kind, *tvel);
}

s32 ftRemixNessSharedPKThunderEndDelay(FTStruct *fp, s32 vanilla_delay)
{
    return ftRemixNessRulesPKThunderEndDelay(fp->fkind, vanilla_delay);
}

void ftRemixNessSharedPKThunderLanding(FTStruct *fp, f32 *drift, f32 *landing_lag)
{
    *drift = ftRemixNessRulesPKThunderDrift(fp->fkind, *drift);
    *landing_lag = ftRemixNessRulesPKThunderLandingLag(fp->fkind, *landing_lag);
}

void ftRemixNessSharedPKThunderHeadDesc(FTStruct *fp, WPDesc *desc)
{
    desc->p_weapon = ftRemixNessSharedGetFile(fp->fkind, desc->p_weapon, nFTRemixNessFileMain);

    if (fp->fkind == REMIX_ID_LUCAS)
    {
        desc->proc_hit = wpNessPKThunderTrailProcHit;
    }
}

void ftRemixNessSharedPKThunderTrailDesc(FTStruct *fp, WPDesc *desc)
{
    desc->p_weapon = ftRemixNessSharedGetFile(fp->fkind, desc->p_weapon, nFTRemixNessFileMain);
}

/** Fighter kind that fired the PK Thunder whose head is `head_gobj`, or -1 if the head does not know it. */
static s32 ftRemixNessSharedReflectHeadFiredBy(GObj *head_gobj)
{
    GObj *parent_gobj = wpGetStruct(head_gobj)->weapon_vars.pkthunder.parent_gobj;

    if (parent_gobj == NULL)
    {
        return -1;
    }
    return ftGetStruct(parent_gobj)->fkind;
}

void ftRemixNessSharedPKReflectHeadDesc(GObj *old_head_gobj, WPDesc *desc)
{
    desc->p_weapon = ftRemixNessSharedGetFile(ftRemixNessSharedReflectHeadFiredBy(old_head_gobj), desc->p_weapon, nFTRemixNessFileMain);
}

void ftRemixNessSharedPKReflectHeadSetParent(GObj *new_head_gobj, GObj *old_head_gobj)
{
    wpGetStruct(new_head_gobj)->weapon_vars.pkthunder.parent_gobj = wpGetStruct(old_head_gobj)->weapon_vars.pkthunder.parent_gobj;
}

void ftRemixNessSharedPKReflectTrailDesc(GObj *old_gobj, s32 trail_id, WPDesc *desc)
{
    GObj *head_gobj = (trail_id == 0) ? old_gobj : wpGetStruct(old_gobj)->weapon_vars.pkthunder_trail.head_gobj;

    desc->p_weapon = ftRemixNessSharedGetFile(ftRemixNessSharedReflectHeadFiredBy(head_gobj), desc->p_weapon, nFTRemixNessFileMain);
}

FTSpecialColl* ftRemixNessSharedPsiMagnetColl(FTStruct *fp)
{
    void *mainmotion_file = gFTNessFileMainMotion;
    FTSpecialColl *vanilla_coll;

    if ((fp->fkind == REMIX_ID_JNESS) || (fp->fkind == REMIX_ID_NLUCAS))
    {
        mainmotion_file = *fp->data->p_file_mainmotion;
    }
    vanilla_coll = (FTSpecialColl*) ((uintptr_t)mainmotion_file + (intptr_t)llNessMainMotionLwAbsorbFTSpecialColl);

    return ftRemixLucasDSPSpecialColl(fp, vanilla_coll);
}

sb32 ftRemixNessSharedPKReflectFixEnabled(void)
{
    return gmRemixToggle(pk_thunder_reflect_crash_fix) != 0;
}

#endif
