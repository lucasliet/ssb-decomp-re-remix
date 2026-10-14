/**
 * ftremixlinkshared.c - Smash Remix linkshared.asm (scope LinkShared): the Link
 * clones (Young, E and J Link, plus N Link and NY Link where the vanilla check
 * lists them) reuse the Link code paths. Each helper keeps the vanilla value
 * unless the fighter is one of the clones that patch covers.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <wp/wptypes.h>
#include "ftremix.h"
#include "ftremixhooks.h"
#include "fighter_registry.h"

#define LINKSHARED_USP_JLINK_AIR_VEL_Y 71.0F        /* 0x428E0000, linkshared.asm up_special_velocity_ */
#define LINKSHARED_USP_ELINK_AIR_VEL_Y 72.0F        /* 0x42900000, linkshared.asm up_special_velocity_ */
#define LINKSHARED_USP_JLINK_FALL_DRIFT 0.75F       /* 0x3F400000, linkshared.asm up_special_aerial_drift_ */

/* dair_fix_, grab_shield_fix_, item_shield_fix_ and the Link checks in ftCommonAttackAirLw, ftCommonCatchWait
 * and ftParamLinkResetShieldModelParts: the Link family that uses the Link shield and dair code paths. */
sb32 ftRemixLinkSharedIsLinkFamily(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_LINK) || (fp->fkind == REMIX_ID_NLINK) || (fp->fkind == REMIX_ID_YLINK) ||
           (fp->fkind == REMIX_ID_NYLINK) || (fp->fkind == REMIX_ID_ELINK) || (fp->fkind == REMIX_ID_JLINK);
}

/* sword_trail_fix_ (0x800E6768): the Link sword afterimage runs for Link and the Young, E and J Link clones. */
sb32 ftRemixLinkSharedHasSwordTrail(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_LINK) || (fp->fkind == REMIX_ID_YLINK) || (fp->fkind == REMIX_ID_ELINK) ||
           (fp->fkind == REMIX_ID_JLINK);
}

/* up_special_velocity_ (0x80164388): Up Special spin air velocity for J Link and E Link. Young Link is handled by
 * ftRemixYoungLinkUpSpecialAirVelY, which runs first at the call site. */
f32 ftRemixLinkSharedUpSpecialAirVelY(FTStruct *fp, f32 vanilla_vel_y)
{
    if (fp->fkind == REMIX_ID_JLINK)
    {
        return LINKSHARED_USP_JLINK_AIR_VEL_Y;
    }
    if (fp->fkind == REMIX_ID_ELINK)
    {
        return LINKSHARED_USP_ELINK_AIR_VEL_Y;
    }
    return vanilla_vel_y;
}

/* up_special_aerial_drift_ (0x80163FF8): aerial drift multiplier once J Link enters freefall after Up Special. */
f32 ftRemixLinkSharedUpSpecialFallDrift(FTStruct *fp, f32 vanilla_drift)
{
    if (fp->fkind != REMIX_ID_JLINK)
    {
        return vanilla_drift;
    }
    return LINKSHARED_USP_JLINK_FALL_DRIFT;
}

/* The clones whose Link weapon descriptors are patched to read their own file. */
static sb32 ftRemixLinkSharedIsWeaponClone(s32 fkind)
{
    return (fkind == REMIX_ID_YLINK) || (fkind == REMIX_ID_ELINK) || (fkind == REMIX_ID_JLINK);
}

/* Kirby and J Kirby use the power they copied (N64 0x0ADC) for a weapon's owner. */
static s32 ftRemixLinkSharedWeaponOwnerKind(FTStruct *fp)
{
    if ((fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return fp->passive_vars.kirby.copy_id;
    }
    return fp->fkind;
}

/* Points desc->p_weapon at the loaded file 1 (is_special1 = FALSE) or file 6 (TRUE) of a clone owner. Other owners,
 * and clones whose file is not loaded, keep the vanilla pointer. */
static void ftRemixLinkSharedSetWeaponFile(WPDesc *desc, s32 owner_kind, sb32 is_special1)
{
    FTData *data;
    void **file;

    if (!ftRemixLinkSharedIsWeaponClone(owner_kind))
    {
        return;
    }
    data = port_fighter_data(owner_kind);
    if (data == NULL)
    {
        return;
    }
    file = is_special1 ? data->p_file_special1 : data->p_file_main;
    if ((file == NULL) || (*file == NULL))
    {
        return;
    }
    desc->p_weapon = file;
}

/* wpLinkSpinAttackMakeWeapon, linkshared.asm up_special_fix_ (0x8016CB54): the Up Special weapon reads its attributes
 * from the clone's file 1. desc is a copy of the vanilla descriptor. */
void ftRemixLinkSharedUpSpecialWeapon(FTStruct *fp, WPDesc *desc)
{
    ftRemixLinkSharedSetWeaponFile(desc, fp->fkind, FALSE);
}

/* wpLinkBoomerangMakeWeapon, linkshared.asm boomerang_fix_ (0x8016DAC0): the boomerang reads its attributes from the
 * clone's file 6, and a Kirby that copied a clone uses that clone. desc is a copy of the vanilla descriptor. */
void ftRemixLinkSharedBoomerangWeapon(FTStruct *fp, WPDesc *desc)
{
    ftRemixLinkSharedSetWeaponFile(desc, ftRemixLinkSharedWeaponOwnerKind(fp), TRUE);
}

/* itLinkBombMakeItem / bomb_flash_ / bomb_explosion_ (linkshared.asm create_bomb_, 0x801865A0, hooks
 * 0x801859DC and 0x801863B0): a Link bomb owned by a clone loads its descriptor data (attributes, bloat
 * scales and explosion attack events) from the owner clone's file 1, selected by the owner id that
 * ftRemixYoungLinkBombSetOwner stores in coll_data.cliff_id. Other owners keep the vanilla file. Unlike
 * boomerang_fix_, the bomb hooks read the creator's own id, so a Kirby that copied a Link clone does not
 * redirect the bomb. */
void **ftRemixLinkSharedBombFile(s32 fkind, void **vanilla_file)
{
    FTData *data;

    if (!ftRemixLinkSharedIsWeaponClone(fkind))
    {
        return vanilla_file;
    }
    data = port_fighter_data(fkind);
    if ((data == NULL) || (data->p_file_main == NULL) || (*data->p_file_main == NULL))
    {
        return vanilla_file;
    }
    return data->p_file_main;
}

#endif
