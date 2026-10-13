/**
 * ftremixbanjodsp.c - C ports of Smash Remix Banjo down special routines
 * (.toolchain/pkgs/smashremix-src/src/Banjo/BanjoSpecial.asm, scope BanjoDSP).
 *
 * Beak Bomb keeps its phase in temp variable 3 (motion_vars.flags.flag2): BEGIN
 * while the dive starts, MOVE while it is driven, 0 in the aerial loop. The
 * grounded variant reuses Falcon Kick's map helpers from ftcaptainspeciallw.c.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftchar/ftcaptain/ftcaptain.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixBanjoStatusDSPA = 0xE7,
    nFTRemixBanjoStatusDSPLand = 0xE8,
    nFTRemixBanjoStatusDSPG = 0xE9,
    nFTRemixBanjoStatusDSPGAir = 0xEA,
    nFTRemixBanjoStatusDSPALoop = 0xEB
};

enum
{
    nFTRemixBanjoDSPTemp3Begin = 1,
    nFTRemixBanjoDSPTemp3Move = 2
};

#define BANJO_DSP_TEMP2_ACTION_CHANGE 1         /* status_check_ acts only while temp variable 2 = 0x1 */
#define BANJO_DSP_AERIAL_INITIAL_Y_SPEED 12.0F  /* 0x41400000: rise at the start of the aerial dive */
#define BANJO_DSP_Y_SPEED -110.0F               /* 0xC2DC0000: dive speed */
#define BANJO_DSP_BEGIN_Y_SCALE 0.75F           /* 0x3F400000 */
#define BANJO_DSP_AIR_X_SCALE 0.875F            /* 0x3F600000 */

/* BanjoDSP.loop_initial_: starts the aerial loop, with all temp variables cleared. */
static void ftRemixBanjoDSPLoopInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusDSPALoop, 0.0F, 1.0F, FTSTATUS_PRESERVE_HIT);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* BanjoDSP.aerial_main_ */
static void ftRemixBanjoDSPAerialProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixBanjoDSPLoopInitial);
}

/* BanjoDSP.ground_initial_ */
static void ftRemixBanjoDSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusDSPG, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = nFTRemixBanjoDSPTemp3Begin;
}

/* BanjoDSP.air_initial_ */
static void ftRemixBanjoDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusDSPA, 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = nFTRemixBanjoDSPTemp3Begin;

    /* lbu 0x018D; andi 0x07: clears is_absorb..is_fastfall, keeps the rest of the byte. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* BanjoDSP.air_move_: scales the fall while the dive begins, otherwise scales the horizontal drift. */
static void ftRemixBanjoDSPAirMoveProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 == nFTRemixBanjoDSPTemp3Begin)
    {
        fp->physics.vel_air.y *= BANJO_DSP_BEGIN_Y_SCALE;
        return;
    }
    fp->physics.vel_air.x *= BANJO_DSP_AIR_X_SCALE;
}

/* BanjoDSP.physics_: friction while temp variable 2 is set, drift otherwise, then the dive or rise speed. */
static void ftRemixBanjoDSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        ftPhysicsApplyAirVelFriction(fighter_gobj);
    }
    else ftPhysicsApplyAirVelDrift(fighter_gobj);

    if ((fp->status_id == nFTRemixBanjoStatusDSPALoop) || (fp->motion_vars.flags.flag2 == nFTRemixBanjoDSPTemp3Move))
    {
        fp->physics.vel_air.y = BANJO_DSP_Y_SPEED;
    }
    else fp->physics.vel_air.y = BANJO_DSP_AERIAL_INITIAL_Y_SPEED;
}

/* BanjoDSP.begin_landing_: Falcon Kick's landing status (0x801600EC) with the Banjo landing action. */
static void ftRemixBanjoDSPBeginLanding(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusDSPLand, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* Aerial half of BanjoDSP.collision_: the dive lands into the landing status, anything else uses the common landing. */
static void ftRemixBanjoDSPAerialProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->motion_vars.flags.flag2 == nFTRemixBanjoDSPTemp3Move) || (fp->status_id == nFTRemixBanjoStatusDSPALoop))
    {
        mpCommonProcFighterCliff(fighter_gobj, ftRemixBanjoDSPBeginLanding);
        return;
    }
    mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
}

/* BanjoDSP.collision_ */
static void ftRemixBanjoDSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    ftRemixBanjoDSPAerialProcMap(fighter_gobj);
}

/* BanjoDSP.action_change_: a grounded dive that left the ground enters the aerial fall (DSPGAir),
 * keeping the root joint's height and enabling the hitlag effect pause/resume. */
static void ftRemixBanjoDSPActionChange(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 root_y = fp->joints[nFTPartsJointTopN]->translate.vec.f.y;

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusDSPGAir, 0.0F, 1.0F, FTSTATUS_PRESERVE_EFFECT);

    fp->joints[nFTPartsJointTopN]->translate.vec.f.y = root_y;

    fp->proc_lagstart = ftParamProcPauseEffect;
    fp->proc_lagend = ftParamProcResumeEffect;
}

/* BanjoDSP.status_check_: runs the action change once temp variable 2 is armed and the fighter is airborne. */
static void ftRemixBanjoDSPStatusCheck(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != BANJO_DSP_TEMP2_ACTION_CHANGE)
    {
        return;
    }
    if (fp->ga != nMPKineticsAir)
    {
        return;
    }
    ftRemixBanjoDSPActionChange(fighter_gobj);
}

/* BanjoDSP.grounded_collision_: Falcon Kick's map checks, then the status check unless a bound was taken. */
static void ftRemixBanjoDSPGroundedProcMap(GObj *fighter_gobj)
{
    ftCaptainSpecialLwDecideMapCollide(fighter_gobj);
    if (ftCaptainSpecialLwBoundCheckGoto(fighter_gobj) != FALSE)
    {
        return;
    }
    ftRemixBanjoDSPStatusCheck(fighter_gobj);
}

/* Remix address -> port routine for every BanjoDSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixBanjoDSPProcs[] =
{
    { REMIX_SYM_BanjoDSP__aerial_main_, ftRemixBanjoDSPAerialProcUpdate },
    { REMIX_SYM_BanjoDSP__air_initial_, ftRemixBanjoDSPAirInitial },
    { REMIX_SYM_BanjoDSP__air_move_, ftRemixBanjoDSPAirMoveProcInterrupt },
    { REMIX_SYM_BanjoDSP__collision_, ftRemixBanjoDSPProcMap },
    { REMIX_SYM_BanjoDSP__ground_initial_, ftRemixBanjoDSPGroundInitial },
    { REMIX_SYM_BanjoDSP__grounded_collision_, ftRemixBanjoDSPGroundedProcMap },
    { REMIX_SYM_BanjoDSP__physics_, ftRemixBanjoDSPProcPhysics }
};

const s32 dFTRemixBanjoDSPProcsNum = ARRAY_COUNT(dFTRemixBanjoDSPProcs);

#endif
