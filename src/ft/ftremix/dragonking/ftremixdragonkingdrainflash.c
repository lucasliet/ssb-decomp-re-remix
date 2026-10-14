/**
 * ftremixdragonkingdrainflash.c - Dragon King up special drain flash fix for the damage percent digits
 * (.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm, scope drain_flash_fix_, RAM 0x8010E9E4).
 *
 * The vanilla damage digits flash white when the damage rises. While a Dragon King drains the player it
 * captured with its up special (temp variable 1 set), the flash is suppressed so the digits keep their colour.
 * ifCommonPlayerDamageUpdateDigits (if/ifcommon.c) calls ftRemixDragonKingDrainFlashSuppressed.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"

enum
{
    nFTRemixDragonKingDrainFlashStatusUSPGroundThrow = 0xF0,
    nFTRemixDragonKingDrainFlashStatusUSPAirPull = 0xF2
};

/* Returns the fighter that captured fighter_gobj, or NULL when there is no capture. */
static FTStruct* ftRemixDragonKingDrainFlashCapturer(GObj *fighter_gobj)
{
    FTStruct *fp;

    if (fighter_gobj == NULL)
    {
        return NULL;
    }
    fp = ftGetStruct(fighter_gobj);
    if ((fp == NULL) || (fp->capture_gobj == NULL))
    {
        return NULL;
    }
    return ftGetStruct(fp->capture_gobj);
}

/* True for the two Dragon King up special actions that drain the captured player (asm: USPGroundThrow, USPAirPull). */
static sb32 ftRemixDragonKingDrainFlashIsUSPAction(s32 status_id)
{
    return (status_id == nFTRemixDragonKingDrainFlashStatusUSPGroundThrow)
        || (status_id == nFTRemixDragonKingDrainFlashStatusUSPAirPull);
}

/* drain_flash_fix_: TRUE when the digits of fighter_gobj keep their normal colour instead of the white flash.
 * The asm reads the capturer's temp variable 1 (0x17C), which the Dragon King sets to 1 while it drains. */
sb32 ftRemixDragonKingDrainFlashSuppressed(GObj *fighter_gobj)
{
    FTStruct *capturer_fp;

    capturer_fp = ftRemixDragonKingDrainFlashCapturer(fighter_gobj);
    if (capturer_fp == NULL)
    {
        return FALSE;
    }
    if (capturer_fp->fkind != REMIX_ID_DRAGONKING)
    {
        return FALSE;
    }
    if (!ftRemixDragonKingDrainFlashIsUSPAction(capturer_fp->status_id))
    {
        return FALSE;
    }
    return capturer_fp->motion_vars.flags.flag0 != 0;
}

#endif
