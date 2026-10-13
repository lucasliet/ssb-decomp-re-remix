/**
 * ftremixebidsp.c - C ports of Smash Remix Ebisumaru down special routines
 * (.toolchain/pkgs/smashremix-src/src/Ebi/EbiSpecial.asm, scope EbiDSP).
 *
 * The routines are Wario's down special copied with OS.copy_segment. Their
 * actions are Ebi.Action.DSPGROUND..DSPRECOIL (0xEF to 0xF2, Ebi.asm).
 *
 * Temp variables: flag0 = temp 1 (5400XXXX), flag1 = temp 2 (5800XXXX),
 * flag2 = temp 3 (5C00XXXX).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include "ft/ftanimend.h"
#include "ft/ftcommon/ftcommonfunctions.h"
#include "ft/ftmain.h"
#include "ft/ftphysics.h"
#include "mp/mpcommon.h"
#include "mp/mpdef.h"

enum
{
    nFTRemixEbiStatusDSPGround = 0xEF,
    nFTRemixEbiStatusDSPAir = 0xF0,
    nFTRemixEbiStatusDSPLanding = 0xF1,
    nFTRemixEbiStatusDSPRecoil = 0xF2,
    nFTRemixEbiStatusLandingHeavy = 0x20
};

enum
{
    EBI_DSP_TEMP3_BEGIN = 1,
    EBI_DSP_TEMP3_MOVE = 2
};

#define EBI_DSP_N64_RECOIL_USED 0x0AE0 /* set by begin_recoil_, blocks a second DSP in the air */
#define EBI_DSP_MOVE_DECAY 0.875F      /* 0x3F600000 */
#define EBI_DSP_INITIAL_X_SPEED 90.0F  /* 0x42B40000 */
#define EBI_DSP_INITIAL_Y_SPEED 180.0F /* 0x43340000 */
#define EBI_DSP_Y_SPEED -100.0F        /* 0xC2C80000 */
#define EBI_DSP_RECOIL_Y_SPEED 104.0F  /* 0x42D00000 */

/* Clears temp variables 1 and 2, and sets temp variable 3 to BEGIN. */
static void ftRemixEbiDSPResetSpecialVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = EBI_DSP_TEMP3_BEGIN;
}

/* EbiDSP.air_initial_: andi 0x07 on the byte at 0x18D clears these four bitfields. */
static void ftRemixEbiDSPClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Physics for the controlled air phases: drift when temp variable 2 is set, otherwise friction. */
static void ftRemixEbiDSPApplyControlledAirPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
    }
    else ftPhysicsApplyAirVelFriction(fighter_gobj);
}

static void ftRemixEbiDSPBeginLanding(GObj *fighter_gobj);
static void ftRemixEbiDSPBeginRecoil(GObj *fighter_gobj);

/* EbiDSP.ground_initial_ */
static void ftRemixEbiDSPGroundSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixEbiStatusDSPGround, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixEbiDSPResetSpecialVars(fp);
}

/* EbiDSP.air_initial_: does nothing when temp word 0x0AE0 is set or the action is already DSPRECOIL. */
static void ftRemixEbiDSPAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (*ftRemixFighterWord(fp, EBI_DSP_N64_RECOIL_USED) != 0)
    {
        return;
    }
    if (fp->status_id == nFTRemixEbiStatusDSPRecoil)
    {
        return;
    }
    ftMainSetStatus(fighter_gobj, nFTRemixEbiStatusDSPAir, 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixEbiDSPResetSpecialVars(fp);
    ftRemixEbiDSPClearAirFlags(fp);
}

/* EbiDSP.ground_move_: decays x velocity (and y on BEGIN), then launches on temp variable 1. */
static void ftRemixEbiDSPGroundProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->physics.vel_air.x *= EBI_DSP_MOVE_DECAY;
    if (fp->motion_vars.flags.flag2 == EBI_DSP_TEMP3_BEGIN)
    {
        fp->physics.vel_air.y *= EBI_DSP_MOVE_DECAY;
    }
    if (fp->motion_vars.flags.flag0 == 0)
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;
    fp->physics.vel_air.x = EBI_DSP_INITIAL_X_SPEED * fp->lr;
    fp->physics.vel_air.y = EBI_DSP_INITIAL_Y_SPEED;
    mpCommonSetFighterAir(fp);
}

/* EbiDSP.grounded_main_: transitions to the landing on animation end. */
static void ftRemixEbiDSPGroundedProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixEbiDSPBeginLanding);
}

/* EbiDSP.landing_main_: transitions to the recoil on animation end. */
static void ftRemixEbiDSPLandingProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixEbiDSPBeginRecoil);
}

/* EbiDSP.physics_: controlled air physics, then clamps negative y on BEGIN and sets the MOVE fall speed. */
static void ftRemixEbiDSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixEbiDSPApplyControlledAirPhysics(fighter_gobj);
    if (fp->motion_vars.flags.flag2 == EBI_DSP_TEMP3_BEGIN)
    {
        if (fp->physics.vel_air.y < 0.0F)
        {
            fp->physics.vel_air.y = 0.0F;
        }
    }
    else if (fp->motion_vars.flags.flag2 == EBI_DSP_TEMP3_MOVE)
    {
        fp->physics.vel_air.y = EBI_DSP_Y_SPEED;
    }
}

/* EbiDSP.begin_landing_: sets the grounded state, then the landing action (status first, as in the asm). */
static void ftRemixEbiDSPBeginLanding(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTRemixEbiStatusDSPLanding, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
}

/* EbiDSP.begin_recoil_: arms the one-DSP-per-air word when grounded, then the recoil action. */
static void ftRemixEbiDSPBeginRecoil(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        *ftRemixFighterWord(fp, EBI_DSP_N64_RECOIL_USED) = 1;
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, nFTRemixEbiStatusDSPRecoil, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    fp->physics.vel_air.y = EBI_DSP_RECOIL_Y_SPEED;
}

/* EbiDSP.collision_: landing on the ground, or landing into the recoil on a MOVE air collision. */
static void ftRemixEbiDSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    if (fp->motion_vars.flags.flag2 != EBI_DSP_TEMP3_MOVE)
    {
        mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
        return;
    }
    mpCommonProcFighterCliff(fighter_gobj, ftRemixEbiDSPBeginLanding);
}

/* EbiDSP.recoil_ground_transition_: grounded state, then LandingHeavy at frame 0 and speed 1.0. */
static void ftRemixEbiDSPRecoilGroundTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixEbiStatusLandingHeavy, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* EbiDSP.recoil_air_collision_ */
static void ftRemixEbiDSPRecoilAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixEbiDSPRecoilGroundTransition);
}

/* EbiDSP.recoil_move_: a ceiling bonk stops the recoil, otherwise the jump interrupt runs. */
static void ftRemixEbiDSPRecoilProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->coll_data.mask_curr & MAP_FLAG_CEIL)
    {
        ftCommonStopCeilSetStatus(fighter_gobj);
        return;
    }
    ftCommonJumpProcInterrupt(fighter_gobj);
}

/* EbiDSP.recoil_physics_ */
static void ftRemixEbiDSPRecoilProcPhysics(GObj *fighter_gobj)
{
    ftRemixEbiDSPApplyControlledAirPhysics(fighter_gobj);
}

const FTRemixProc dFTRemixEbiDSPProcs[] =
{
    { REMIX_SYM_EbiDSP__air_initial_, ftRemixEbiDSPAirSetStatus },
    { REMIX_SYM_EbiDSP__collision_, ftRemixEbiDSPProcMap },
    { REMIX_SYM_EbiDSP__ground_initial_, ftRemixEbiDSPGroundSetStatus },
    { REMIX_SYM_EbiDSP__ground_move_, ftRemixEbiDSPGroundProcUpdate },
    { REMIX_SYM_EbiDSP__grounded_main_, ftRemixEbiDSPGroundedProcUpdate },
    { REMIX_SYM_EbiDSP__landing_main_, ftRemixEbiDSPLandingProcUpdate },
    { REMIX_SYM_EbiDSP__physics_, ftRemixEbiDSPProcPhysics },
    { REMIX_SYM_EbiDSP__recoil_air_collision_, ftRemixEbiDSPRecoilAirProcMap },
    { REMIX_SYM_EbiDSP__recoil_move_, ftRemixEbiDSPRecoilProcUpdate },
    { REMIX_SYM_EbiDSP__recoil_physics_, ftRemixEbiDSPRecoilProcPhysics }
};

const s32 dFTRemixEbiDSPProcsNum = ARRAY_COUNT(dFTRemixEbiDSPProcs);

#endif
