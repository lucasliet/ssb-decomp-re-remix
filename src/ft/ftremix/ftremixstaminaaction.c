/**
 * ftremixstaminaaction.c - Smash Remix Stamina forced KO action (Stamina.asm stamina_ko_action).
 *
 * A fighter out of HP that changes status goes to the KO action instead: DownBounceD on the ground, Tumble in the air.
 * The statuses the asm lets play out (dead, damage, down bounce, cliff, grab and capture, DK and Marina cargo) are kept.
 * Called from ftMainSetStatus as the last step of the status id mapping.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixstamina.h"
#include "mp/mpdef.h"
#include "remix/remix_roster_data.h"

/* Action.asm DK Cargo (0xEB) to CargoThrowAir (0xF5). The decomp names are nFTDonkeyStatusThrowFWait and
 * nFTDonkeyStatusThrowAirFF (ft/ftchar/ftdonkey/ftdonkey.h). That header cannot be included here (it needs FTStatusDesc
 * first), so the values are local, as in ftremixmarinansp.c. */
enum
{
    nFTRemixStaminaDonkeyCargoFirst = 0xEB,
    nFTRemixStaminaDonkeyCargoLast = 0xF5
};

/* Action.asm Marina.Action.Cargo (0xEB) to CargoShake (0xFB). The decomp names only 0xFA and 0xFB (ftremixmarinansp.c) */
enum
{
    nFTRemixStaminaMarinaCargoFirst = 0xEB,
    nFTRemixStaminaMarinaCargoLast = 0xFB
};

/**
 * TRUE for the dead statuses the asm lets play out: DeadD (0x000) to ScreenKOWait (0x004). ScreenKOWait has the value
 * of nFTCommonStatusSleep in the decomp.
 */
static sb32 ftRemixStaminaIsDeadStatus(s32 status_id)
{
    return ftRemixStaminaStatusInRange(status_id, nFTCommonStatusDeadDown, nFTCommonStatusSleep);
}

/**
 * TRUE for the damage statuses the asm lets play out (0x025 to 0x038). Tumble (0x039, decomp DamageFall) is not one of
 * them, so the KO action itself can still be set.
 */
static sb32 ftRemixStaminaIsDamageStatus(s32 status_id)
{
    return ftRemixStaminaStatusInRange(status_id, nFTCommonStatusDamageStart, nFTCommonStatusDamageEnd);
}

/**
 * TRUE for the down statuses the asm lets play out: DownBounceD to DownWaitU (0x043 to 0x046).
 */
static sb32 ftRemixStaminaIsDownStatus(s32 status_id)
{
    return ftRemixStaminaStatusInRange(status_id, nFTCommonStatusDownBounceD, nFTCommonStatusDownWaitU);
}

/**
 * TRUE for the cliff statuses the asm lets play out: CliffCatch to CliffEscapeSlow1 (0x054 to 0x062). The _cliff loop of
 * Stamina.asm stops when its counter reaches CliffEscapeSlow2 before testing it.
 */
static sb32 ftRemixStaminaIsCliffStatus(s32 status_id)
{
    return ftRemixStaminaStatusInRange(status_id, nFTCommonStatusCliffCatch, nFTCommonStatusCliffEscapeSlow1);
}

/**
 * TRUE for the grab and captured statuses the asm lets play out: Catch to ThrownFoxB (0x0A6 to 0x0BC).
 */
static sb32 ftRemixStaminaIsGrabOrCaptureStatus(s32 status_id)
{
    return ftRemixStaminaStatusInRange(status_id, nFTCommonStatusCatch, nFTCommonStatusThrownFoxB);
}

/**
 * TRUE when `status_id` is one of the common statuses the asm lets play out (Tornado and Barrel are single statuses).
 */
static sb32 ftRemixStaminaIsCommonProtectedStatus(s32 status_id)
{
    return ftRemixStaminaIsDeadStatus(status_id) || ftRemixStaminaIsDamageStatus(status_id) ||
           ftRemixStaminaIsDownStatus(status_id) || ftRemixStaminaIsCliffStatus(status_id) ||
           ftRemixStaminaIsGrabOrCaptureStatus(status_id) || (status_id == nFTCommonStatusTwister) ||
           (status_id == nFTCommonStatusTaruCann);
}

/**
 * TRUE when the fighter is DK or Japanese DK and `status_id` is in the DK cargo range (0x0EB to 0x0F5, up to but not
 * including DK HeavyItemThrowF, as in Stamina.asm).
 */
static sb32 ftRemixStaminaIsDonkeyCargoStatus(FTStruct *fp, s32 status_id)
{
    if ((fp->fkind != REMIX_ID_DONKEY) && (fp->fkind != REMIX_ID_JDK))
    {
        return FALSE;
    }
    return ftRemixStaminaStatusInRange(status_id, nFTRemixStaminaDonkeyCargoFirst, nFTRemixStaminaDonkeyCargoLast);
}

/**
 * TRUE when the fighter is Marina and `status_id` is in her cargo range (0x0EB to 0x0FB).
 */
static sb32 ftRemixStaminaIsMarinaCargoStatus(FTStruct *fp, s32 status_id)
{
    if (fp->fkind != REMIX_ID_MARINA)
    {
        return FALSE;
    }
    return ftRemixStaminaStatusInRange(status_id, nFTRemixStaminaMarinaCargoFirst, nFTRemixStaminaMarinaCargoLast);
}

/**
 * TRUE when the asm lets `status_id` play out for this fighter, so the KO action does not replace it.
 */
static sb32 ftRemixStaminaIsProtectedStatus(FTStruct *fp, s32 status_id)
{
    return ftRemixStaminaIsCommonProtectedStatus(status_id) || ftRemixStaminaIsDonkeyCargoStatus(fp, status_id) ||
           ftRemixStaminaIsMarinaCargoStatus(fp, status_id);
}

/**
 * TRUE when the Stamina rule is active and the fighter is out of HP: defeated, or its percent is above the total HP.
 */
static sb32 ftRemixStaminaIsKOPending(FTStruct *fp)
{
    if (gmRemixStaminaIsActive() == FALSE)
    {
        return FALSE;
    }
    return (gmRemixStaminaIsDefeated(fp->player) || ftRemixStaminaIsOutOfHP(fp)) ? TRUE : FALSE;
}

/**
 * KO status for the fighter's current ground state: DownBounceD on the ground, Tumble (decomp DamageFall) in the air.
 */
static s32 ftRemixStaminaKOStatusFor(FTStruct *fp)
{
    if (fp->ga == nMPKineticsGround)
    {
        return nFTCommonStatusDownBounceD;
    }
    return nFTCommonStatusDamageFall;
}

/**
 * Status a fighter goes to when `status_id` is requested by ftMainSetStatus (Stamina.asm stamina_ko_action).
 *
 * Returns `status_id` unchanged when the rule is off, the fighter is in HP, or the status is one the asm lets play out.
 * REMIX-TODO(stamina-gigabowser): the 1P Giga Bowser branch of stamina_ko_action (200, 250 or 300 HP by difficulty) is
 * not ported yet; it belongs to T6.2.
 */
s32 ftRemixStaminaKOStatus(FTStruct *fp, s32 status_id)
{
    if (ftRemixStaminaIsKOPending(fp) == FALSE)
    {
        return status_id;
    }
    if (ftRemixStaminaIsProtectedStatus(fp, status_id))
    {
        return status_id;
    }
    return ftRemixStaminaKOStatusFor(fp);
}

#endif
