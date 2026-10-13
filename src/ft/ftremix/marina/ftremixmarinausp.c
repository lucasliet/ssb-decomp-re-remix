/**
 * ftremixmarinausp.c - C ports of Smash Remix Marina up special routines
 * (.toolchain/pkgs/smashremix-src/src/Marina/MarinaSpecial.asm, scope MarinaUSP).
 *
 * The routines splice vanilla code with OS.copy_segment: Fox's end of up special
 * (0x8015C750, first 8 lines), Captain Falcon's turn (0x80160370 is called
 * directly), Mario's up special collision (0x80156358, 30 lines) and the vanilla
 * air physics (0x800D90F0, 16 lines). Temp variables 1/2/3 are
 * motion_vars.flags.flag0/1/2 (set by moveset command 5C0000XX / 580000XX).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>
#include <ft/ftchar/ftmario/ftmariofunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

#define MARINA_USP_X_SPEED 10.0F                  /* 0x41200000 */
#define MARINA_USP_AIR_Y_SPEED 92.0F              /* 0x42B80000 */
#define MARINA_USP_GROUND_Y_SPEED 98.0F           /* 0x42C40000 */
#define MARINA_USP_AIR_ACCELERATION 0.0166015625F /* 0x3C880000 */
#define MARINA_USP_AIR_SPEED 22.0F                /* 0x41B00000 */
#define MARINA_USP_AIR_CONTROL_STICK_MIN 8
#define MARINA_USP_BEGIN_X_SCALE 0.875F           /* 0x3F600000 */
#define MARINA_USP_LANDING_LAG_MAIN 0.375F        /* 0x3EC00000: lui 0x3EC0 only */
#define MARINA_USP_LANDING_LAG_COLLISION 0.3757031261920929F /* 0x3EC05C29: lui 0x3EC0 + Mario's ori 0x5C29 */

enum
{
    nFTRemixMarinaStatusUSPG = 0xFC,
    nFTRemixMarinaStatusUSPA = 0xFD
};

/* Temp variable 3 (motion_vars.flags.flag2) states. */
enum
{
    MARINA_USP_BEGIN = 1,
    MARINA_USP_BEGIN_MOVE = 2,
    MARINA_USP_MOVE = 3
};

/* Temp variable 2 (motion_vars.flags.flag1) value that turns Marina. */
#define MARINA_USP_CHANGE_DIRECTION 2

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);

/* ftMainSetStatus(gobj, status, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE) followed by ftMainPlayAnimEventsAll. */
static void ftRemixMarinaUSPSetStatus(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Temp variable 1 and 2 = 0, temp variable 3 = BEGIN. */
static void ftRemixMarinaUSPInitTempVariables(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = MARINA_USP_BEGIN;
}

/* lbu 0x18D; andi 0x07: clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall. */
static void ftRemixMarinaUSPClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* MarinaUSP.air_initial_: aerial up special entry, freezes y at gravity. */
static void ftRemixMarinaUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaUSPSetStatus(fighter_gobj, nFTRemixMarinaStatusUSPA);
    ftRemixMarinaUSPInitTempVariables(fp);
    ftRemixMarinaUSPClearAirFlags(fp);
    fp->physics.vel_air.y = fp->attr->gravity;
}

/* MarinaUSP.ground_initial_: grounded up special entry. */
static void ftRemixMarinaUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaUSPSetStatus(fighter_gobj, nFTRemixMarinaStatusUSPG);
    ftRemixMarinaUSPInitTempVariables(fp);
}

/* MarinaUSP.main_: Fox's up special end (0x8015C750) with the landing lag 0.375F and the interrupt flag off. */
static void ftRemixMarinaUSPMain(GObj *fighter_gobj)
{
    /* Negated so that a NaN frame returns, as the bc1fl in the copied code does. */
    if (!(fighter_gobj->anim_frame <= 0.0F))
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, MARINA_USP_LANDING_LAG_MAIN, FALSE);
}

/* MarinaUSP.change_direction_: Captain Falcon's turn subroutine while temp variable 2 is CHANGE_DIRECTION. */
static void ftRemixMarinaUSPChangeDirection(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != MARINA_USP_CHANGE_DIRECTION)
    {
        return;
    }
    ftCaptainSpecialHiProcInterrupt(fighter_gobj);
}

/* Copied vanilla air physics (0x800D90F0): gravity or fast fall, then the x decay check.
 * Returns the check result; non-zero skips air control and air friction. */
static sb32 ftRemixMarinaUSPApplyVanillaAirPhysics(FTStruct *fp)
{
    FTAttributes *attr = fp->attr;

    if (fp->is_fastfall)
    {
        ftPhysicsApplyFastFall(fp, attr);
    }
    else ftPhysicsApplyGravityDefault(fp, attr);

    return ftPhysicsCheckClampAirVelXDecMax(fp, attr);
}

/* Air control for MOVE (ftPhysicsClampAirVelXStickRange with Marina's accel and speed), otherwise the x decay check. */
static void ftRemixMarinaUSPApplyAirControl(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag2 != MARINA_USP_MOVE)
    {
        ftPhysicsCheckClampAirVelXDecMax(fp, fp->attr);
        return;
    }
    ftPhysicsClampAirVelXStickRange(fp, MARINA_USP_AIR_CONTROL_STICK_MIN, MARINA_USP_AIR_ACCELERATION,
                                    MARINA_USP_AIR_SPEED);
}

/* Temp variable 3 BEGIN: slow x and freeze y, unless the action is USPG. */
static void ftRemixMarinaUSPSlowBegin(FTStruct *fp)
{
    if ((fp->motion_vars.flags.flag2 != MARINA_USP_BEGIN) || (fp->status_id == nFTRemixMarinaStatusUSPG))
    {
        return;
    }
    fp->physics.vel_air.x *= MARINA_USP_BEGIN_X_SCALE;
    fp->physics.vel_air.y = 0.0F;
}

/* Temp variable 3 BEGIN_MOVE: set x and y velocity from the direction and the action, then enter MOVE. */
static void ftRemixMarinaUSPStartMove(FTStruct *fp)
{
    f32 y_speed = (fp->status_id == nFTRemixMarinaStatusUSPG) ? MARINA_USP_GROUND_Y_SPEED : MARINA_USP_AIR_Y_SPEED;

    fp->motion_vars.flags.flag2 = MARINA_USP_MOVE;
    fp->jumps_used = fp->attr->jumps_max;
    fp->physics.vel_air.x = MARINA_USP_X_SPEED * (f32)fp->lr;
    fp->physics.vel_air.y = y_speed;
}

/* Aerial physics after the copied vanilla step: the begin states run whether or not that step returned early. */
static void ftRemixMarinaUSPCheckBegin(FTStruct *fp)
{
    ftRemixMarinaUSPSlowBegin(fp);

    if (fp->motion_vars.flags.flag2 != MARINA_USP_BEGIN_MOVE)
    {
        return;
    }
    ftRemixMarinaUSPStartMove(fp);
}

/* Aerial branch of MarinaUSP.physics_. */
static void ftRemixMarinaUSPAirPhysics(FTStruct *fp)
{
    if (!ftRemixMarinaUSPApplyVanillaAirPhysics(fp))
    {
        ftRemixMarinaUSPApplyAirControl(fp);
        ftPhysicsApplyAirVelXFriction(fp, fp->attr);
    }
    ftRemixMarinaUSPCheckBegin(fp);
}

/* MarinaUSP.physics_: grounded friction, or the aerial branch. */
static void ftRemixMarinaUSPPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsAir)
    {
        ftPhysicsApplyGroundVelFriction(fighter_gobj);
        return;
    }
    ftRemixMarinaUSPAirPhysics(fp);
}

/* MarinaUSP.collision_: ftMarioSpecialHiProcMap (0x80156358) with Marina's landing lag. */
static void ftRemixMarinaUSPCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsAir)
    {
        if ((fp->motion_vars.flags.flag1 == 0) || (fp->physics.vel_air.y >= 0.0F))
        {
            mpCommonCheckFighterProject(fighter_gobj);
        }
        else if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) != FALSE)
        {
            if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
            {
                ftCommonCliffCatchSetStatus(fighter_gobj);
            }
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, MARINA_USP_LANDING_LAG_COLLISION);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* Remix address -> port routine for every MarinaUSP routine in the Marina action array. */
const FTRemixProc dFTRemixMarinaUSPProcs[] =
{
    { REMIX_SYM_MarinaUSP__air_initial_, ftRemixMarinaUSPAirInitial },
    { REMIX_SYM_MarinaUSP__change_direction_, ftRemixMarinaUSPChangeDirection },
    { REMIX_SYM_MarinaUSP__collision_, ftRemixMarinaUSPCollision },
    { REMIX_SYM_MarinaUSP__ground_initial_, ftRemixMarinaUSPGroundInitial },
    { REMIX_SYM_MarinaUSP__main_, ftRemixMarinaUSPMain },
    { REMIX_SYM_MarinaUSP__physics_, ftRemixMarinaUSPPhysics }
};

const s32 dFTRemixMarinaUSPProcsNum = ARRAY_COUNT(dFTRemixMarinaUSPProcs);

#endif
