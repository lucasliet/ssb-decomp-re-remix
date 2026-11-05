/**
 * ftremixyoshishared.c - Smash Remix yoshishared.asm: Yoshi's egg and star weapons
 * read their model and attributes from a hardcoded file pointer, so the clones that
 * reuse them need a descriptor that points at their own file.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixhooks.h"
#include <wp/weapon.h>
#include "fighter_registry.h"
#include "ft/ftremix/ftremixyoshisharedrules.h"

static WPDesc sFTRemixYoshiCloneWeaponDesc;

/* up_special_struct_fix: J Yoshi throws the egg from his own main file. */
WPDesc *ftRemixYoshiEggWeaponDesc(FTStruct *fp, WPDesc *vanilla_desc)
{
    if ((fp->fkind != REMIX_ID_JYOSHI) || (fp->data->p_file_main == NULL))
    {
        return vanilla_desc;
    }
    sFTRemixYoshiCloneWeaponDesc = *vanilla_desc;
    sFTRemixYoshiCloneWeaponDesc.p_weapon = fp->data->p_file_main;

    return &sFTRemixYoshiCloneWeaponDesc;
}

/* down_special_struct_fix: J Yoshi's stars come from his main file, Dedede's Super Dedede Jump
 * stars from his second special file (DEDEDE_file_7). */
WPDesc *ftRemixYoshiStarWeaponDesc(FTStruct *fp, WPDesc *vanilla_desc)
{
    void **star_file;

    if (fp->fkind == REMIX_ID_JYOSHI)
    {
        star_file = fp->data->p_file_main;
    }
    else if (fp->fkind == REMIX_ID_DEDEDE)
    {
        star_file = fp->data->p_file_special2;
    }
    else
    {
        return vanilla_desc;
    }
    if (star_file == NULL)
    {
        return vanilla_desc;
    }
    sFTRemixYoshiCloneWeaponDesc = *vanilla_desc;
    sFTRemixYoshiCloneWeaponDesc.p_weapon = star_file;

    return &sFTRemixYoshiCloneWeaponDesc;
}

/* yoshi_shield_fix_1..8 */
sb32 ftRemixYoshiHasEggShield(s32 fkind)
{
    return ftRemixYoshiSharedRulesHasEggShield(fkind);
}

/* yoshi_grab_fix_1/2 */
sb32 ftRemixYoshiIsGrabSwallower(s32 fkind)
{
    return ftRemixYoshiSharedRulesIsGrabSwallower(fkind);
}

/* yoshi_throw_fix_1 */
sb32 ftRemixYoshiIsThrowSwallower(s32 fkind)
{
    return ftRemixYoshiSharedRulesIsThrowSwallower(fkind);
}

/* The fighter that laid the egg is the capturer of the fighter in it; the Cruel Z Punish egg has none (the asm "safety branch"). */
static sb32 ftRemixYoshiEggIsLaidByJapaneseYoshi(FTStruct *victim_fp)
{
    FTStruct *capture_fp;

    if (victim_fp->capture_gobj == NULL)
    {
        return FALSE;
    }
    capture_fp = ftGetStruct(victim_fp->capture_gobj);

    return ftRemixYoshiSharedRulesIsJapaneseEggLayer(capture_fp->fkind, capture_fp->passive_vars.kirby.copy_id);
}

/* JYoshi.asm joshi_egg_timer */
s32 ftRemixYoshiEggEscapeWaitMax(FTStruct *victim_fp, s32 vanilla_wait)
{
    return ftRemixYoshiSharedRulesEggEscapeWaitMax(ftRemixYoshiEggIsLaidByJapaneseYoshi(victim_fp), vanilla_wait);
}

/* JYoshi.asm joshi_mash */
s32 ftRemixYoshiEggBreakoutInputsMin(FTStruct *victim_fp, s32 vanilla_inputs)
{
    return ftRemixYoshiSharedRulesEggBreakoutInputsMin(ftRemixYoshiEggIsLaidByJapaneseYoshi(victim_fp), vanilla_inputs);
}

/* The registry row is a copy of the parent descriptor's, which is what Remix keeps in Character.yoshi_egg for every fighter
 * except the six that set their own (Bowser.asm:378, Dedede.asm:644, gbowser.asm:364, nbowser.asm:301, NDedede.asm:393,
 * Piano.asm:410). */
ftCommonYoshiEggDesc* ftRemixYoshiEggGetDesc(s32 fkind)
{
    static ftCommonYoshiEggDesc sFTRemixYoshiEggOwnRow;
    f32 row[FTREMIX_YOSHI_EGG_ROW_FLOATS];

    if (ftRemixYoshiSharedRulesGetEggRow(fkind, row) == FALSE)
    {
        return (ftCommonYoshiEggDesc*)port_fighter_yoshi_egg_damage_coll(fkind);
    }
    sFTRemixYoshiEggOwnRow.effect_size = row[0];
    sFTRemixYoshiEggOwnRow.offset.x = row[1];
    sFTRemixYoshiEggOwnRow.offset.y = row[2];
    sFTRemixYoshiEggOwnRow.offset.z = row[3];
    sFTRemixYoshiEggOwnRow.size.x = row[4];
    sFTRemixYoshiEggOwnRow.size.y = row[5];
    sFTRemixYoshiEggOwnRow.size.z = row[6];

    return &sFTRemixYoshiEggOwnRow;
}

#endif
