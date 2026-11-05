/**
 * wpfoxremixblaster.c - J Fox laser file (JFox.asm get_laser_special_struct_, 0x80168A84).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "wp/wpfox/wpfoxremixblaster.h"
#include "fighter_registry.h"
#include "remix/remix_roster_data.h"

/* Kirby and J Kirby shoot with the power they copied (N64 0x0ADC). */
static s32 wpRemixFoxBlasterShooterKind(FTStruct *fp)
{
    if ((fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return fp->passive_vars.kirby.copy_id;
    }
    return fp->fkind;
}

void wpRemixFoxBlasterSetFile(FTStruct *fp, WPDesc *desc)
{
    FTData *data;

    if (wpRemixFoxBlasterShooterKind(fp) != REMIX_ID_JFOX)
    {
        return;
    }
    data = port_fighter_data(REMIX_ID_JFOX);

    if ((data == NULL) || (data->p_file_special1 == NULL) || (*data->p_file_special1 == NULL))
    {
        return;
    }
    desc->p_weapon = data->p_file_special1;
}

#endif
