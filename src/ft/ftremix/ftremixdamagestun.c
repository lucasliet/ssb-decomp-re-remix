/*
 * ftremixdamagestun.c - Smash Remix stun, deku stun, flashbang and bury damage actions (.toolchain/pkgs/smashremix-src/src/Damage.asm).
 * ftCommonDamageGotoDamageStatus calls ftRemixDamageGotoStunAction before the sleep check. The electric status choice of
 * ftCommonDamageInitDamageVars calls ftRemixDamageUsesElectricStatus (stun_electric_action_, 0x801412E8).
 * Bury (ftremixbury.c) is started through ftRemixDamageBuryOrPlungeInitial.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <mp/mpcommon.h>
#include "ft/ftremix/ftremixbury.h"
#include "ft/ftremix/ftremixhooks.h"

/* Damage.asm: Action.Stun (0xA4) is the Fura Fura status, nFTCommonStatusFuraFura */
#define FTREMIX_STUN_ACTION nFTCommonStatusFuraFura
/* Damage.asm: the fighter colour animation of the flashbang overlay (argument 0x25) */
#define FTREMIX_FLASHBANG_OVERLAY_COLANIM 0x25
/* Damage.asm: the fighter colour animation of the deku stun (argument 0x6C, the DEKU_STUN GFX routine) */
#define FTREMIX_DEKU_STUN_COLANIM 0x6C
/* Damage.asm: the breakout wait of deku stun, 0x5A (90) added to the damage based wait */
#define FTREMIX_DEKU_STUN_BREAKOUT_MIN 0x5A
/* Damage.asm: shield_health (0x34) is set to 30 on every stun action */
#define FTREMIX_STUN_SHIELD_HEALTH 30
/* Damage.asm flashbang_initial_modified_: 90 base frames, 150 extra grounded, 0 extra aerial */
#define FTREMIX_FLASHBANG_BASE_STUN_TIME 90
#define FTREMIX_FLASHBANG_STUN_TIME_GROUNDED 150
#define FTREMIX_FLASHBANG_STUN_TIME_AERIAL 0

/**
 * Breakout wait from the fighter damage: the default wait minus the percent, never below zero.
 * The caller adds its own minimum, as the stun, deku stun and flashbang formulas differ.
 */
static s32 ftRemixDamageDefaultBreakoutWait(FTStruct *fp)
{
    s32 breakout_wait = FTCOMMON_FURAFURA_BREAKOUT_WAIT_DEFAULT - fp->percent_damage;

    if (breakout_wait <= 0)
    {
        return 0;
    }
    return breakout_wait;
}

/**
 * TRUE when the fighter is already in the stun action. Damage.asm compares the current action id (0xA4) to skip the
 * change of action.
 */
static sb32 ftRemixDamageIsInStunAction(FTStruct *fp)
{
    return fp->status_id == FTREMIX_STUN_ACTION;
}

/**
 * Stun damage on a grounded fighter facing the hit (Damage.asm stun_initial_modified_): the Fura Fura status is set
 * without the flags of ftCommonFuraFuraSetStatus, so the fighter keeps no texture or model part from the previous status.
 */
static void ftRemixDamageStunSetStatus(GObj *fighter_gobj, FTStruct *fp)
{
    s32 breakout_wait;

    ftMainSetStatus(fighter_gobj, FTREMIX_STUN_ACTION, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    fp->shield_health = FTREMIX_STUN_SHIELD_HEALTH;

    breakout_wait = ftRemixDamageDefaultBreakoutWait(fp) + FTCOMMON_FURAFURA_BREAKOUT_WAIT_MIN;
    ftCommonCaptureTrappedInitBreakoutVars(fp, ftRemixFuraFuraBreakoutWait(breakout_wait));
    ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterFuraFura, 0);
}

/**
 * Stun damage on a fighter that is not already stunned, when the hit faces the fighter and the fighter is grounded
 * (Damage.asm stun_damage_action_, STUN branch).
 * @return TRUE when the stun action started. FALSE leaves the vanilla damage action to the caller.
 */
static sb32 ftRemixDamageGotoStun(GObj *fighter_gobj, FTStruct *fp)
{
    if (ftRemixDamageIsInStunAction(fp))
    {
        return FALSE;
    }
    if (fp->damage_lr != fp->lr)
    {
        return FALSE;
    }
    if (fp->ga == nMPKineticsAir)
    {
        return FALSE;
    }
    ftRemixDamageStunSetStatus(fighter_gobj, fp);
    return TRUE;
}

/**
 * Deku stun damage (Damage.asm deku_stun_initial_modified_): the stun action on a grounded fighter, ShieldBreakFall in
 * the air. Does nothing while the fighter is already stunned, so the hit is ignored.
 */
static void ftRemixDamageDekuStunSetStatus(GObj *fighter_gobj, FTStruct *fp)
{
    s32 breakout_wait;
    s32 status_id = (fp->ga == nMPKineticsAir) ? nFTCommonStatusShieldBreakFall : FTREMIX_STUN_ACTION;

    if (ftRemixDamageIsInStunAction(fp))
    {
        return;
    }
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 0.0F, FTSTATUS_PRESERVE_NONE);
    fp->shield_health = FTREMIX_STUN_SHIELD_HEALTH;

    breakout_wait = ftRemixDamageDefaultBreakoutWait(fp) + FTREMIX_DEKU_STUN_BREAKOUT_MIN;
    ftCommonCaptureTrappedInitBreakoutVars(fp, breakout_wait);
    ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_DEKU_STUN_COLANIM, 0);
}

/**
 * Halves the self-induced air velocity of a flashbanged fighter in the air (Damage.asm flashbang_initial_modified_).
 */
static void ftRemixDamageFlashbangHalveAirVelocity(FTStruct *fp)
{
    fp->physics.vel_air.x *= 0.5F;
    fp->physics.vel_air.y *= 0.5F;
}

/**
 * One frame of the aerial flashbang stun (Damage.asm stun_main_aerial_): the breakout wait counts down, the mash inputs
 * of ftCommonCaptureTrappedUpdateBreakoutVars add time, and the fighter falls when the wait ends.
 * The end test uses the value after the mash and before the x3 scaling, as the asm does.
 */
static void ftRemixDamageStunMainAerial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 breakout_wait_counted;
    s32 breakout_wait_mashed;

    fp->breakout_wait--;
    breakout_wait_counted = fp->breakout_wait;

    ftCommonCaptureTrappedUpdateBreakoutVars(fp);
    breakout_wait_mashed = fp->breakout_wait;

    fp->breakout_wait = breakout_wait_mashed + (breakout_wait_mashed - breakout_wait_counted) * 3;

    if (breakout_wait_mashed > 0)
    {
        return;
    }
    ftCommonFallSetStatus(fighter_gobj);
}

/**
 * Flashbang damage (Damage.asm flashbang_initial_modified_): the stun action on the ground, or ShieldBreakFall in the air
 * with the aerial main routine that lets the fighter mash out. Does nothing while the fighter is already stunned.
 */
static void ftRemixDamageFlashbangSetStatus(GObj *fighter_gobj, FTStruct *fp)
{
    sb32 is_aerial = (fp->ga == nMPKineticsAir);
    s32 stun_time;
    s32 status_id = is_aerial ? nFTCommonStatusShieldBreakFall : FTREMIX_STUN_ACTION;

    if (ftRemixDamageIsInStunAction(fp))
    {
        return;
    }
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    if (is_aerial)
    {
        fp->proc_update = ftRemixDamageStunMainAerial;
        stun_time = FTREMIX_FLASHBANG_STUN_TIME_AERIAL;
        ftRemixDamageFlashbangHalveAirVelocity(fp);
    }
    else
    {
        stun_time = FTREMIX_FLASHBANG_STUN_TIME_GROUNDED;
    }
    fp->shield_health = FTREMIX_STUN_SHIELD_HEALTH;

    ftCommonCaptureTrappedInitBreakoutVars(fp, stun_time + fp->percent_damage + FTREMIX_FLASHBANG_BASE_STUN_TIME);
    ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_FLASHBANG_OVERLAY_COLANIM, 0);
}

/**
 * Bury damage (Damage.asm bury_branch): the bury starts unless the fighter is already buried. The caller must pass
 * the buried state, see ftRemixDamageBuryOrPlungeInitial in ftremixbury.h.
 */
static void ftRemixDamageBuryStart(GObj *fighter_gobj, FTStruct *fp)
{
    ftRemixDamageBuryOrPlungeInitial(fighter_gobj, !ftRemixDamageIsBuried(fp));
}

/**
 * Starts the stun, deku stun, flashbang or bury action of a Remix damage type, in place of the sleep and common damage
 * branch of ftCommonDamageGotoDamageStatus (Damage.asm stun_damage_action_, 0x8014158C).
 * @return TRUE when the damage type was handled here. FALSE sends the caller to the vanilla damage status.
 */
sb32 ftRemixDamageGotoStunAction(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    switch (fp->damage_element)
    {
    case FTREMIX_DAMAGE_STUN:
        return ftRemixDamageGotoStun(fighter_gobj, fp);

    case FTREMIX_DAMAGE_DEKU_STUN:
        ftRemixDamageDekuStunSetStatus(fighter_gobj, fp);
        return TRUE;

    case FTREMIX_DAMAGE_FLASHBANG:
        ftRemixDamageFlashbangSetStatus(fighter_gobj, fp);
        return TRUE;

    case FTREMIX_DAMAGE_BURY:
        ftRemixDamageBuryStart(fighter_gobj, fp);
        return TRUE;

    default:
        return FALSE;
    }
}

/**
 * TRUE when the damage type takes the electric status path of ftCommonDamageInitDamageVars (Damage.asm stun_electric_action_,
 * 0x801412E8). In the asm, STUN, DEKU_STUN and BURY take the vanilla branch, and only ELECTRIC and FLASHBANG take the
 * electric branch.
 */
sb32 ftRemixDamageUsesElectricStatus(s32 element)
{
    return (element == nGMHitElementElectric) || (element == FTREMIX_DAMAGE_FLASHBANG);
}

#endif
