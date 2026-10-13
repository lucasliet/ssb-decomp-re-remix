/**
 * ftremixdkcaptainshared.c - Shared Remix patches for the Donkey Kong and Captain Falcon clones
 * (.toolchain/pkgs/smashremix-src/src/dkshared.asm, scope DKShared, and captainshared.asm, scope CaptainShared).
 *
 * Each helper is called from a vanilla routine under REGION_REMIX and returns the vanilla
 * answer for every fighter that is not one of the clones named in its comment.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include <ef/efdef.h>

/* dkshared.asm: the cargo, item and damage checks that treat Donkey Kong, NDonkey and GDonkey alike */
sb32 ftRemixDKSharedIsCargoClone(s32 fkind)
{
    return (fkind == REMIX_ID_JDK) || (fkind == REMIX_ID_MARINA) || (fkind == REMIX_ID_NMARINA);
}

/* captainshared.asm: Falcon-family clones that share the Captain Falcon kick, punch and hand bone */
static sb32 ftRemixCaptainSharedIsFalconClone(s32 fkind)
{
    return (fkind == REMIX_ID_GND) || (fkind == REMIX_ID_JFALCON) || (fkind == REMIX_ID_DRAGONKING);
}

/* captainshared.asm get_punch_bone_: the Falcon-family clones use the Falcon hand bone (joints[16]) */
sb32 ftRemixCaptainSharedUsesFalconHandBone(s32 fkind)
{
    return ftRemixCaptainSharedIsFalconClone(fkind);
}

/* captainshared.asm get_kick_anim_struct_: a Falcon-family clone loads its own file 7 (fp->data->p_file_special2) */
EFDesc* ftRemixCaptainSharedKickDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixCaptainKickDesc;

    if (!ftRemixCaptainSharedIsFalconClone(fp->fkind))
    {
        return vanilla_desc;
    }
    sRemixCaptainKickDesc = *vanilla_desc;
    sRemixCaptainKickDesc.file_head = fp->data->p_file_special2;

    return &sRemixCaptainKickDesc;
}

/* captainshared.asm get_punch_anim_struct_: a Falcon-family clone loads its own file 8 (fp->data->p_file_special3) */
EFDesc* ftRemixCaptainSharedPunchDesc(FTStruct *fp, EFDesc *vanilla_desc)
{
    static EFDesc sRemixCaptainPunchDesc;

    if (!ftRemixCaptainSharedIsFalconClone(fp->fkind))
    {
        return vanilla_desc;
    }
    sRemixCaptainPunchDesc = *vanilla_desc;
    sRemixCaptainPunchDesc.file_head = fp->data->p_file_special3;

    return &sRemixCaptainPunchDesc;
}

#endif
