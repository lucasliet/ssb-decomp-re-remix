/**
 * ftremixcrashusp.c - C ports of Smash Remix Crash up special routines
 * (.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm, scope CrashUSP).
 *
 * The physics routine copies the vanilla air drift subroutine 0x800D90E0 and
 * replaces its gravity and air control calls with Crash's own. Temp variable 1
 * (flags.flag0) starts the jump, temp variable 2 (flags.flag1) selects the
 * faster air control and gravity, and temp variable 3 (flags.flag2) turns.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include "ft/ftmain.h"
#include "ft/ftphysics.h"
#include "mp/mpcommon.h"
#include "mp/mpdef.h"

#define CRASH_USP_Y_SPEED 56.0F
#define CRASH_USP_FALL_SPEED 120.0F
#define CRASH_USP_GRAVITY 1.5F
#define CRASH_USP_GRAVITY_2 18.0F
#define CRASH_USP_AIR_SPEED 48.0F
#define CRASH_USP_AIR_ACCEL 0.080078125F        /* 0x3DA40000 */
#define CRASH_USP_AIR_SPEED_2 20.0F
#define CRASH_USP_AIR_ACCEL_2 0.050048828125F   /* 0x3D4D0000 */
#define CRASH_USP_AIR_FRICTION 2.0F
#define CRASH_USP_AIR_INITIAL_Y_SCALE 0.048828125F /* 0x3D480000 */

enum
{
    nFTRemixCrashStatusUSPG = 0xEB,
    nFTRemixCrashStatusUSPA = 0xEC,
    nFTRemixCrashStatusUSPLanding = 0xED
};

extern void ftCaptainSpecialHiProcInterrupt(GObj *fighter_gobj);

/* Shared prologue of CrashUSP.ground_initial_ and air_initial_: change action at
 * frame 0, play anim events, then clear temp variables 1-3. */
static void ftRemixCrashUSPBeginStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* CrashUSP.ground_initial_ */
static void ftRemixCrashUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashUSPBeginStatus(fighter_gobj, nFTRemixCrashStatusUSPG);
    fp->physics.vel_air.y = 0.0F;
}

/* CrashUSP.air_initial_ */
static void ftRemixCrashUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashUSPBeginStatus(fighter_gobj, nFTRemixCrashStatusUSPA);
    /* lbu 0x18D; andi 0x07: clears is_absorb..is_fastfall. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
    fp->physics.vel_air.y *= CRASH_USP_AIR_INITIAL_Y_SCALE;
}

/* CrashUSP.landing_initial_ (helper, passed to mpCommonProcFighterCliff). */
static void ftRemixCrashUSPLandingInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixCrashStatusUSPLanding, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_ground.x = 0.0F;
}

/* CrashUSP.collision_ */
static void ftRemixCrashUSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    mpCommonProcFighterCliff(fighter_gobj, ftRemixCrashUSPLandingInitial);
}

/* CrashUSP.main_ */
static void ftRemixCrashUSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 == 0)
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;
    fp->physics.vel_air.y = CRASH_USP_Y_SPEED;
    /* Take mid-air jumps away at this point. */
    mpCommonSetFighterAir(fp);
    fp->jumps_used = fp->attr->jumps_max;
    /* REMIX-TODO(PokemonAnnouncer): PokemonAnnouncer.body_slam_announcement_ is skipped. */
}

/* CrashUSP.change_direction_ */
static void ftRemixCrashUSPChangeDirection(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 == 0)
    {
        return;
    }
    /* Turn subroutine copied from Captain Falcon (0x80160370). */
    fp->motion_vars.flags.flag1 = 1;
    ftCaptainSpecialHiProcInterrupt(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* CrashUSP.air_control_ (helper): vanilla 0x800D9044 with faster values while
 * temp variable 2 is set. The stick minimum 8 is the original a1 argument. */
static void ftRemixCrashUSPAirControl(FTStruct *fp)
{
    f32 accel = CRASH_USP_AIR_ACCEL;
    f32 speed = CRASH_USP_AIR_SPEED;

    if (fp->motion_vars.flags.flag1 != 0)
    {
        accel = CRASH_USP_AIR_ACCEL_2;
        speed = CRASH_USP_AIR_SPEED_2;
    }
    ftPhysicsClampAirVelXStickRange(fp, 8, accel, speed);
}

/* CrashUSP.air_friction_ (helper): vanilla 0x800D9074 with a stack attribute
 * whose air_friction is CRASH_USP_AIR_FRICTION. The vanilla code reads no other field. */
static void ftRemixCrashUSPAirFriction(FTStruct *fp)
{
    FTAttributes friction_attr = { 0 };

    friction_attr.air_friction = CRASH_USP_AIR_FRICTION;
    ftPhysicsApplyAirVelXFriction(fp, &friction_attr);
}

/* CrashUSP.physics_: vanilla 0x800D90E0 with Crash's gravity and air control.
 * When temp variable 2 is set and the vanilla decel check passes, the air control
 * and friction steps are skipped (the copied bnez targets the epilogue). */
static void ftRemixCrashUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 gravity = (fp->motion_vars.flags.flag1 != 0) ? CRASH_USP_GRAVITY_2 : CRASH_USP_GRAVITY;

    ftPhysicsApplyGravityClampTVel(fp, gravity, CRASH_USP_FALL_SPEED);
    if ((fp->motion_vars.flags.flag1 != 0) && ftPhysicsCheckClampAirVelXDecMax(fp, fp->attr))
    {
        return;
    }
    ftRemixCrashUSPAirControl(fp);
    ftRemixCrashUSPAirFriction(fp);
}

const FTRemixProc dFTRemixCrashUSPProcs[] =
{
    { REMIX_SYM_CrashUSP__air_initial_, ftRemixCrashUSPAirInitial },
    { REMIX_SYM_CrashUSP__change_direction_, ftRemixCrashUSPChangeDirection },
    { REMIX_SYM_CrashUSP__collision_, ftRemixCrashUSPProcMap },
    { REMIX_SYM_CrashUSP__ground_initial_, ftRemixCrashUSPGroundInitial },
    { REMIX_SYM_CrashUSP__main_, ftRemixCrashUSPProcUpdate },
    { REMIX_SYM_CrashUSP__physics_, ftRemixCrashUSPProcPhysics }
};

const s32 dFTRemixCrashUSPProcsNum = ARRAY_COUNT(dFTRemixCrashUSPProcs);

#endif
