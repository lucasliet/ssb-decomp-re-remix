/**
 * ftremixstaminaguard.c - Smash Remix Stamina guards that stop a fighter out of HP from acting (Stamina.asm
 * stamina_shield_prevent, stamina_armor_fix_, stamina_invis_prevent and egg_crash_prevent).
 *
 * A fighter is out of HP when its damage percent is above the total HP (Stamina.asm TOTAL_HP). Each guard checks the
 * Stamina rule first, so the other rules keep the vanilla behaviour.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixstamina.h"
#include "gm/gmremixstaminarules.h"
#include "remix/remix_roster_data.h"

/* Action.asm ids read by stamina_invis_prevent and egg_crash_prevent */
#define FTREMIX_STAMINA_ACTION_FALCON_DIVE_PULLED 0x0B3

/* Roy.asm DSP_G_Strong_End and DSP_A_Strong_End, read by stamina_invis_prevent for Roy only */
#define FTREMIX_STAMINA_ROY_DSP_G_STRONG_END 0x0F2
#define FTREMIX_STAMINA_ROY_DSP_A_STRONG_END 0x0F6

/**
 * TRUE when the Stamina rule is active and the damage percent is above the total HP (Stamina.asm TOTAL_HP).
 */
sb32 ftRemixStaminaIsOutOfHP(FTStruct *fp)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    return gmRemixStaminaRulesIsOverTotal(fp->percent_damage, gmRemixStaminaGetTotalHP()) ? TRUE : FALSE;
}

/**
 * TRUE when the Stamina rule is active and the damage percent plus the damage queued this frame is above the total HP.
 * passive_armor_ (RAM 0x800E6324) then returns no armor (Stamina.asm stamina_armor_fix_, called from Size.asm).
 */
sb32 ftRemixStaminaArmorIsZeroed(FTStruct *fp)
{
    s32 total_hp;

    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    total_hp = gmRemixStaminaGetTotalHP();
    return gmRemixStaminaRulesIsOverTotal(fp->percent_damage + fp->damage_queue, total_hp) ? TRUE : FALSE;
}

/**
 * TRUE when the fighter is in an action that Stamina.asm stamina_invis_prevent keeps visible: the throw and capture
 * actions, the Falcon dive pull up to Thrown Fox, and Roy's down special strong ends.
 */
static sb32 sFTRemixStaminaIsInvisibleExcludedAction(FTStruct *fp)
{
    s32 status_id = fp->status_id;

    if ((status_id == nFTCommonStatusDamageFlyN) || (status_id == nFTCommonStatusCapturePulled) ||
        (status_id == nFTCommonStatusCaptureWait))
    {
        return TRUE;
    }
    if (ftRemixStaminaStatusInRange(status_id, FTREMIX_STAMINA_ACTION_FALCON_DIVE_PULLED, nFTCommonStatusThrownFoxB))
    {
        return TRUE;
    }
    if (fp->fkind != REMIX_ID_ROY)
    {
        return FALSE;
    }
    if ((status_id == FTREMIX_STAMINA_ROY_DSP_G_STRONG_END) || (status_id == FTREMIX_STAMINA_ROY_DSP_A_STRONG_END))
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * TRUE when the KO must not set the invisible flag: the Stamina rule is active and the fighter is in an action that
 * stamina_invis_prevent excludes. The asm sets the flag in every other case.
 */
sb32 ftRemixStaminaSkipsInvisible(FTStruct *fp)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    return sFTRemixStaminaIsInvisibleExcludedAction(fp);
}

/**
 * TRUE when the KO must not write the dead wait: the Stamina rule is active and the fighter is in the egg lay action
 * (Stamina.asm egg_crash_prevent, a crash when a fighter is killed during the egg lay).
 */
sb32 ftRemixStaminaSkipsDeadWaitInit(FTStruct *fp)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    return (fp->status_id == nFTCommonStatusYoshiEgg) ? TRUE : FALSE;
}

#endif
