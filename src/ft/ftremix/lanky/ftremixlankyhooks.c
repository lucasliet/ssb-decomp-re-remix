/**
 * ftremixlankyhooks.c - Smash Remix patches that Lanky's up and down specials
 * make in vanilla damage and 1P bonus code (LankySpecial.asm).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"

#define LANKY_STATUS_USP_GROUND_BEGIN 0xE5
#define LANKY_STATUS_USP_AIR_BEGIN 0xE6
#define LANKY_STATUS_USP_MOVE 0xE7
#define LANKY_STATUS_USP_TURN 0xE8
#define LANKY_STATUS_USP_DAMAGE 0xEA
#define LANKY_STATUS_DSP_TAUNT 0xF1 /* OrangStandTaunt */

static sb32 ftRemixLankyIsUpSpecialStatus(s32 status_id)
{
    return (status_id == LANKY_STATUS_USP_GROUND_BEGIN) || (status_id == LANKY_STATUS_USP_AIR_BEGIN) ||
           (status_id == LANKY_STATUS_USP_TURN) || (status_id == LANKY_STATUS_USP_MOVE);
}

static sb32 ftRemixLankyIsDamageFlyStatus(s32 status_id)
{
    return (status_id >= nFTCommonStatusDamageFlyHi) && (status_id <= nFTCommonStatusDamageFlyRoll);
}

/* LankyUSP.damage_patch_ (0x801413A4): a fly hit out of the up special enters
 * USPDamage instead. For electric damage the replaced status is the saved one. */
void ftRemixLankyUpSpecialDamageStatus(FTStruct *fp, s32 *status_id_set, s32 *status_id_var)
{
    if ((fp->fkind != REMIX_ID_LANKY) || !ftRemixLankyIsUpSpecialStatus(fp->status_id))
    {
        return;
    }
    if (*status_id_set == nFTCommonStatusDamageE2)
    {
        if (ftRemixLankyIsDamageFlyStatus(*status_id_var))
        {
            *status_id_var = LANKY_STATUS_USP_DAMAGE;
        }
        return;
    }
    if (ftRemixLankyIsDamageFlyStatus(*status_id_set))
    {
        *status_id_set = LANKY_STATUS_USP_DAMAGE;
    }
}

/* Lanky.lanky_fighter_stance_bonus_ (0x80190650): the down special taunt counts
 * as a fighter stance in 1P modes. */
sb32 ftRemixLankyIsStanceBonus(s32 fkind, s32 end_status_id)
{
    return (fkind == REMIX_ID_LANKY) && (end_status_id == LANKY_STATUS_DSP_TAUNT);
}

#endif
