/**
 * ftremixsamusshared2.c - Smash Remix samusshared.asm (scope SamusShared),
 * second group: the Samus branches of the vanilla charge sparkle, Kirby copy
 * power, Kirby copy charge flash and grab beam routines, plus the Dark Samus
 * grapple glow file. The first group (charge shot and bomb files) is in
 * ftremixsamusshared.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftparam.h"
#include "gm/gmdef.h"
#include "ef/eftypes.h"

/* Remix puts the original Dark Samus file 0x15D at the end of its file 6 (samusshared.asm, dsamus_grapple_anim_gfx_struct). */
#define REMIX_DSAMUS_GRAPPLE_FILE_OFFSET 0x2970

static EFDesc sFTRemixDSamusGrappleBeamDesc;

/* GFXRoutine.id.* from build/remixasm/bass.log (id = counter + 0x55). */
#define REMIX_GFX_KIRBY_MTWO_CHARGE 0x64
#define REMIX_GFX_SHEIK_CHARGE 0x6D
#define REMIX_GFX_DEDEDE_CHARGE 0x74

/* Full-charge levels that Kirby's copy word must equal for each GFX flash.
 * Mewtwo, Sheik and Peppy read the copysamus charge word (N64 0x0AE0); Dedede
 * reads N64 0x0AE4, which the Kirby struct names copysamus_charge_recoil. */
#define REMIX_KIRBY_MTWO_CHARGE_LEVEL 7
#define REMIX_KIRBY_SHEIK_CHARGE_LEVEL 6
#define REMIX_KIRBY_DEDEDE_CHARGE_LEVEL 2

static sb32 ftRemixSamusSharedIsClone(s32 fkind)
{
    return (fkind == REMIX_ID_DSAMUS) || (fkind == REMIX_ID_JSAMUS) || (fkind == REMIX_ID_ESAMUS);
}

/* fully_charged_check_ (0x800EAC18, ftParamMakeEffect): the charge sparkle takes
 * the Samus case for the Samus clones and Mewtwo. */
s32 ftRemixSamusSharedChargeSparkleFkind(s32 fkind)
{
    if (ftRemixSamusSharedIsClone(fkind) || (fkind == REMIX_ID_MTWO))
    {
        return nFTKindSamus;
    }
    return fkind;
}

/* kirby_power_change_ (0x80161EC4, ftKirbySpecialNInitPassiveVars): copying these
 * powers takes the Samus case, which clears the copysamus charge words. */
s32 ftRemixSamusSharedKirbyCopyPowerFkind(s32 copy_id)
{
    if (ftRemixSamusSharedIsClone(copy_id) || (copy_id == REMIX_ID_MTWO) || (copy_id == REMIX_ID_MARTH) ||
        (copy_id == REMIX_ID_SHEIK) || (copy_id == REMIX_ID_PEPPY))
    {
        return nFTKindSamus;
    }
    return copy_id;
}

/* Full-charge GFX routine for a Kirby copy, or -1 when the copy has none or is
 * not at its full-charge level. */
static s32 ftRemixKirbyCopyChargeFlashGfxID(FTStruct *fp)
{
    s32 copy_id = fp->passive_vars.kirby.copy_id;

    switch (copy_id)
    {
    case REMIX_ID_MTWO:
        if (fp->passive_vars.kirby.copysamus_charge_level == REMIX_KIRBY_MTWO_CHARGE_LEVEL)
        {
            return REMIX_GFX_KIRBY_MTWO_CHARGE;
        }
        break;

    case REMIX_ID_SHEIK:
    case REMIX_ID_PEPPY:
        if (fp->passive_vars.kirby.copysamus_charge_level == REMIX_KIRBY_SHEIK_CHARGE_LEVEL)
        {
            return REMIX_GFX_SHEIK_CHARGE;
        }
        break;

    case REMIX_ID_DEDEDE:
        if (fp->passive_vars.kirby.copysamus_charge_recoil == REMIX_KIRBY_DEDEDE_CHARGE_LEVEL)
        {
            return REMIX_GFX_DEDEDE_CHARGE;
        }
        break;
    }
    return -1;
}

/* kirby_power_check_flash_ (0x800E99C4, ftParamResetStatUpdateColAnim): called
 * after the vanilla Samus check. A Samus clone copy uses the Samus full-charge
 * flash; Mewtwo, Sheik, Peppy and Dedede copies use their own GFX routine. */
void ftRemixSamusSharedKirbyCopyChargeFlash(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 gfx_id;

    if (ftRemixSamusSharedIsClone(fp->passive_vars.kirby.copy_id))
    {
        if (fp->passive_vars.kirby.copysamus_charge_level == FTKIRBY_COPYSAMUS_CHARGE_MAX)
        {
            ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterCommonSpecialNCharge, 0);
        }
        return;
    }
    gfx_id = ftRemixKirbyCopyChargeFlashGfxID(fp);
    if (gfx_id < 0)
    {
        return;
    }
    /* REMIX-TODO(GFXRoutine): ftParamCheckSetColAnimID rejects ids >= dGMColScriptsDescsNum
     * (ftparam.c), so this colour anim is a no-op until the Remix GFX routine table is ported. */
    ftParamCheckSetFighterColAnimID(fighter_gobj, gfx_id, 0);
}

/* throw_ball_graphic (0x80149C14, ftCommonCatchSetStatus) and throw_ball_graphic_2
 * (0x8014A2DC, ftCommonThrowSetStatus): these fighters get the grab beam glow. */
sb32 ftRemixSamusSharedHasGrappleBeam(s32 fkind)
{
    return (fkind == nFTKindSamus) || (fkind == nFTKindNSamus) || ftRemixSamusSharedIsClone(fkind);
}

/* get_grapple_anim_gfx_struct (0x80101E88, efManagerSamusGrappleBeamGlowMakeEffect): Dark Samus gets a copy of the
 * vanilla descriptor that uses its file 6 and the DObj/MObj/Anim offsets shifted by REMIX_DSAMUS_GRAPPLE_FILE_OFFSET.
 * Its proc 0x8000DF34 is gcPlayAnimAll (same body as the vanilla proc). */
EFDesc *ftRemixSamusSharedGrappleBeamDesc(GObj *fighter_gobj, EFDesc *vanilla_desc)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->fkind != REMIX_ID_DSAMUS) || (fp->data->p_file_special1 == NULL) || (*fp->data->p_file_special1 == NULL))
    {
        return vanilla_desc;
    }
    sFTRemixDSamusGrappleBeamDesc = *vanilla_desc;
    sFTRemixDSamusGrappleBeamDesc.file_head = fp->data->p_file_special1;
    sFTRemixDSamusGrappleBeamDesc.o_dobjsetup += REMIX_DSAMUS_GRAPPLE_FILE_OFFSET;
    sFTRemixDSamusGrappleBeamDesc.o_mobjsub += REMIX_DSAMUS_GRAPPLE_FILE_OFFSET;
    sFTRemixDSamusGrappleBeamDesc.o_anim_joint += REMIX_DSAMUS_GRAPPLE_FILE_OFFSET;
    sFTRemixDSamusGrappleBeamDesc.o_matanim_joint += REMIX_DSAMUS_GRAPPLE_FILE_OFFSET;

    return &sFTRemixDSamusGrappleBeamDesc;
}

#endif
