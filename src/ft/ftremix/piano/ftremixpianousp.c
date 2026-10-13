/**
 * ftremixpianousp.c - C ports of Smash Remix Mad Piano up special routines
 * (.toolchain/pkgs/smashremix-src/src/Piano/PianoSpecial.asm, scope PianoUSP).
 *
 * Piano's up special is driven by temp variable 3 (motion_vars.flags.flag2,
 * set by command 5C0000XX): BEGIN, BEGIN_MOVE and MOVE. Temp variable 2
 * (flag1) requests a turn through command 580000XX.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftchar/ftmario/ftmario.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

#define PIANO_USP_AIR_DRIFT 1.0F
#define PIANO_USP_LANDING_FSM 0.15625F                  /* 0x3E200000 */
#define PIANO_USP_Y_SPEED 120.0F                        /* 0x42F00000 */
#define PIANO_USP_X_SPEED 50.0F                         /* 0x42480000 */
#define PIANO_USP_AIR_ACCEL_MOVE 0.01495361328125F      /* 0x3C750000 */
#define PIANO_USP_BEGIN_DECEL 0.875F                    /* 0x3F600000 */

enum
{
    nFTRemixPianoStatusSuperJumpChomp = 0xE1
};

enum
{
    PIANO_USP_BEGIN = 1,
    PIANO_USP_BEGIN_MOVE = 2,
    PIANO_USP_MOVE = 3
};

#define PIANO_USP_TURN_REQUEST 2

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);
extern void ftCaptainSpecialHiProcInterrupt(GObj *fighter_gobj);

/* PianoUSP.air_control_: horizontal air control. Stick clamp takes the
 * Remix air acceleration once MOVE has started. */
static void ftRemixPianoUSPAirControl(FTStruct *fp, FTAttributes *attr)
{
    f32 air_accel = attr->air_accel;

    if (fp->motion_vars.flags.flag2 == PIANO_USP_MOVE)
    {
        air_accel = PIANO_USP_AIR_ACCEL_MOVE;
    }
    ftPhysicsClampAirVelXStickRange(fp, 8, air_accel, attr->air_speed_max_x);
}

/* Physics_ BEGIN_MOVE: launch velocity from stick_x * direction. Stick
 * pushing forward lowers y speed by 0.75 * (stick_x * direction * 0.5). */
static void ftRemixPianoUSPBeginMove(FTStruct *fp)
{
    f32 stick_dir = (f32) fp->input.pl.stick_range.x * (f32) fp->lr;
    f32 vel_x = 0.0F;
    f32 vel_y = PIANO_USP_Y_SPEED;

    if (stick_dir >= 0.0F)
    {
        vel_x = stick_dir * 0.5F;
        vel_y -= 0.75F * vel_x;
    }
    vel_x += PIANO_USP_X_SPEED;

    fp->physics.vel_air.x = (f32) fp->lr * vel_x;
    fp->physics.vel_air.y = vel_y;
    fp->motion_vars.flags.flag2 = PIANO_USP_MOVE;
}

/* Physics_ BEGIN: slows horizontal speed and freezes y. */
static void ftRemixPianoUSPBegin(FTStruct *fp)
{
    fp->physics.vel_air.x *= PIANO_USP_BEGIN_DECEL;
    fp->physics.vel_air.y = 0.0F;
}

/* PianoSpecial.initial_ (special entry for the ground and air up special). */
static void ftRemixPianoUSPInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, nFTRemixPianoStatusSuperJumpChomp, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = PIANO_USP_BEGIN;

    /* Clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall (byte 0x18D & 0x07). */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;

    fp->jumps_used = (u8) fp->attr->jumps_max;
    fp->physics.vel_air.y = fp->attr->gravity;
}

/* PianoUSP.main_: when the animation ends, starts the special fall with Piano's landing lag. */
static void ftRemixPianoUSPProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, PIANO_USP_AIR_DRIFT, FALSE, TRUE, FALSE, PIANO_USP_LANDING_FSM, FALSE);
}

/* PianoUSP.change_direction_ (interrupt): turns the fighter when temp variable 2 requests it. */
static void ftRemixPianoUSPProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != PIANO_USP_TURN_REQUEST)
    {
        return;
    }
    ftCaptainSpecialHiProcInterrupt(fighter_gobj);
}

/* PianoUSP.physics_: air physics of the up special, driven by temp variable 3. */
static void ftRemixPianoUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;
    u32 state = fp->motion_vars.flags.flag2;

    if (fp->is_fastfall)
    {
        ftPhysicsApplyFastFall(fp, attr);
    }
    else ftPhysicsApplyGravityDefault(fp, attr);

    if (!ftPhysicsCheckClampAirVelXDecMax(fp, attr))
    {
        if (state == PIANO_USP_BEGIN)
        {
            ftPhysicsCheckClampAirVelXDecMax(fp, attr);
        }
        else ftRemixPianoUSPAirControl(fp, attr);

        ftPhysicsApplyAirVelXFriction(fp, attr);
    }

    if (state == PIANO_USP_BEGIN)
    {
        ftRemixPianoUSPBegin(fp);
    }
    if (fp->motion_vars.flags.flag2 == PIANO_USP_BEGIN_MOVE)
    {
        ftRemixPianoUSPBeginMove(fp);
    }
}

/* PianoUSP.collision_: Mario's up special collision with Piano's landing lag. */
static void ftRemixPianoUSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsAir)
    {
        if ((fp->motion_vars.flags.flag1 == 0) || ((fp->physics.vel_air.y >= 0.0F)))
        {
            mpCommonCheckFighterProject(fighter_gobj);
        }
        else if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) != FALSE)
        {
            if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
            {
                ftCommonCliffCatchSetStatus(fighter_gobj);
            }
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, PIANO_USP_LANDING_FSM);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* Remix address -> port routine for every PianoUSP routine the action array or
 * special tables reference. */
const FTRemixProc dFTRemixPianoUSPProcs[] =
{
    { REMIX_SYM_PianoUSP__initial_, ftRemixPianoUSPInitial },
    { REMIX_SYM_PianoUSP__main_, ftRemixPianoUSPProcUpdate },
    { REMIX_SYM_PianoUSP__change_direction_, ftRemixPianoUSPProcInterrupt },
    { REMIX_SYM_PianoUSP__physics_, ftRemixPianoUSPProcPhysics },
    { REMIX_SYM_PianoUSP__collision_, ftRemixPianoUSPProcMap }
};

const s32 dFTRemixPianoUSPProcsNum = ARRAY_COUNT(dFTRemixPianoUSPProcs);

#endif
