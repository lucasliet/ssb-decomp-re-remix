/*
 * Smash Remix Z-Cancel settings (ZCancel.asm, Practice.asm flash_on_z_cancel_): the landing of an aerial attack after the
 * Z-cancel check. Called from ftCommonAttackAirProcMap under REGION_REMIX (decomp/src/ft/ftcommon/ftcommonattackair.c).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixbury.h"
#include "gm/gmremixtoggle.h"
#include "enhancements/enhancements.h"
#include <ft/fighter.h>
#include <sys/utils.h>
#include "ft/ftremix/ftremixaiteching.h"
#include "mn/mnvsmode/mnremixvsstats.h"
#include "sc/scmanager.h"
#include "sc/sctypes.h"

extern void *func_800269C0_275C0(u16 id);

#define FTREMIX_ZCANCEL_OPT_DISABLED 1
#define FTREMIX_ZCANCEL_OPT_MELEE 2
#define FTREMIX_ZCANCEL_OPT_AUTO 3
#define FTREMIX_ZCANCEL_OPT_GLIDE 4

#define FTREMIX_ZCANCEL_WINDOW_DEFAULT 11
#define FTREMIX_ZCANCEL_WINDOW_MELEE 7

#define FTREMIX_ZCANCEL_RESULT_SUCCESS 0
#define FTREMIX_ZCANCEL_RESULT_FAIL 1
#define FTREMIX_ZCANCEL_RESULT_GLIDE 2

#define FTREMIX_ZCANCEL_PUNISH_OFF 0
#define FTREMIX_ZCANCEL_PUNISH_ON 1
#define FTREMIX_ZCANCEL_PUNISH_LAVA 2
#define FTREMIX_ZCANCEL_PUNISH_SHIELD_BREAK 3
#define FTREMIX_ZCANCEL_PUNISH_INSTANT_KO 4
#define FTREMIX_ZCANCEL_PUNISH_FORCE_TAUNT 5
#define FTREMIX_ZCANCEL_PUNISH_BURY 6
#define FTREMIX_ZCANCEL_PUNISH_LAUGH_TRACK 7
#define FTREMIX_ZCANCEL_PUNISH_EGG 8
#define FTREMIX_ZCANCEL_PUNISH_SLEEP 9
#define FTREMIX_ZCANCEL_PUNISH_TRIP 10
#define FTREMIX_ZCANCEL_PUNISH_RANDOM 11

/* Low halfword of coll_data.floor_flags (N64 0x0F6), the custom surface flags of ZCancel.asm _on and _lava */
#define FTREMIX_ZCANCEL_SURFACE_ON 0x0012
#define FTREMIX_ZCANCEL_SURFACE_LAVA 0x001E

/* 64.0F (0x42800000) */
#define FTREMIX_ZCANCEL_SHIELD_BREAK_VEL_Y 64.0F
#define FTREMIX_ZCANCEL_TRIP_HITSTUN_TICS 8

#define FTREMIX_ZCANCEL_SFX_LAUGH_TRACK 0x0524
#define FTREMIX_ZCANCEL_SFX_TRIPSTART 0x0456

/* Flash id 8 of Global.flash_ (red, from Mario's fireball) */
#define FTREMIX_ZCANCEL_FLASH_COLANIM 8
#define FTREMIX_ZCANCEL_FLASH_LENGTH 0

/* Sets the low halfword of coll_data.floor_flags and keeps the high halfword (ZCancel.asm _on, _lava). */
static void ftRemixZCancelSetSurfaceFlag(FTStruct *fp, u32 surface_flag)
{
    fp->coll_data.floor_flags = (fp->coll_data.floor_flags & 0xFFFF0000) | surface_flag;
}

/* ZCancel.asm _instant_ko. REMIX-TODO(1P final boss): Remix skips the KO in the 1P final stage while the match is off. */
static void ftRemixZCancelInstantKO(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->p_loop_sfx = NULL;
    fp->loop_sfx_id = 0;
    ftParamResetColAnim(&fp->colanim);
    ftCommonDeadDownSetStatus(fighter_gobj);
}

/* ZCancel.asm _egg. REMIX-TODO(ZCancel.setup_): the Yoshi egg file (0x153) that setup_ loads is not ported. */
static void ftRemixZCancelEgg(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->status_vars.common.dead.wait = 0;
    fp->unk_ft_0x7A0 = 0.0F;
    ftMainSetStatus(fighter_gobj, nFTCommonStatusYoshiEgg, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftCommonYoshiEggProcStatus(fighter_gobj);
    fp->is_invisible = TRUE;
}

/* ZCancel.asm _trip, Tripping.asm player_trip_flag store. */
static void ftRemixZCancelTrip(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->status_vars.common.damage.hitstun_tics = FTREMIX_ZCANCEL_TRIP_HITSTUN_TICS;
    ftMainSetStatus(fighter_gobj, nFTCommonStatusDamageLw3, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    func_800269C0_275C0(FTREMIX_ZCANCEL_SFX_TRIPSTART);
    ftRemixTrippingStoreZCancelFlag(fp->player);
}

/* Applies one punishment (ZCancel.asm jump_table). Returns TRUE when the punishment replaces the landing. */
static sb32 ftRemixZCancelPunishMode(GObj *fighter_gobj, s32 mode)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    switch (mode)
    {
        case FTREMIX_ZCANCEL_PUNISH_ON:
            ftRemixZCancelSetSurfaceFlag(fp, FTREMIX_ZCANCEL_SURFACE_ON);
            return FALSE;
        case FTREMIX_ZCANCEL_PUNISH_LAVA:
            ftRemixZCancelSetSurfaceFlag(fp, FTREMIX_ZCANCEL_SURFACE_LAVA);
            return FALSE;
        case FTREMIX_ZCANCEL_PUNISH_SHIELD_BREAK:
            ftCommonShieldBreakFlySetStatus(fighter_gobj);
            fp->physics.vel_air.y = FTREMIX_ZCANCEL_SHIELD_BREAK_VEL_Y;
            return TRUE;
        case FTREMIX_ZCANCEL_PUNISH_INSTANT_KO:
            ftRemixZCancelInstantKO(fighter_gobj);
            return TRUE;
        case FTREMIX_ZCANCEL_PUNISH_FORCE_TAUNT:
            ftCommonAppealSetStatus(fighter_gobj);
            return TRUE;
        case FTREMIX_ZCANCEL_PUNISH_BURY:
            ftRemixDamageBuryInitial(fighter_gobj);
            return TRUE;
        case FTREMIX_ZCANCEL_PUNISH_LAUGH_TRACK:
            func_800269C0_275C0(FTREMIX_ZCANCEL_SFX_LAUGH_TRACK);
            return FALSE;
        case FTREMIX_ZCANCEL_PUNISH_EGG:
            ftRemixZCancelEgg(fighter_gobj);
            return TRUE;
        case FTREMIX_ZCANCEL_PUNISH_SLEEP:
            ftCommonFuraSleepSetStatus(fighter_gobj);
            return TRUE;
        case FTREMIX_ZCANCEL_PUNISH_TRIP:
            ftRemixZCancelTrip(fighter_gobj);
            return TRUE;
        default:
            return FALSE;
    }
}

/**
 * TRUE on the how-to-play screen. ZCancel.asm _cruel_z_cancel reads the first byte of the match info
 * (gSCManagerBattleState->game_type) and compares it with 3, which is nSCBattleGameTypeExplain (scexplain.c).
 */
static sb32 ftRemixZCancelIsHowToPlay(void)
{
    return (gSCManagerBattleState->game_type == nSCBattleGameTypeExplain) ? TRUE : FALSE;
}

/* ZCancel.asm _cruel_z_cancel. Returns TRUE when the punishment replaces the landing. The how-to-play screen skips the
 * punishment (asm branch to _end), so no random roll happens there either. */
static sb32 ftRemixZCancelPunish(GObj *fighter_gobj)
{
    s32 mode = gmRemixToggle(punish_on_failed_z_cancel);

    if ((mode == FTREMIX_ZCANCEL_PUNISH_OFF) || ftRemixZCancelIsHowToPlay())
    {
        return FALSE;
    }
    if (mode == FTREMIX_ZCANCEL_PUNISH_RANDOM)
    {
        mode = syUtilsRandIntRange(FTREMIX_ZCANCEL_PUNISH_RANDOM - 1) + 1;
    }
    return ftRemixZCancelPunishMode(fighter_gobj, mode);
}

/* ZCancel.asm _z_cancel_opts for a player (the CPU branch needs AI). */
static s32 ftRemixZCancelResult(FTStruct *fp)
{
    s32 option = gmRemixToggle(z_cancel_opts);
    s32 window;

    if (ftRemixAiShouldZCancel(fp))
    {
        return (option == FTREMIX_ZCANCEL_OPT_GLIDE) ? FTREMIX_ZCANCEL_RESULT_GLIDE : FTREMIX_ZCANCEL_RESULT_SUCCESS;
    }
    if (option == FTREMIX_ZCANCEL_OPT_DISABLED)
    {
        return FTREMIX_ZCANCEL_RESULT_FAIL;
    }
    if (option == FTREMIX_ZCANCEL_OPT_AUTO)
    {
        return FTREMIX_ZCANCEL_RESULT_SUCCESS;
    }
    window = (option == FTREMIX_ZCANCEL_OPT_MELEE) ? FTREMIX_ZCANCEL_WINDOW_MELEE : FTREMIX_ZCANCEL_WINDOW_DEFAULT;
    if (fp->tics_since_last_z < window)
    {
        return FTREMIX_ZCANCEL_RESULT_SUCCESS;
    }
    if (option == FTREMIX_ZCANCEL_OPT_GLIDE)
    {
        return FTREMIX_ZCANCEL_RESULT_GLIDE;
    }
    return FTREMIX_ZCANCEL_RESULT_FAIL;
}

/* Flash On Z-Cancel (Practice.asm flash_on_z_cancel_): flashes a normal landing made inside the Z-cancel window. The
 * landing status is set first, as in the asm. */
static void ftRemixZCancelFlashOnLanding(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (gmRemixToggle(flash_on_z_cancel) == FALSE)
    {
        return;
    }
    if (fp->tics_since_last_z >= FTREMIX_ZCANCEL_WINDOW_DEFAULT)
    {
        return;
    }
    ftParamCheckSetFighterColAnimID(fighter_gobj, FTREMIX_ZCANCEL_FLASH_COLANIM, FTREMIX_ZCANCEL_FLASH_LENGTH);
}

/* Vanilla landing without a Z-cancel punishment: Wait when falling slowly, otherwise Landing (ftCommonAttackAirProcMap). */
static void ftRemixZCancelNormalLanding(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->physics.vel_air.y > FTCOMMON_ATTACKAIR_SKIPLANDING_VEL_Y_MAX)
    {
        ftCommonWaitSetStatus(fighter_gobj);
        return;
    }
    ftCommonLandingSetStatus(fighter_gobj);
    ftRemixZCancelFlashOnLanding(fighter_gobj);
}

/* Failed Z-cancel landing: the port flash, the punishment, then the vanilla aerial landing lag. */
static void ftRemixZCancelFailedLanding(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 landing_motion_id = nFTCommonMotionLandingAirStart - nFTCommonMotionAttackAirStart;

    if (port_enhancement_IsFailedZCancelFlashEnabled())
    {
        ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterDamageFireStart, 15);
    }
    if (ftRemixZCancelPunish(fighter_gobj))
    {
        return;
    }
    if (fp->data->mainmotion->motion_desc[fp->motion_id + landing_motion_id].anim_file_id != 0)
    {
        ftCommonLandingAirSetStatus(fighter_gobj);
    }
    else ftCommonLandingAirNullSetStatus(fighter_gobj, F_PCT_TO_DEC(fp->motion_vars.flags.flag1));
}

/* ZCancel.asm _z_cancel_success and _z_cancel_miss: a Disabled setting counts nothing, the asm returns before the
 * counters. Glide is a miss, as in the asm miss path. */
static void ftRemixZCancelCountResult(FTStruct *fp, s32 result)
{
    if (gmRemixToggle(z_cancel_opts) == FTREMIX_ZCANCEL_OPT_DISABLED)
    {
        return;
    }
    if (result == FTREMIX_ZCANCEL_RESULT_SUCCESS)
    {
        mnRemixVSStatsCountTracker(fp->player, nMNRemixVSStatsTrackerZCancelSuccess);
        return;
    }
    mnRemixVSStatsCountTracker(fp->player, nMNRemixVSStatsTrackerZCancelMiss);
}

/* ZCancel.asm _z_cancel_opts and _cruel_z_cancel: the landing of an aerial attack, called by ftCommonAttackAirProcMap when
 * the fighter lands. Auto Z-cancel (port enhancement) turns every miss into a normal landing. */
void ftRemixZCancelAttackAirLanding(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 result;

    if (fp->motion_vars.flags.flag1 == 0)
    {
        ftRemixZCancelNormalLanding(fighter_gobj);
        return;
    }
    result = ftRemixZCancelResult(fp);
    ftRemixZCancelCountResult(fp, result);
    if ((result == FTREMIX_ZCANCEL_RESULT_SUCCESS) || port_enhancement_IsAutoZCancelEnabled())
    {
        ftRemixZCancelNormalLanding(fighter_gobj);
        return;
    }
    if (result == FTREMIX_ZCANCEL_RESULT_GLIDE)
    {
        return;
    }
    ftRemixZCancelFailedLanding(fighter_gobj);
}

#endif
