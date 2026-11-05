/**
 * ftremixchargeflash.c - Remix gfx_routine_end table run from ftParamResetStatUpdateColAnim.
 *
 * When a color animation ends (hit flash, invincibility flash, action change) the vanilla routine starts the Samus / Donkey
 * Kong / Ness glows again. Character.asm replaces that switch with a per-id jump table (get_gfx_routine_end_,
 * 0x800E9948); a new character inherits the entry of its parent unless its own file patches it.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixchargeflashrules.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixparamhooks.h"
#include "ft/ftparam.h"
#include "gm/gmdef.h"

static s32 ftRemixChargeFlashColAnimID(const FTRemixChargeFlash *flash)
{
    if (flash->colanim_id == FTREMIX_CHARGE_FLASH_COMMON)
    {
        return nGMColAnimFighterCommonSpecialNCharge;
    }
    return flash->colanim_id;
}

/* The inherited Ness entry: PSI Magnet glow while the fighter absorbs. */
static void ftRemixRestartAbsorbFlash(GObj *fighter_gobj, FTStruct *fp)
{
    if (fp->is_absorb)
    {
        ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterNessSpecialLwHold, 0);
    }
}

/**
 * Starts the charge glow of the fighter again when its charge word is at the fully charged level.
 *
 * @param fighter_gobj Fighter whose color animation was just reset.
 */
void ftRemixRestartChargeFlash(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixChargeFlash flash;

    if (fp->fkind == REMIX_ID_JNESS)
    {
        ftRemixRestartAbsorbFlash(fighter_gobj, fp);
        return;
    }
    if (!ftRemixChargeFlashFind(fp->fkind, &flash))
    {
        return;
    }
    if (*ftRemixFighterWord(fp, flash.n64_offset) != flash.full_level)
    {
        return;
    }
    ftParamCheckSetFighterColAnimID(fighter_gobj, ftRemixChargeFlashColAnimID(&flash), 0);
}

#endif
