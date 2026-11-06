/**
 * ftremixstaminaaction.c - Smash Remix Stamina forced KO action (Stamina.asm stamina_ko_action).
 *
 * A fighter out of HP that changes status goes to the KO action instead: DownBounceD on the ground, Tumble in the air.
 * The statuses the asm lets play out (dead, damage, down bounce, cliff, grab and capture, DK and Marina cargo) are kept.
 * Called from ftMainSetStatus as the last step of the status id mapping.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixstaminaaction.h"
#include "gm/gmremixstamina.h"
#include "mp/mpdef.h"
#include "remix/remix_roster_data.h"
#include "sc/scmanager.h"
#include "sc/sc1pmode/sc1premix1p.h"

/* Stamina.asm _remix branch: Giga Bowser HP by 1P difficulty (below Normal, Normal and Hard, Very Hard) */
#define FTREMIX_STAMINA_GIGA_HP_LOW 200
#define FTREMIX_STAMINA_GIGA_HP_MID 250
#define FTREMIX_STAMINA_GIGA_HP_HIGH 300
#define FTREMIX_STAMINA_KO_ANIM_SPEED 1.0F

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
 * Giga Bowser HP of the 1P Remix stamina rule (Stamina.asm _remix): 200 below Normal difficulty, 250 for Normal and
 * Hard, 300 for Very Hard. Reads the 1P difficulty of the save (0x800A493A in the asm).
 */
static s32 ftRemixStaminaGigaBowserTotalHP(void)
{
    u8 difficulty = gSCManagerBackupData.spgame_difficulty;

    if (difficulty < nSC1PGameDifficultyNormal)
    {
        return FTREMIX_STAMINA_GIGA_HP_LOW;
    }
    if (difficulty < nSC1PGameDifficultyVeryHard)
    {
        return FTREMIX_STAMINA_GIGA_HP_MID;
    }
    return FTREMIX_STAMINA_GIGA_HP_HIGH;
}

/**
 * TRUE when the fighter is a CPU Giga Bowser of the 1P Remix game whose percent is above its HP (Stamina.asm _remix
 * branch). That branch runs outside VS mode, so the VS stamina rule does not apply to it.
 */
static sb32 ftRemixStaminaIsGigaBowserOutOfHP(FTStruct *fp)
{
    if (sc1PRemix1PIsActive() == FALSE)
    {
        return FALSE;
    }
    if ((fp->pkind != nFTPlayerKindCom) || (fp->fkind != REMIX_ID_GBOWSER))
    {
        return FALSE;
    }
    return (fp->percent_damage > ftRemixStaminaGigaBowserTotalHP()) ? TRUE : FALSE;
}

/**
 * TRUE when ftMainSetStatus must replace the requested status with the KO action: a stamina KO is pending and the asm does
 * not let `status_id` play out.
 */
static sb32 ftRemixStaminaForcesKO(FTStruct *fp, s32 status_id)
{
    if ((ftRemixStaminaIsKOPending(fp) == FALSE) && (ftRemixStaminaIsGigaBowserOutOfHP(fp) == FALSE))
    {
        return FALSE;
    }
    return (ftRemixStaminaIsProtectedStatus(fp, status_id) == FALSE) ? TRUE : FALSE;
}

/**
 * Status a fighter goes to when `status_id` is requested by ftMainSetStatus (Stamina.asm stamina_ko_action).
 *
 * Returns `status_id` unchanged when no stamina KO is pending (VS stamina rule or 1P Giga Bowser), or the status is one
 * the asm lets play out.
 */
s32 ftRemixStaminaKOStatus(FTStruct *fp, s32 status_id)
{
    if (ftRemixStaminaForcesKO(fp, status_id) == FALSE)
    {
        return status_id;
    }
    return ftRemixStaminaKOStatusFor(fp);
}

/**
 * Animation speed of the status ftMainSetStatus is about to set (Stamina.asm stamina_ko_action, _corrections: lui a3, 0x3F80).
 * The forced KO action always plays at normal speed, whatever speed the replaced status was requested with.
 *
 * @param fp Fighter struct.
 * @param status_id Status requested, before ftRemixStaminaKOStatus.
 * @param anim_speed Animation speed requested.
 * @return 1.0 when the KO action replaces the request (also when it is the requested status itself), otherwise anim_speed.
 */
f32 ftRemixStaminaKOAnimSpeed(FTStruct *fp, s32 status_id, f32 anim_speed)
{
    if (ftRemixStaminaForcesKO(fp, status_id) == FALSE)
    {
        return anim_speed;
    }
    return FTREMIX_STAMINA_KO_ANIM_SPEED;
}

#endif
