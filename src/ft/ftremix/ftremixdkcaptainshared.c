/**
 * ftremixdkcaptainshared.c - Shared Remix patches for the Donkey Kong and Captain Falcon clones
 * (.toolchain/pkgs/smashremix-src/src/dkshared.asm, scope DKShared, and captainshared.asm, scope CaptainShared).
 *
 * Each helper is called from a vanilla routine under REGION_REMIX and returns the vanilla
 * answer for every fighter that is not one of the clones named in its comment.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixeffects.h"
#include "ft/ftremix/ftremixeffectsrules.h"
#include "ft/ftremix/ftremixnessshared.h"
#include "fighter_registry.h"
#include "if/ifremixannouncer.h"
#include <ef/efdef.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

/* dkshared.asm: the cargo, item and damage checks that treat Donkey Kong, NDonkey and GDonkey alike */
sb32 ftRemixDKSharedIsCargoClone(s32 fkind)
{
    return (fkind == REMIX_ID_JDK) || (fkind == REMIX_ID_MARINA) || (fkind == REMIX_ID_NMARINA);
}

/* Character.asm falcon_dive_x_fix_ and falcon_dive_y_fix_: the row of Falcon's grab offset table (27 rows, vanilla ids)
 * that a grabbed fighter's Remix id stands for, read from Character.falcon_dive_id.table */
s32 ftRemixFalconDiveOffsetRow(s32 fkind)
{
    const u32 *row = (const u32*)remix_ram_image_resolve(REMIX_SYM_Character__falcon_dive_id__table + (fkind * 4));

    return (row != NULL) ? (s32)*row : nFTKindMario;
}

/* captainshared.asm: Falcon-family clones that share the Captain Falcon kick, punch and hand bone */
static sb32 ftRemixCaptainSharedIsFalconClone(s32 fkind)
{
    return (fkind == REMIX_ID_GND) || (fkind == REMIX_ID_JFALCON) || (fkind == REMIX_ID_DRAGONKING);
}

/* captainshared.asm get_punch_bone_: the Falcon-family clones and Wolf (his Up Special slash) use the Falcon hand bone (joints[16]) */
sb32 ftRemixCaptainSharedUsesFalconHandBone(s32 fkind)
{
    return ftRemixCaptainSharedIsFalconClone(fkind) || (fkind == REMIX_ID_WOLF);
}

/* captainshared.asm get_kick_anim_struct_: a Falcon-family clone loads its own file 7 (ftRemixPowerData(fp)->p_file_special2) */
EFDesc* ftRemixCaptainSharedKickDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixCaptainKickDesc;

    if (!ftRemixCaptainSharedIsFalconClone(fp->fkind))
    {
        return vanilla_desc;
    }
    sRemixCaptainKickDesc = *vanilla_desc;
    sRemixCaptainKickDesc.file_head = ftRemixPowerData(fp)->p_file_special2;

    return &sRemixCaptainKickDesc;
}

/* captainshared.asm get_punch_anim_struct_: the file slot of the punch gfx of `power_kind`, NULL when it is not loaded */
static void** ftRemixCaptainSharedPunchFile(s32 power_kind, FTRemixEffectsPunchGfx gfx)
{
    FTData *data = port_fighter_data(power_kind);
    void **file;

    if (data == NULL)
    {
        return NULL;
    }
    file = (gfx == nFTRemixEffectsPunchGfxWolfSlash) ? data->p_file_special4 : data->p_file_special3;

    if ((file == NULL) || (*file == NULL))
    {
        return NULL;
    }
    return file;
}

/* captainshared.asm get_punch_anim_struct_: the clone a Kirby or J Kirby copied (0x0ADC) picks the file, as the clone itself does.
 * GND, J Falcon and Dragon King read their own file 8; Wolf's slash reads WOLF file 9 at its own offsets. */
EFDesc* ftRemixCaptainSharedPunchDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixCaptainPunchDesc;
    s32 power_kind = ftRemixNessSharedPowerKind(fp);
    FTRemixEffectsPunchGfx gfx = ftRemixEffectsRulesPunchGfx(power_kind);
    void **file;

    if (gfx == nFTRemixEffectsPunchGfxFalcon)
    {
        return vanilla_desc;
    }
    file = ftRemixCaptainSharedPunchFile(power_kind, gfx);

    if (file == NULL)
    {
        return vanilla_desc;
    }
    sRemixCaptainPunchDesc = *vanilla_desc;
    sRemixCaptainPunchDesc.file_head = file;

    if (gfx == nFTRemixEffectsPunchGfxWolfSlash)
    {
        sRemixCaptainPunchDesc.o_dobjsetup = FTREMIX_EFFECTS_WOLF_SLASH_O_DOBJSETUP;
        sRemixCaptainPunchDesc.o_mobjsub = FTREMIX_EFFECTS_WOLF_SLASH_O_MOBJSUB;
        sRemixCaptainPunchDesc.o_anim_joint = FTREMIX_EFFECTS_WOLF_SLASH_O_ANIM_JOINT;
        sRemixCaptainPunchDesc.o_matanim_joint = FTREMIX_EFFECTS_WOLF_SLASH_O_MATANIM_JOINT;
    }
    return &sRemixCaptainPunchDesc;
}

/* captainshared.asm get_punch_anim_struct_: calls PokemonAnnouncer.firepunch_announcement_ for every power except GND, WOLF and MTWO */
void ftRemixCaptainSharedPunchAnnounce(FTStruct *fp)
{
    if (!ftRemixEffectsRulesPunchAnnounces(ftRemixNessSharedPowerKind(fp)) || !ifRemixAnnouncerIsActive())
    {
        return;
    }
    ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallFirePunch);
}

#endif
