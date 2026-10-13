/**
 * ftremixsamusshared.c - Smash Remix samusshared.asm (scope SamusShared): the
 * Samus clones (Dark, J and E Samus) fire their own Charge Shot and Bomb
 * models. Remix swaps the WPDesc for one whose file pointer is the clone's
 * file; the port keeps the vanilla WPDesc and swaps only that pointer.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixhooks.h"
#include "fighter_registry.h"

static sb32 ftRemixSamusSharedIsClone(s32 fkind)
{
    return (fkind == REMIX_ID_DSAMUS) || (fkind == REMIX_ID_JSAMUS) || (fkind == REMIX_ID_ESAMUS);
}

static sb32 ftRemixSamusSharedIsKirby(s32 fkind)
{
    return (fkind == nFTKindKirby) || (fkind == REMIX_ID_JKIRBY);
}

/* get_chargeinstructions_struct_ (0x80168E14): `charge_anim_struct*` uses the
 * clone's file 7. Kirby and J Kirby use the clone whose power they hold. */
void **ftRemixSamusSharedChargeShotFile(FTStruct *fp, void **vanilla_file)
{
    s32 fkind = fp->fkind;
    FTData *data;

    if (ftRemixSamusSharedIsKirby(fkind))
    {
        fkind = fp->passive_vars.kirby.copy_id;
    }
    if (!ftRemixSamusSharedIsClone(fkind))
    {
        return vanilla_file;
    }
    data = port_fighter_data(fkind);

    if ((data == NULL) || (data->p_file_special2 == NULL) || (*data->p_file_special2 == NULL))
    {
        return vanilla_file;
    }
    return data->p_file_special2;
}

/* get_bombinstructions_struct_ (0x80169334): `bomb_anim_struct*` uses the
 * clone's file 1. */
void **ftRemixSamusSharedBombFile(FTStruct *fp, void **vanilla_file)
{
    if (!ftRemixSamusSharedIsClone(fp->fkind) || (fp->data->p_file_main == NULL) || (*fp->data->p_file_main == NULL))
    {
        return vanilla_file;
    }
    return fp->data->p_file_main;
}

#endif
