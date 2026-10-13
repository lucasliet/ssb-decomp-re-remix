/**
 * ftremixconkerusp.c - C ports of Smash Remix Conker up special routines
 * (.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm, scope ConkerUSP).
 *
 * The Remix source copies the first instructions of Fox's up special
 * (ROM 0xD6328, 0xD6340, 0xD7190, 0xD7194, 0xD6788) with OS.copy_segment. Those
 * copies are decoded here by hand into the calls they make.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixConkerStatusHelicopteryTailThing = 0xE3,
    nFTRemixConkerStatusHelicopteryTailThingAir = 0xE4,
    nFTRemixConkerStatusHelicopteryTailThingDescent = 0xE6
};

#define CONKER_USP_LANDING_LAG 0.35F                   /* LANDING_FSM 0x3EB33333 */
#define CONKER_USP_INITIAL_LIFT_AIR 30.0F              /* 0x41F00000 */
#define CONKER_USP_INITIAL_LIFT_GROUND 0.0F
#define CONKER_USP_INITIAL_VEL_X_CLAMP 38.0F           /* 0x42180000 */
#define CONKER_USP_INITIAL_JUMPS_STATUS_VAR 9          /* stored at status_vars word 0 (0xB18) */
#define CONKER_USP_LIFT_MULT_NORMAL 1.0F
#define CONKER_USP_LIFT_MULT_LINK 0.07000160217285156F /* 0x3D8F5D00, the value Link and DK use */
#define CONKER_USP_AIR_VEL_X_MULT 0.04475586116313934F /* 0x3D3751EC */
#define CONKER_USP_AIR_VEL_X_CAP 50.0F                 /* 0x42480000 */
#define CONKER_USP_DESCENT_LIFT_MULT 0.25F             /* 0x3E800000 */
#define CONKER_USP_DESCENT_VEL_X_MULT 0.04F            /* 0x3D23D70A */
#define CONKER_USP_DESCENT_VEL_X_CAP 32.0F             /* 0x42000000 */
#define CONKER_USP_LIFT_OFF_VEL_Y 80.0F                /* 0x42A00000 */
#define CONKER_USP_Y_VEL_DECAY 0.9375F                 /* 0x3F700000 (the Remix comment says 0.94) */

/* Shared body of ConkerUSP.air_initial_ and ground_initial_ (ConkerUSP_part2_). */
static void ftRemixConkerUSPInitialSetStatus(GObj *fighter_gobj, s32 status_id, f32 lift)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->physics.vel_air.y = lift;

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftPhysicsClampAirVelX(fp, CONKER_USP_INITIAL_VEL_X_CLAMP);

    fp->motion_vars.flags.flag1 = 0;
    fp->status_vars.common.dead.wait = CONKER_USP_INITIAL_JUMPS_STATUS_VAR;
    fp->jumps_used = attr->jumps_max;
}

/* ConkerUSP.air_initial_ */
static void ftRemixConkerUSPAirInitial(GObj *fighter_gobj)
{
    ftRemixConkerUSPInitialSetStatus(fighter_gobj, nFTRemixConkerStatusHelicopteryTailThingAir, CONKER_USP_INITIAL_LIFT_AIR);
}

/* ConkerUSP.ground_initial_ */
static void ftRemixConkerUSPGroundInitial(GObj *fighter_gobj)
{
    ftRemixConkerUSPInitialSetStatus(fighter_gobj, nFTRemixConkerStatusHelicopteryTailThing, CONKER_USP_INITIAL_LIFT_GROUND);
}

/* Begins the special fall (ftCommonFallSpecialSetStatus) used by main_air and descent_main_air. */
static void ftRemixConkerUSPSetFallSpecial(GObj *fighter_gobj)
{
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, CONKER_USP_LANDING_LAG, FALSE);
}

/* ConkerUSP.main_air: holding B enters the descent, releasing it starts the fall. */
static void ftRemixConkerUSPMainAirProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    if (fp->input.pl.button_hold & B_BUTTON)
    {
        ftMainSetStatus(fighter_gobj, nFTRemixConkerStatusHelicopteryTailThingDescent, 0.0F, 1.0F, 0);
        ftMainPlayAnimEventsAll(fighter_gobj);
        return;
    }
    ftRemixConkerUSPSetFallSpecial(fighter_gobj);
}

/* ConkerUSP.ground_y_velocity_: temp variable 2 (0x180) selects lift-off, decay or nothing. */
static void ftRemixConkerUSPGroundYVelocityProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 velocity_mode = fp->motion_vars.flags.flag1;

    if (velocity_mode == 0)
    {
        return;
    }
    if (velocity_mode == 1)
    {
        mpCommonSetFighterAir(fp);
        fp->physics.vel_air.y = CONKER_USP_LIFT_OFF_VEL_Y;
        return;
    }
    fp->physics.vel_air.y *= CONKER_USP_Y_VEL_DECAY;
}

/* Applies gravity clamped to tvel_base, scaled by lift_multiplier (ftPhysicsApplyGravityClampTVel). */
static void ftRemixConkerUSPApplyLift(FTStruct *fp, f32 lift_multiplier)
{
    ftPhysicsApplyGravityClampTVel(fp, fp->attr->gravity * lift_multiplier, fp->attr->tvel_base);
}

/* ConkerUSP.air_physics_: the lift multiplier is smaller while temp variable 1 (0x17C) is 0. */
static void ftRemixConkerUSPAirPhysicsProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 lift_multiplier = CONKER_USP_LIFT_MULT_NORMAL;

    if (fp->motion_vars.flags.flag0 == 0)
    {
        lift_multiplier = CONKER_USP_LIFT_MULT_LINK;
    }
    ftRemixConkerUSPApplyLift(fp, lift_multiplier);
    ftPhysicsClampAirVelXStickRange(fp, 0, CONKER_USP_AIR_VEL_X_MULT, CONKER_USP_AIR_VEL_X_CAP);
}

/* ConkerUSP.descent_main_air: releasing B during the descent starts the fall. */
static void ftRemixConkerUSPDescentMainAirProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    ftRemixConkerUSPSetFallSpecial(fighter_gobj);
}

/* ConkerUSP.descent_air_physics_2: slow descent while B is held. */
static void ftRemixConkerUSPDescentAirPhysicsProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixConkerUSPApplyLift(fp, CONKER_USP_DESCENT_LIFT_MULT);
    ftPhysicsClampAirVelXStickRange(fp, 0, CONKER_USP_DESCENT_VEL_X_MULT, CONKER_USP_DESCENT_VEL_X_CAP);
}

/* Remix address -> port routine for every Conker up special routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixConkerUSPProcs[] =
{
    { REMIX_SYM_ConkerUSP__air_initial_, ftRemixConkerUSPAirInitial },
    { REMIX_SYM_ConkerUSP__air_physics_, ftRemixConkerUSPAirPhysicsProcPhysics },
    { REMIX_SYM_ConkerUSP__descent_air_physics_2, ftRemixConkerUSPDescentAirPhysicsProcPhysics },
    { REMIX_SYM_ConkerUSP__descent_main_air, ftRemixConkerUSPDescentMainAirProcUpdate },
    { REMIX_SYM_ConkerUSP__ground_initial_, ftRemixConkerUSPGroundInitial },
    { REMIX_SYM_ConkerUSP__ground_y_velocity_, ftRemixConkerUSPGroundYVelocityProcInterrupt },
    { REMIX_SYM_ConkerUSP__main_air, ftRemixConkerUSPMainAirProcUpdate }
};

const s32 dFTRemixConkerUSPProcsNum = ARRAY_COUNT(dFTRemixConkerUSPProcs);

#endif
