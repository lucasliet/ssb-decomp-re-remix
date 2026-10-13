/**
 * ftremixgoemondsp.c - C ports of Smash Remix Goemon down special routines
 * (.toolchain/pkgs/smashremix-src/src/Goemon/GoemonSpecial.asm, scope GoemonDSP).
 *
 * Grounded and aerial DSP use actions 0xEF to 0xF9: Goemon.asm adds them in the
 * order DSPGround..DSPEnd, and the action string table is indexed by id - 0xDC.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftchar/ftyoshi/ftyoshifunctions.h>
#include <ft/ftchar/ftdonkey/ftdonkeyfunctions.h>
#include <ft/ftchar/ftmario/ftmario.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <mp/mpprocess.h>

enum
{
    nFTRemixGoemonStatusDSPGround = 0xEF,
    nFTRemixGoemonStatusDSPGroundPull = 0xF0,
    nFTRemixGoemonStatusDSPGroundWallPull = 0xF1,
    nFTRemixGoemonStatusDSPGBreak = 0xF3,
    nFTRemixGoemonStatusDSPAir = 0xF4,
    nFTRemixGoemonStatusDSPAirPull = 0xF5,
    nFTRemixGoemonStatusDSPAirWallPull = 0xF6,
    nFTRemixGoemonStatusDSPAAttack = 0xF7,
    nFTRemixGoemonStatusDSPABreak = 0xF8,
    nFTRemixGoemonStatusDSPEnd = 0xF9
};

enum
{
    GOEMON_DSP_CHAIN_NONE = 0,
    GOEMON_DSP_CHAIN_LEFT_WALL = 1,
    GOEMON_DSP_CHAIN_RIGHT_WALL = 2,
    GOEMON_DSP_CHAIN_WALL_BREAK = 3
};

#define GOEMON_DSP_COLLISION_FGM 0x13
#define GOEMON_DSP_ATTACK_X_SPEED 64.0F
#define GOEMON_DSP_END_X_SPEED 24.0F
#define GOEMON_DSP_LAUNCH_Y_SPEED 36.0F
#define GOEMON_DSP_MAX_WALL_DISTANCE 400.0F
/* 0x3EE05C29: the end_collision_ copy patches Mario's lui 0x3E8F to 0x3EE0 and keeps its ori 0x5C29. */
#define GOEMON_DSP_LANDING_LAG 0.4382031261920929F
/* FTStruct 0x94C is joints[25] (joints starts at 0x8E8, 4 bytes per DObj *). */
#define GOEMON_DSP_CHAIN_JOINT_INDEX 25
#define GOEMON_DSP_CHAIN_ECB_TOP 10.0F
#define GOEMON_DSP_CHAIN_ECB_CENTER 5.0F
#define GOEMON_DSP_CHAIN_ECB_BOTTOM 0.0F
#define GOEMON_DSP_CHAIN_ECB_WIDTH 40.0F

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);
extern void *func_800269C0_275C0(u16);

static void ftRemixGoemonDSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Starts a status at the frame stored in temp variable 2 (flag1). */
static void ftRemixGoemonDSPEnterTempFrameStatus(GObj *fighter_gobj, s32 status_id, f32 anim_speed)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, (f32)fp->motion_vars.flags.flag1, anim_speed, FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* GoemonDSP.grab_pull_setup_ */
static void ftRemixGoemonDSPGrabPullSetup(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftParamSetCaptureImmuneMask(fp, 0x3F);
    fp->catch_gobj = fp->search_gobj;
}

static void ftRemixGoemonDSPSetAerialKinetics(FTStruct *fp)
{
    fp->ga = nMPKineticsAir;

    if (fp->jumps_used == 0)
    {
        fp->jumps_used = 1;
    }
}

/* Starts an aerial status that launches the fighter with x_speed in its facing direction. */
static void ftRemixGoemonDSPEnterLaunch(GObj *fighter_gobj, s32 status_id, f32 x_speed)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPSetAerialKinetics(fp);
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.x = x_speed * fp->lr;
    fp->physics.vel_air.y = GOEMON_DSP_LAUNCH_Y_SPEED;
}

static void ftRemixGoemonDSPEnterInitial(GObj *fighter_gobj, s32 status_id, void (*proc_catch)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftYoshiSpecialNSetCatchParams(fp, proc_catch);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixGoemonDSPClearTempVars(fp);
}

/* Changes to the status `offset` ids away from the current one (ground <-> air). */
static void ftRemixGoemonDSPChangeStatusByOffset(GObj *fighter_gobj, s32 offset)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, fp->status_id + offset, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
}

static void ftRemixGoemonDSPGroundPullInitial(GObj *fighter_gobj);
static void ftRemixGoemonDSPAirPullInitial(GObj *fighter_gobj);

/* GoemonDSP.ground_initial_ */
static void ftRemixGoemonDSPGroundInitial(GObj *fighter_gobj)
{
    ftRemixGoemonDSPEnterInitial(fighter_gobj, nFTRemixGoemonStatusDSPGround, ftRemixGoemonDSPGroundPullInitial);
}

/* GoemonDSP.air_initial_ */
static void ftRemixGoemonDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterInitial(fighter_gobj, nFTRemixGoemonStatusDSPAir, ftRemixGoemonDSPAirPullInitial);
    /* Remix: `and v1, v1, 0x07` then `sb v1, 0x18D`, but v1 is never loaded from 0x18D, so the mask is
     * applied to a stale register. Only the documented intent (disable fast fall) is kept. */
    fp->is_fastfall = FALSE;
}

/* GoemonDSP.ground_pull_initial_ */
static void ftRemixGoemonDSPGroundPullInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterTempFrameStatus(fighter_gobj, nFTRemixGoemonStatusDSPGroundPull, 1.0F);
    ftRemixGoemonDSPGrabPullSetup(fighter_gobj);
    ftRemixGoemonDSPClearTempVars(fp);
    func_800269C0_275C0(GOEMON_DSP_COLLISION_FGM);
}

/* GoemonDSP.air_pull_initial_ */
static void ftRemixGoemonDSPAirPullInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterTempFrameStatus(fighter_gobj, nFTRemixGoemonStatusDSPAirPull, 1.0F);
    ftRemixGoemonDSPGrabPullSetup(fighter_gobj);
    ftRemixGoemonDSPClearTempVars(fp);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    func_800269C0_275C0(GOEMON_DSP_COLLISION_FGM);
}

/* GoemonDSP.ground_wall_pull_initial_ */
static void ftRemixGoemonDSPGroundWallPullInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterTempFrameStatus(fighter_gobj, nFTRemixGoemonStatusDSPGroundWallPull, 1.0F);
    ftRemixGoemonDSPClearTempVars(fp);
    func_800269C0_275C0(GOEMON_DSP_COLLISION_FGM);
}

/* GoemonDSP.air_wall_pull_initial_ */
static void ftRemixGoemonDSPAirWallPullInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterTempFrameStatus(fighter_gobj, nFTRemixGoemonStatusDSPAirWallPull, 1.0F);
    ftRemixGoemonDSPClearTempVars(fp);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    func_800269C0_275C0(GOEMON_DSP_COLLISION_FGM);
}

/* GoemonDSP.ground_break_initial_ */
static void ftRemixGoemonDSPGroundBreakInitial(GObj *fighter_gobj)
{
    ftRemixGoemonDSPEnterTempFrameStatus(fighter_gobj, nFTRemixGoemonStatusDSPGBreak, 2.0F);
}

/* GoemonDSP.air_break_initial_ */
static void ftRemixGoemonDSPAirBreakInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterTempFrameStatus(fighter_gobj, nFTRemixGoemonStatusDSPABreak, 2.0F);
    fp->jumps_used = (u8)fp->attr->jumps_max;
}

/* GoemonDSP.attack_initial_ (called by pull_main_) */
static void ftRemixGoemonDSPAttackInitial(GObj *fighter_gobj)
{
    ftRemixGoemonDSPEnterLaunch(fighter_gobj, nFTRemixGoemonStatusDSPAAttack, GOEMON_DSP_ATTACK_X_SPEED);
}

/* GoemonDSP.end_initial_ (called by wall_pull_main_) */
static void ftRemixGoemonDSPEndInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonDSPEnterLaunch(fighter_gobj, nFTRemixGoemonStatusDSPEnd, GOEMON_DSP_END_X_SPEED);
    fp->motion_vars.flags.flag1 = 1;
}

/* GoemonDSP.ground_to_air_ */
static void ftRemixGoemonDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixGoemonDSPChangeStatusByOffset(fighter_gobj, 5);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixGoemonDSPAirPullInitial);
}

/* GoemonDSP.air_to_ground_ */
static void ftRemixGoemonDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixGoemonDSPChangeStatusByOffset(fighter_gobj, -5);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixGoemonDSPGroundPullInitial);
}

/* GoemonDSP.shared_ground_to_air_ */
static void ftRemixGoemonDSPSharedGroundToAir(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixGoemonDSPChangeStatusByOffset(fighter_gobj, 5);
}

/* GoemonDSP.shared_air_to_ground_ */
static void ftRemixGoemonDSPSharedAirToGround(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixGoemonDSPChangeStatusByOffset(fighter_gobj, -5);
}

/* GoemonDSP.check_wall_distance_: sets pos.x back to x_initial and reports whether the chain moved past the limit. */
static sb32 ftRemixGoemonDSPCheckWallDistance(Vec3f *pos, f32 x_initial)
{
    f32 distance = x_initial - pos->x;

    if (distance < 0.0F)
    {
        distance = -distance;
    }
    pos->x = x_initial;

    return distance > GOEMON_DSP_MAX_WALL_DISTANCE;
}

/* GoemonDSP.chain_detect_collision_: returns GOEMON_DSP_CHAIN_* for the chain's ECB. */
static s32 ftRemixGoemonDSPChainDetectCollision(MPCollData *coll_data)
{
    f32 x_initial = coll_data->p_translate->x;
    sb32 is_wall_break = FALSE;
    s32 result = GOEMON_DSP_CHAIN_NONE;

    if (mpProcessCheckTestLWallCollisionAdjNew(coll_data))
    {
        mpProcessRunLWallCollisionAdjNew(coll_data);
        is_wall_break = ftRemixGoemonDSPCheckWallDistance(coll_data->p_translate, x_initial);
        if (!is_wall_break)
        {
            return GOEMON_DSP_CHAIN_LEFT_WALL;
        }
        result = GOEMON_DSP_CHAIN_LEFT_WALL;
    }
    if (mpProcessCheckTestRWallCollisionAdjNew(coll_data))
    {
        mpProcessRunRWallCollisionAdjNew(coll_data);
        is_wall_break = ftRemixGoemonDSPCheckWallDistance(coll_data->p_translate, x_initial);
        result = GOEMON_DSP_CHAIN_RIGHT_WALL;
    }
    if (is_wall_break)
    {
        return GOEMON_DSP_CHAIN_WALL_BREAK;
    }
    return result;
}

/* GoemonDSP.chain_collision_: builds the chain ECB from the fighter's collision data and detects walls. */
static s32 ftRemixGoemonDSPChainCollision(FTStruct *fp)
{
    Vec3f chain_pos = { 0.0F, 0.0F, 0.0F };
    MPCollData chain_coll = fp->coll_data;

    gmCollisionGetFighterPartsWorldPosition(fp->joints[GOEMON_DSP_CHAIN_JOINT_INDEX], &chain_pos);
    chain_coll.p_translate = &chain_pos;
    chain_coll.map_coll.top = GOEMON_DSP_CHAIN_ECB_TOP;
    chain_coll.map_coll.center = GOEMON_DSP_CHAIN_ECB_CENTER;
    chain_coll.map_coll.bottom = GOEMON_DSP_CHAIN_ECB_BOTTOM;
    chain_coll.map_coll.width = GOEMON_DSP_CHAIN_ECB_WIDTH;
    chain_coll.p_map_coll = &chain_coll.map_coll;

    return ftRemixGoemonDSPChainDetectCollision(&chain_coll);
}

/* Wall bit tested for a chain result: the Remix asm masks with 1 when DIRECTION >= 0 and with 2 otherwise. */
static sb32 ftRemixGoemonDSPChainHitsWall(FTStruct *fp, s32 chain_result)
{
    s32 wall_mask = (fp->lr >= 0) ? GOEMON_DSP_CHAIN_LEFT_WALL : GOEMON_DSP_CHAIN_RIGHT_WALL;

    return (chain_result & wall_mask) != 0;
}

/* GoemonDSP.ground_collision_ */
static void ftRemixGoemonDSPGroundProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 chain_result;

    if (fp->motion_vars.flags.flag2 != 0)
    {
        chain_result = ftRemixGoemonDSPChainCollision(fp);
        if (chain_result == GOEMON_DSP_CHAIN_WALL_BREAK)
        {
            ftRemixGoemonDSPGroundBreakInitial(fighter_gobj);
            return;
        }
        if (ftRemixGoemonDSPChainHitsWall(fp, chain_result))
        {
            ftRemixGoemonDSPGroundWallPullInitial(fighter_gobj);
            return;
        }
    }
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixGoemonDSPGroundToAir);
}

/* GoemonDSP.air_collision_ */
static void ftRemixGoemonDSPAirProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 chain_result;

    if (fp->motion_vars.flags.flag2 != 0)
    {
        chain_result = ftRemixGoemonDSPChainCollision(fp);
        if (chain_result == GOEMON_DSP_CHAIN_WALL_BREAK)
        {
            ftRemixGoemonDSPAirBreakInitial(fighter_gobj);
            return;
        }
        if (ftRemixGoemonDSPChainHitsWall(fp, chain_result))
        {
            ftRemixGoemonDSPAirWallPullInitial(fighter_gobj);
            return;
        }
    }
    mpCommonProcFighterLanding(fighter_gobj, ftRemixGoemonDSPAirToGround);
}

/* GoemonDSP.shared_ground_collision_ */
static void ftRemixGoemonDSPSharedGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixGoemonDSPSharedGroundToAir);
}

/* GoemonDSP.shared_air_collision_ */
static void ftRemixGoemonDSPSharedAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixGoemonDSPSharedAirToGround);
}

/* GoemonDSP.end_collision_: Mario's up special collision with Goemon's landing lag. */
static void ftRemixGoemonDSPEndProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsAir)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    if ((fp->motion_vars.flags.flag1 == 0) || (fp->physics.vel_air.y >= 0.0F))
    {
        mpCommonCheckFighterProject(fighter_gobj);
        return;
    }
    if (!mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass))
    {
        return;
    }
    if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
    {
        ftCommonCliffCatchSetStatus(fighter_gobj);
    }
    else
    {
        ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, GOEMON_DSP_LANDING_LAG);
    }
}

static sb32 ftRemixGoemonDSPIsDonkeyThrowF(FTStruct *fp)
{
    if ((fp->fkind != nFTKindDonkey) && (fp->fkind != nFTKindNDonkey) && (fp->fkind != nFTKindGDonkey))
    {
        return FALSE;
    }
    return fp->status_id == nFTCommonStatusThrowF;
}

/* GoemonDSP.main_ (grounded and aerial DSP) */
static void ftRemixGoemonDSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        fp->motion_vars.flags.flag1 -= 1;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* GoemonDSP.pull_main_: copy of ftCommonThrowProcUpdate (0x8014A0C0) where the final wait-or-fall becomes attack_initial_. */
static void ftRemixGoemonDSPPullProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        fp->motion_vars.flags.flag1 = 0;
        fp->lr = -fp->lr;
        fp->physics.vel_ground.x = -fp->physics.vel_ground.x;
    }
    if (fp->motion_vars.flags.flag2 != 0)
    {
        ftCommonThrownProcPhysics(fp->catch_gobj);
        ftCommonThrownReleaseThrownUpdateStats(fp->catch_gobj, (fp->motion_vars.flags.flag2 == 1) ? -fp->lr : fp->lr, (fp->status_id == nFTCommonStatusThrowB) ? 1 : 0, TRUE);
        fp->motion_vars.flags.flag2 = 0;
        fp->catch_gobj = NULL;
        ftParamSetCaptureImmuneMask(fp, FTCATCHKIND_MASK_NONE);
    }
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    if (ftRemixGoemonDSPIsDonkeyThrowF(fp))
    {
        ftCommonCaptureShoulderedSetStatus(fp->catch_gobj);
        ftDonkeyThrowFWaitSetStatus(fighter_gobj);
        return;
    }
    ftRemixGoemonDSPAttackInitial(fighter_gobj);
}

/* GoemonDSP.wall_pull_main_ */
static void ftRemixGoemonDSPWallPullProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixGoemonDSPEndInitial(fighter_gobj);
    }
}

/* Remix address -> port routine for every GoemonDSP routine in the worklist. */
const FTRemixProc dFTRemixGoemonDSPProcs[] =
{
    { REMIX_SYM_GoemonDSP__ground_initial_, ftRemixGoemonDSPGroundInitial },
    { REMIX_SYM_GoemonDSP__air_initial_, ftRemixGoemonDSPAirInitial },
    { REMIX_SYM_GoemonDSP__ground_collision_, ftRemixGoemonDSPGroundProcMap },
    { REMIX_SYM_GoemonDSP__air_collision_, ftRemixGoemonDSPAirProcMap },
    { REMIX_SYM_GoemonDSP__shared_ground_collision_, ftRemixGoemonDSPSharedGroundProcMap },
    { REMIX_SYM_GoemonDSP__shared_air_collision_, ftRemixGoemonDSPSharedAirProcMap },
    { REMIX_SYM_GoemonDSP__end_collision_, ftRemixGoemonDSPEndProcMap },
    { REMIX_SYM_GoemonDSP__main_, ftRemixGoemonDSPProcUpdate },
    { REMIX_SYM_GoemonDSP__pull_main_, ftRemixGoemonDSPPullProcUpdate },
    { REMIX_SYM_GoemonDSP__wall_pull_main_, ftRemixGoemonDSPWallPullProcUpdate }
};

const s32 dFTRemixGoemonDSPProcsNum = ARRAY_COUNT(dFTRemixGoemonDSPProcs);

#endif
