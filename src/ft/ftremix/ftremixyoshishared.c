/**
 * ftremixyoshishared.c - Smash Remix yoshishared.asm: Yoshi's egg and star weapons
 * read their model and attributes from a hardcoded file pointer, so the clones that
 * reuse them need a descriptor that points at their own file.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixhooks.h"
#include <wp/weapon.h>

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

#endif
