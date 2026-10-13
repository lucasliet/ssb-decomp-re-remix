/**
 * ftremixpianodsp.c - C ports of Smash Remix Mad Piano down special routines
 * (.toolchain/pkgs/smashremix-src/src/Piano/PianoSpecial.asm, scope PianoDSP).
 *
 * The down special keeps its variables in the slots Ness's PSI Magnet release lag
 * uses: status_vars (0x0B18 min_frame_timer, 0x0B1C b_not_held) and the temp
 * variables 1-3 (motion_vars.flags). Piano absorbs with is_absorb (0x018D bit 0).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixPianoStatusDSPGBegin = 0xE5,
    nFTRemixPianoStatusDSPGWait = 0xE6,
    nFTRemixPianoStatusDSPGAbsorb = 0xE7,
    nFTRemixPianoStatusDSPGEnd = 0xE8,
    nFTRemixPianoStatusDSPABegin = 0xE9,
    nFTRemixPianoStatusDSPAWait = 0xEA,
    nFTRemixPianoStatusDSPAAbsorb = 0xEB,
    nFTRemixPianoStatusDSPAEnd = 0xEC,
    nFTRemixPianoStatusGroundCmdThrow = 0xED,
    nFTRemixPianoStatusAirCmdThrow = 0xEE
};

#define PIANO_DSP_WAIT_FLAGS 0x0804
#define PIANO_DSP_ABSORB_FLAGS 0x0004
#define PIANO_DSP_CMD_THROW_FLAGS 0x0024
#define PIANO_DSP_CMD_THROW_CAPTURE_IMMUNE_MASK 0x3F
#define PIANO_DSP_CMD_THROW_ACCEL 0.1F      /* 0x3DCCCCCD */
#define PIANO_DSP_CMD_THROW_MAX_SPEED 26.0F /* 0x41D00000 */

extern void ftNessSpecialLw_InitStatusVars(GObj *fighter_gobj);
extern void ftNessSpecialLw_UpdateReleaseLag(GObj *fighter_gobj);
extern void ftNessSpecialLwHitSetAbsorbTrue(GObj *fighter_gobj);
extern void ftYoshiSpecialNSetCatchParams(FTStruct *fp, void (*proc_catch)(GObj *));

/* PianoDSP's variables in status_vars. min_frame_timer and b_not_held are the slots
 * ftNessSpecialLw_UpdateReleaseLag writes as release_lag and is_release. */
typedef struct FTRemixPianoDSPStatusVars
{
    s32 min_frame_timer;  /* 0x0B18 */
    sb32 b_not_held;      /* 0x0B1C */
} FTRemixPianoDSPStatusVars;

/* One row of ground_to_air_table or air_to_ground_table. */
typedef struct FTRemixPianoDSPTransition
{
    s32 status_id;
    s32 flags;            /* ftMainSetStatus flags (table argument 4) */
    sb32 is_absorb;       /* set_bitflag: sets is_absorb (0x018D bit 0) */
    sb32 is_command_grab; /* command_grab: arms the command grab setup */
} FTRemixPianoDSPTransition;

/* Remix absorb_struct (PianoDSP.absorb_struct). Its 0x20 bytes end at size.z, so
 * damage_resist is 0 here; the Remix data does not define it. */
static FTSpecialColl sFTRemixPianoDSPAbsorbColl =
{
    1,
    0,
    { 0.0F, 330.0F, 330.0F },
    { 350.0F, 350.0F, 350.0F },
    0
};

/* ground_to_air_table, indexed by status_id - nFTRemixPianoStatusDSPGBegin. */
static const FTRemixPianoDSPTransition sFTRemixPianoDSPGroundToAirTable[] =
{
    { nFTRemixPianoStatusDSPABegin, 0x0092, FALSE, FALSE },
    { nFTRemixPianoStatusDSPAWait, 0x0097, TRUE, TRUE },
    { nFTRemixPianoStatusDSPAAbsorb, 0x0097, TRUE, FALSE },
    { nFTRemixPianoStatusDSPAEnd, 0x0092, FALSE, FALSE }
};

/* air_to_ground_table, indexed by status_id - nFTRemixPianoStatusDSPABegin. */
static const FTRemixPianoDSPTransition sFTRemixPianoDSPAirToGroundTable[] =
{
    { nFTRemixPianoStatusDSPGBegin, 0x0092, FALSE, FALSE },
    { nFTRemixPianoStatusDSPGWait, 0x0097, TRUE, TRUE },
    { nFTRemixPianoStatusDSPGAbsorb, 0x0097, TRUE, FALSE },
    { nFTRemixPianoStatusDSPGEnd, 0x0092, FALSE, FALSE }
};

static FTRemixPianoDSPStatusVars *ftRemixPianoDSPStatusVars(FTStruct *fp)
{
    return (FTRemixPianoDSPStatusVars *)&fp->status_vars;
}

/* Changes status with frame 0 and speed 1.0, then plays the new status's animation events. */
static void ftRemixPianoDSPSetStatusAnimEvents(GObj *fighter_gobj, s32 status_id, u32 flags)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* absorb_setup_: enables the absorb bitflag and attaches the absorb hitbox. */
static void ftRemixPianoDSPAbsorbSetup(FTStruct *fp)
{
    fp->is_absorb = TRUE;
    fp->special_coll = &sFTRemixPianoDSPAbsorbColl;
}

/* command_throw_setup_: sets the capture immune mask and moves the search target into the catch slot. */
static void ftRemixPianoDSPCommandThrowSetup(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftParamSetCaptureImmuneMask(fp, PIANO_DSP_CMD_THROW_CAPTURE_IMMUNE_MASK);
    fp->catch_gobj = fp->search_gobj;
}

/* cmd_throw_ground_initial_: starts the grounded command throw after a successful grab. */
static void ftRemixPianoDSPCmdThrowGroundInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPSetStatusAnimEvents(fighter_gobj, nFTRemixPianoStatusGroundCmdThrow, PIANO_DSP_CMD_THROW_FLAGS);
    ftRemixPianoDSPCommandThrowSetup(fighter_gobj);
}

/* cmd_throw_air_initial_: starts the aerial command throw after a successful grab. */
static void ftRemixPianoDSPCmdThrowAirInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPSetStatusAnimEvents(fighter_gobj, nFTRemixPianoStatusAirCmdThrow, PIANO_DSP_CMD_THROW_FLAGS);
    ftRemixPianoDSPCommandThrowSetup(fighter_gobj);
}

/* begin_initial_ (ground and aerial): starts the special and resets its temp variable 2. */
static void ftRemixPianoDSPBeginSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPianoDSPSetStatusAnimEvents(fighter_gobj, status_id, 0);
    ftNessSpecialLw_InitStatusVars(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
}

/* DSP_Ground_Begin: PianoDSP.ground_begin_initial_ */
static void ftRemixPianoDSPGroundBeginInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPBeginSetStatus(fighter_gobj, nFTRemixPianoStatusDSPGBegin);
}

/* DSP_Air_Begin: PianoDSP.air_begin_initial_ */
static void ftRemixPianoDSPAirBeginInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPBeginSetStatus(fighter_gobj, nFTRemixPianoStatusDSPABegin);
}

/* wait_initial_ (ground and aerial): clears the temp variables, arms the command grab, enables absorb. */
static void ftRemixPianoDSPWaitSetStatus(GObj *fighter_gobj, s32 status_id, FTRemixProcFn proc_cmd_throw_initial)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, PIANO_DSP_WAIT_FLAGS);
    ftYoshiSpecialNSetCatchParams(fp, proc_cmd_throw_initial);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixPianoDSPAbsorbSetup(fp);
}

/* DSP_Ground_Wait: PianoDSP.ground_wait_initial_ */
static void ftRemixPianoDSPGroundWaitInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPWaitSetStatus(fighter_gobj, nFTRemixPianoStatusDSPGWait, ftRemixPianoDSPCmdThrowGroundInitialSetStatus);
}

/* DSP_Air_Wait: PianoDSP.air_wait_initial_ */
static void ftRemixPianoDSPAirWaitInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPWaitSetStatus(fighter_gobj, nFTRemixPianoStatusDSPAWait, ftRemixPianoDSPCmdThrowAirInitialSetStatus);
}

/* ground_end_initial_: PianoDSP.ground_end_initial_ */
static void ftRemixPianoDSPGroundEndSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPSetStatusAnimEvents(fighter_gobj, nFTRemixPianoStatusDSPGEnd, 0);
}

/* air_end_initial_: PianoDSP.air_end_initial_ */
static void ftRemixPianoDSPAirEndSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPSetStatusAnimEvents(fighter_gobj, nFTRemixPianoStatusDSPAEnd, 0);
}

/* absorb_initial_ (ground and aerial): starts the absorb, arms the Ness absorb and clears temp variable 2.
 * Non-static: the Ness absorb patch calls it (see build/remixasm/notes/36_PIANO_DSP.md). */
static void ftRemixPianoDSPAbsorbSetStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPianoDSPSetStatusAnimEvents(fighter_gobj, status_id, PIANO_DSP_ABSORB_FLAGS);
    ftNessSpecialLwHitSetAbsorbTrue(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
}

/* PianoDSP.ground_absorb_initial_: called by the Ness absorb patch for Piano. */
void ftRemixPianoDSPGroundAbsorbInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPAbsorbSetStatus(fighter_gobj, nFTRemixPianoStatusDSPGAbsorb);
}

/* PianoDSP.air_absorb_initial_: called by the Ness absorb patch for Piano. */
void ftRemixPianoDSPAirAbsorbInitialSetStatus(GObj *fighter_gobj)
{
    ftRemixPianoDSPAbsorbSetStatus(fighter_gobj, nFTRemixPianoStatusDSPAAbsorb);
}

/* Ness absorb patches (0x80155278), for Mad Piano: returns TRUE when it entered Piano's absorb status. */
sb32 ftRemixPianoDSPAbsorbRedirect(FTStruct *fp, GObj *fighter_gobj)
{
    if (fp->fkind != REMIX_ID_PIANO)
    {
        return FALSE;
    }
    if (fp->ga == nMPKineticsGround)
    {
        ftRemixPianoDSPGroundAbsorbInitialSetStatus(fighter_gobj);
    }
    else ftRemixPianoDSPAirAbsorbInitialSetStatus(fighter_gobj);

    return TRUE;
}

/* PianoDSP.ground_begin_main_ */
static void ftRemixPianoDSPGroundBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixPianoDSPGroundWaitInitialSetStatus);
}

/* PianoDSP.air_begin_main_ */
static void ftRemixPianoDSPAirBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixPianoDSPAirWaitInitialSetStatus);
}

/* Updates the release lag and reports whether the minimum frames have elapsed and B is released. */
static sb32 ftRemixPianoDSPIsWaitEnded(GObj *fighter_gobj)
{
    FTRemixPianoDSPStatusVars *vars;

    ftNessSpecialLw_UpdateReleaseLag(fighter_gobj);
    vars = ftRemixPianoDSPStatusVars(ftGetStruct(fighter_gobj));

    return (vars->min_frame_timer <= 0) && vars->b_not_held;
}

/* PianoDSP.ground_wait_main_ */
static void ftRemixPianoDSPGroundWaitProcUpdate(GObj *fighter_gobj)
{
    if (ftRemixPianoDSPIsWaitEnded(fighter_gobj))
    {
        ftRemixPianoDSPGroundEndSetStatus(fighter_gobj);
    }
}

/* PianoDSP.air_wait_main_ */
static void ftRemixPianoDSPAirWaitProcUpdate(GObj *fighter_gobj)
{
    if (ftRemixPianoDSPIsWaitEnded(fighter_gobj))
    {
        ftRemixPianoDSPAirEndSetStatus(fighter_gobj);
    }
}

/* Shared body of ground_absorb_main_ and air_absorb_main_: releasing B with temp variable 2 set
 * goes to proc_release (idle or fall); otherwise the animation end goes to the wait status. */
static void ftRemixPianoDSPAbsorbProcUpdate(GObj *fighter_gobj, FTRemixProcFn proc_release, FTRemixProcFn proc_wait)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->motion_vars.flags.flag1 != 0) && !(fp->input.pl.button_hold & B_BUTTON))
    {
        proc_release(fighter_gobj);
        return;
    }
    ftAnimEndCheckSetStatus(fighter_gobj, proc_wait);
}

/* PianoDSP.ground_absorb_main_ */
static void ftRemixPianoDSPGroundAbsorbProcUpdate(GObj *fighter_gobj)
{
    ftRemixPianoDSPAbsorbProcUpdate(fighter_gobj, ftCommonWaitSetStatus, ftRemixPianoDSPGroundWaitInitialSetStatus);
}

/* PianoDSP.air_absorb_main_ */
static void ftRemixPianoDSPAirAbsorbProcUpdate(GObj *fighter_gobj)
{
    ftRemixPianoDSPAbsorbProcUpdate(fighter_gobj, ftCommonFallSetStatus, ftRemixPianoDSPAirWaitInitialSetStatus);
}

/* Applies a transition row's absorb bit and command grab setup, in that order. */
static void ftRemixPianoDSPApplyTransitionFlags(FTStruct *fp, const FTRemixPianoDSPTransition *transition, FTRemixProcFn proc_cmd_throw_initial)
{
    if (transition->is_absorb)
    {
        fp->is_absorb = TRUE;
    }
    if (transition->is_command_grab)
    {
        ftYoshiSpecialNSetCatchParams(fp, proc_cmd_throw_initial);
    }
}

/* ground_to_air_: switches a grounded down special to its aerial status. */
static void ftRemixPianoDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    const FTRemixPianoDSPTransition *transition;

    mpCommonSetFighterAir(fp);
    transition = &sFTRemixPianoDSPGroundToAirTable[fp->status_id - nFTRemixPianoStatusDSPGBegin];
    ftMainSetStatus(fighter_gobj, transition->status_id, fighter_gobj->anim_frame, 1.0F, transition->flags);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixPianoDSPApplyTransitionFlags(fp, transition, ftRemixPianoDSPCmdThrowAirInitialSetStatus);
}

/* air_to_ground_: switches an aerial down special to its grounded status. */
static void ftRemixPianoDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    const FTRemixPianoDSPTransition *transition;

    mpCommonSetFighterGround(fp);
    transition = &sFTRemixPianoDSPAirToGroundTable[fp->status_id - nFTRemixPianoStatusDSPABegin];
    ftMainSetStatus(fighter_gobj, transition->status_id, fighter_gobj->anim_frame, 1.0F, transition->flags);
    ftRemixPianoDSPApplyTransitionFlags(fp, transition, ftRemixPianoDSPCmdThrowGroundInitialSetStatus);
}

/* PianoDSP.ground_collision_ */
static void ftRemixPianoDSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixPianoDSPGroundToAir);
}

/* PianoDSP.air_collision_ */
static void ftRemixPianoDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPianoDSPAirToGround);
}

/* cmd_throw_ground_to_air_: switches the grounded command throw to the aerial one. */
static void ftRemixPianoDSPCmdThrowGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixPianoStatusAirCmdThrow, fighter_gobj->anim_frame, 1.0F, PIANO_DSP_CMD_THROW_FLAGS);
}

/* cmd_throw_air_to_ground_: switches the aerial command throw to the grounded one. */
static void ftRemixPianoDSPCmdThrowAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixPianoStatusGroundCmdThrow, fighter_gobj->anim_frame, 1.0F, PIANO_DSP_CMD_THROW_FLAGS);
}

/* PianoDSP.cmd_throw_hide_: hides the captured opponent while temp variable 1 is set. */
static void ftRemixPianoDSPCmdThrowHideProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTStruct *captured_fp;

    if ((fp->motion_vars.flags.flag0 == 0) || (fp->catch_gobj == NULL))
    {
        return;
    }
    captured_fp = ftGetStruct(fp->catch_gobj);
    captured_fp->is_invisible = TRUE;
}

/* PianoDSP.cmd_throw_ground_physics_ */
static void ftRemixPianoDSPCmdThrowGroundPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftPhysicsApplyClampGroundVelStickRange(fp, 0, PIANO_DSP_CMD_THROW_ACCEL, PIANO_DSP_CMD_THROW_MAX_SPEED);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/* PianoDSP.cmd_throw_ground_collision_ */
static void ftRemixPianoDSPCmdThrowGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixPianoDSPCmdThrowGroundToAir);
}

/* PianoDSP.cmd_throw_air_collision_ */
static void ftRemixPianoDSPCmdThrowAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPianoDSPCmdThrowAirToGround);
}

/* Remix address -> port routine for every PianoDSP routine the action array references. */
const FTRemixProc dFTRemixPianoDSPProcs[] =
{
    { REMIX_SYM_PianoDSP__air_absorb_main_, ftRemixPianoDSPAirAbsorbProcUpdate },
    { REMIX_SYM_PianoDSP__air_begin_initial_, ftRemixPianoDSPAirBeginInitialSetStatus },
    { REMIX_SYM_PianoDSP__air_begin_main_, ftRemixPianoDSPAirBeginProcUpdate },
    { REMIX_SYM_PianoDSP__air_collision_, ftRemixPianoDSPAirProcMap },
    { REMIX_SYM_PianoDSP__air_wait_main_, ftRemixPianoDSPAirWaitProcUpdate },
    { REMIX_SYM_PianoDSP__cmd_throw_air_collision_, ftRemixPianoDSPCmdThrowAirProcMap },
    { REMIX_SYM_PianoDSP__cmd_throw_ground_collision_, ftRemixPianoDSPCmdThrowGroundProcMap },
    { REMIX_SYM_PianoDSP__cmd_throw_ground_physics_, ftRemixPianoDSPCmdThrowGroundPhysics },
    { REMIX_SYM_PianoDSP__cmd_throw_hide_, ftRemixPianoDSPCmdThrowHideProcInterrupt },
    { REMIX_SYM_PianoDSP__ground_absorb_main_, ftRemixPianoDSPGroundAbsorbProcUpdate },
    { REMIX_SYM_PianoDSP__ground_begin_initial_, ftRemixPianoDSPGroundBeginInitialSetStatus },
    { REMIX_SYM_PianoDSP__ground_begin_main_, ftRemixPianoDSPGroundBeginProcUpdate },
    { REMIX_SYM_PianoDSP__ground_collision_, ftRemixPianoDSPGroundProcMap },
    { REMIX_SYM_PianoDSP__ground_wait_main_, ftRemixPianoDSPGroundWaitProcUpdate }
};

const s32 dFTRemixPianoDSPProcsNum = ARRAY_COUNT(dFTRemixPianoDSPProcs);

#endif
