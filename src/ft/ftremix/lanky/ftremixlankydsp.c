/**
 * ftremixlankydsp.c - C ports of Smash Remix Lanky Kong's down special routines
 * (.toolchain/pkgs/smashremix-src/src/Lanky/LankySpecial.asm, scope LankyDSP).
 *
 * Status ids are the DSP entries of Lanky's action table (Lanky.asm, 0xEB-0xF9).
 * Routines the worklist does not list (the *_initial_ setters and the shared
 * input checks) stay static and are only called from this file.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftcommon.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <sys/objanim.h>

enum
{
    nFTRemixLankyStatusDSPGBegin = 0xEB,
    nFTRemixLankyStatusDSPGWait = 0xEC,
    nFTRemixLankyStatusDSPGEnd = 0xED,
    nFTRemixLankyStatusDSPGCancel = 0xEE,
    nFTRemixLankyStatusDSPTurn = 0xEF,
    nFTRemixLankyStatusDSPMove = 0xF0,
    nFTRemixLankyStatusDSPTaunt = 0xF1,
    nFTRemixLankyStatusDSPLanding = 0xF2,
    nFTRemixLankyStatusDSPJumpSquat = 0xF3,
    nFTRemixLankyStatusDSPJump = 0xF4,
    nFTRemixLankyStatusDSPPlatDrop = 0xF5,
    nFTRemixLankyStatusDSPABegin = 0xF6,
    nFTRemixLankyStatusDSPAWait = 0xF7,
    nFTRemixLankyStatusDSPAEnd = 0xF8,
    nFTRemixLankyStatusDSPACancel = 0xF9
};

#define LANKY_DSP_DOWN_STICK_Y_LIMIT -39
#define LANKY_DSP_UP_STICK_Y_LIMIT 53
#define LANKY_DSP_NEUTRAL_STICK_X_LIMIT 11
#define LANKY_DSP_TAP_STICK_X_LIMIT 7
#define LANKY_DSP_TAP_STICK_Y_LIMIT 6
#define LANKY_DSP_CANCEL_STICK_Y 80
#define LANKY_DSP_SHORTHOP_FRAMES 3.0F              /* 0x40400000 */
#define LANKY_DSP_JUMP_RELEASE_MASK 0x000F
#define LANKY_DSP_CANCEL_BUTTONS (B_BUTTON | A_BUTTON)

#define LANKY_DSP_ACCELERATION 10.0F                /* 0x41200000 */
#define LANKY_DSP_NEUTRAL_ACCELERATION 4.0F         /* 0x40800000 */
#define LANKY_DSP_MIN_SPEED 8.0F                    /* 0x41000000 */
#define LANKY_DSP_MAX_SPEED 54.0F                   /* 0x42580000 */
#define LANKY_DSP_SPEED_MULTIPLIER 0.8125F          /* 0x3F500000 */
#define LANKY_DSP_STICK_SQUARE_SCALE 0.01123046875F /* 0x3C380000 */
#define LANKY_DSP_SETTLE_DIFF 2.0F                  /* 0x40000000 */
#define LANKY_DSP_FSM_SCALE 0.0234375F              /* 0x3CC00000 */
#define LANKY_DSP_FSM_OFFSET 0.10009765625F         /* 0x3DCD0000 */
#define LANKY_DSP_MOVE_START_VEL_SCALE 0.625F       /* 0x3F200000 */
#define LANKY_DSP_END_VEL_X_BOOST 16.0F             /* 0x41800000 */
#define LANKY_DSP_END_VEL_Y_SCALE 0.375F            /* 0x3EC00000 */

/* Remix scratch words in the status vars, N64 FTStruct offsets (see ftremixfields.h). */
#define LANKY_DSP_N64_MOVE_TIMER 0x0B20
#define LANKY_DSP_N64_CANCEL_VARS 0x0B20

/* Words cancel_initial_ and cancel_main_ share, N64 0x0B20-0x0B27. */
typedef struct FTRemixLankyDSPCancelVars
{
    u16 buffered_buttons; /* N64 0x0B20 */
    u8 tap_stick_x;       /* N64 0x0B22 */
    u8 tap_stick_y;       /* N64 0x0B23 */
    s32 is_jumpsquat;     /* N64 0x0B24 */
} FTRemixLankyDSPCancelVars;

_Static_assert(sizeof(FTRemixLankyDSPCancelVars) == 8, "cancel scratch must cover the 8 N64 bytes at 0x0B20");

static FTRemixLankyDSPCancelVars *ftRemixLankyDSPGetCancelVars(FTStruct *fp)
{
    return (FTRemixLankyDSPCancelVars *)ftRemixFighterWord(fp, LANKY_DSP_N64_CANCEL_VARS);
}

static f32 ftRemixLankyDSPAbsF32(f32 value)
{
    return (value < 0.0F) ? -value : value;
}

static sb32 ftRemixLankyDSPIsStickNeutral(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;

    if (stick_x < 0)
    {
        stick_x = -stick_x;
    }
    return stick_x < LANKY_DSP_NEUTRAL_STICK_X_LIMIT;
}

/* True when the stick and DIRECTION share a sign, the way the asm compares sign bits. */
static sb32 ftRemixLankyDSPIsStickWithDirection(FTStruct *fp)
{
    return (fp->input.pl.stick_range.x < 0) == (fp->lr < 0);
}

static void ftRemixLankyDSPSetStatus(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

static void ftRemixLankyDSPBeginStatus(GObj *fighter_gobj, s32 status_id)
{
    ftRemixLankyDSPSetStatus(fighter_gobj, status_id);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

static void ftRemixLankyDSPBeginStatusWithTemps(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixLankyDSPBeginStatus(fighter_gobj, status_id);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* LankyDSP.ground_initial_ (DSPGBegin entry). */
static void ftRemixLankyDSPGroundSetStatus(GObj *fighter_gobj)
{
    ftRemixLankyDSPBeginStatusWithTemps(fighter_gobj, nFTRemixLankyStatusDSPGBegin);
}

/* LankyDSP.air_initial_ (DSPABegin entry). */
static void ftRemixLankyDSPAirSetStatus(GObj *fighter_gobj)
{
    ftRemixLankyDSPBeginStatusWithTemps(fighter_gobj, nFTRemixLankyStatusDSPABegin);
}

/* LankyDSP.wait_initial_: picks DSPAWait, DSPGWait, or hands grounded movement to move/turn. */
static void ftRemixLankyDSPWaitSetStatus(GObj *fighter_gobj);

static void ftRemixLankyDSPMoveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 vel_x = (f32)fp->input.pl.stick_range.x * LANKY_DSP_MOVE_START_VEL_SCALE;

    ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPMove);
    fp->physics.vel_air.x = vel_x;
    fp->physics.vel_ground.x = vel_x * (f32)fp->lr;
}

static void ftRemixLankyDSPTurnSetStatus(GObj *fighter_gobj)
{
    ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPTurn);
}

static void ftRemixLankyDSPLandingSetStatus(GObj *fighter_gobj)
{
    ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPLanding);
}

static void ftRemixLankyDSPTauntSetStatus(GObj *fighter_gobj)
{
    ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPTaunt);
}

/* LankyDSP.wait_initial_ (grounded branch chooses move/turn/wait). */
static void ftRemixLankyDSPWaitSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsGround)
    {
        ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPAWait);
        return;
    }
    if (ftRemixLankyDSPIsStickNeutral(fp))
    {
        ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPGWait);
        return;
    }
    if (ftRemixLankyDSPIsStickWithDirection(fp))
    {
        ftRemixLankyDSPMoveSetStatus(fighter_gobj);
        return;
    }
    ftRemixLankyDSPTurnSetStatus(fighter_gobj);
}

/* LankyDSP.jumpsquat_initial_: stores the jump type and stick_y for the jump routines. */
static void ftRemixLankyDSPJumpSquatSetStatus(GObj *fighter_gobj, s32 jump_type)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixLankyDSPSetStatus(fighter_gobj, nFTRemixLankyStatusDSPJumpSquat);
    fp->status_vars.common.kneebend.anim_frame = 0.0F;
    fp->status_vars.common.kneebend.jump_force = (f32)fp->input.pl.stick_range.y;
    fp->status_vars.common.kneebend.is_shorthop = FALSE;
    fp->status_vars.common.kneebend.input_source = jump_type;
}

/* Jump velocity from the jump type, as ftDonkeyThrowFJumpSetStatus does after its status set. */
static void ftRemixLankyDSPSetJumpVelocity(FTStruct *fp)
{
    s32 vel_x;
    s32 vel_y;

    switch (fp->status_vars.common.kneebend.input_source)
    {
    case FTCOMMON_KNEEBEND_INPUT_TYPE_BUTTON:
        ftCommonJumpGetJumpForceButton(fp->input.pl.stick_range.x, &vel_x, &vel_y, fp->status_vars.common.kneebend.is_shorthop);
        break;

    case FTCOMMON_KNEEBEND_INPUT_TYPE_STICK:
    default:
        vel_x = fp->input.pl.stick_range.x;
        vel_y = fp->status_vars.common.kneebend.jump_force;
    }
    fp->physics.vel_air.y = (vel_y * fp->attr->jump_height_mul) + fp->attr->jump_height_base;
    fp->physics.vel_air.x = vel_x * fp->attr->jump_vel_x;
    fp->tap_stick_y = FTINPUT_STICKBUFFER_TICS_MAX;
}

/* LankyDSP.jump_initial_ (sets the air state, DSPJump, then the jump velocity). */
static void ftRemixLankyDSPJumpSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixLankyDSPSetStatus(fighter_gobj, nFTRemixLankyStatusDSPJump);
    ftRemixLankyDSPSetJumpVelocity(fp);
}

/* LankyDSP.plat_drop_initial_. */
static void ftRemixLankyDSPPlatDropSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixLankyDSPBeginStatus(fighter_gobj, nFTRemixLankyStatusDSPPlatDrop);
    ftPhysicsClampAirVelXMax(fp);
    fp->coll_data.ignore_line_id = fp->coll_data.floor_line_id;
    fp->tap_stick_y = FTINPUT_STICKBUFFER_TICS_MAX;
    fp->physics.vel_air.y = 0.0F;
}

/* Buffered tap value: a tap below the limit is stored as 1, any other tap is kept. */
static u8 ftRemixLankyDSPBufferTapStick(u8 tap_stick, u8 neutral_limit)
{
    if (tap_stick < neutral_limit)
    {
        return 1;
    }
    return tap_stick;
}

/* LankyDSP.cancel_initial_: a1 is the jumpsquat flag; buffers the buttons and tap sticks. */
static void ftRemixLankyDSPCancelSetStatus(GObj *fighter_gobj, sb32 is_jumpsquat)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixLankyDSPCancelVars *vars = ftRemixLankyDSPGetCancelVars(fp);
    s32 status_id = (fp->ga == nMPKineticsGround) ? nFTRemixLankyStatusDSPGCancel : nFTRemixLankyStatusDSPACancel;

    ftRemixLankyDSPBeginStatus(fighter_gobj, status_id);
    vars->buffered_buttons = fp->input.pl.button_tap;
    vars->tap_stick_x = ftRemixLankyDSPBufferTapStick(fp->tap_stick_x, LANKY_DSP_TAP_STICK_X_LIMIT);
    vars->tap_stick_y = ftRemixLankyDSPBufferTapStick(fp->tap_stick_y, LANKY_DSP_TAP_STICK_Y_LIMIT);
    if (is_jumpsquat)
    {
        vars->tap_stick_y = 1;
    }
    vars->is_jumpsquat = is_jumpsquat;
}

/* LankyDSP.end_initial_: boosts x velocity toward DIRECTION, damps y velocity, then DSPGEnd or DSPAEnd. */
static void ftRemixLankyDSPEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = (fp->ga == nMPKineticsGround) ? nFTRemixLankyStatusDSPGEnd : nFTRemixLankyStatusDSPAEnd;

    fp->physics.vel_air.x += (fp->lr < 0) ? -LANKY_DSP_END_VEL_X_BOOST : LANKY_DSP_END_VEL_X_BOOST;
    fp->physics.vel_air.y *= LANKY_DSP_END_VEL_Y_SCALE;
    ftRemixLankyDSPBeginStatus(fighter_gobj, status_id);
}

/* LankyDSP.ground_to_air_: leaves the ground into the wait action, then clamps air speed. */
static void ftRemixLankyDSPGroundToAir(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixLankyDSPWaitSetStatus(fighter_gobj);
    ftPhysicsClampAirVelXMax(ftGetStruct(fighter_gobj));
}

/* LankyDSP.air_to_ground_: lands into DSPLanding. */
static void ftRemixLankyDSPAirToGround(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixLankyDSPLandingSetStatus(fighter_gobj);
}

/* LankyDSP.ground_collision_. */
static void ftRemixLankyDSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixLankyDSPGroundToAir);
}

/* LankyDSP.air_collision_. */
static void ftRemixLankyDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixLankyDSPAirToGround);
}

/* Down special: stick held down with B. */
static sb32 ftRemixLankyDSPTryEnd(GObj *fighter_gobj, FTStruct *fp)
{
    if (fp->input.pl.stick_range.y >= LANKY_DSP_DOWN_STICK_Y_LIMIT)
    {
        return FALSE;
    }
    if ((fp->input.pl.button_tap & B_BUTTON) == 0)
    {
        return FALSE;
    }
    ftRemixLankyDSPEndSetStatus(fighter_gobj);

    return TRUE;
}

/* A or B always cancels. Shield (Z) cancels only on the ground, as in the Remix asm. */
static sb32 ftRemixLankyDSPTryCancel(GObj *fighter_gobj, FTStruct *fp, sb32 is_shield_cancel)
{
    u16 buttons = fp->input.pl.button_tap;
    sb32 is_pressed = (buttons & LANKY_DSP_CANCEL_BUTTONS) != 0;
    sb32 is_shield = is_shield_cancel && ((fp->input.button_mask_z & buttons) != 0);

    if (!is_pressed && !is_shield)
    {
        return FALSE;
    }
    ftRemixLankyDSPCancelSetStatus(fighter_gobj, FALSE);

    return TRUE;
}

static sb32 ftRemixLankyDSPTryTaunt(GObj *fighter_gobj, FTStruct *fp)
{
    if ((fp->input.button_mask_l & fp->input.pl.button_tap) == 0)
    {
        return FALSE;
    }
    ftRemixLankyDSPTauntSetStatus(fighter_gobj);

    return TRUE;
}

static sb32 ftRemixLankyDSPTryJump(GObj *fighter_gobj, FTStruct *fp)
{
    s32 jump_type = ftCommonKneeBendGetInputTypeCommon(fp);

    if (jump_type == 0)
    {
        return FALSE;
    }
    ftRemixLankyDSPJumpSquatSetStatus(fighter_gobj, jump_type);

    return TRUE;
}

static sb32 ftRemixLankyDSPTryPlatDrop(GObj *fighter_gobj, FTStruct *fp)
{
    if ((fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS) == 0)
    {
        return FALSE;
    }
    if (!ftCommonPassCheckInputSuccess(fp))
    {
        return FALSE;
    }
    ftRemixLankyDSPPlatDropSetStatus(fighter_gobj);

    return TRUE;
}

/* Interrupt chain shared by the grounded DSP wait, move, and turn routines. */
static sb32 ftRemixLankyDSPGroundInterrupt(GObj *fighter_gobj, FTStruct *fp)
{
    if (ftRemixLankyDSPTryEnd(fighter_gobj, fp))
    {
        return TRUE;
    }
    if (ftRemixLankyDSPTryCancel(fighter_gobj, fp, TRUE))
    {
        return TRUE;
    }
    if (ftRemixLankyDSPTryTaunt(fighter_gobj, fp))
    {
        return TRUE;
    }
    if (ftRemixLankyDSPTryJump(fighter_gobj, fp))
    {
        return TRUE;
    }
    return ftRemixLankyDSPTryPlatDrop(fighter_gobj, fp);
}

/* LankyDSP.begin_main_: waits for the begin animation to finish, then DSPWait. */
static void ftRemixLankyDSPBeginProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixLankyDSPWaitSetStatus(fighter_gobj);
    }
}

/* LankyDSP.ground_wait_main_. */
static void ftRemixLankyDSPGroundWaitProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixLankyDSPGroundInterrupt(fighter_gobj, fp))
    {
        return;
    }
    if (ftRemixLankyDSPIsStickNeutral(fp))
    {
        return;
    }
    if (ftRemixLankyDSPIsStickWithDirection(fp))
    {
        ftRemixLankyDSPMoveSetStatus(fighter_gobj);
        return;
    }
    ftRemixLankyDSPTurnSetStatus(fighter_gobj);
}

/* LankyDSP.air_wait_main_. */
static void ftRemixLankyDSPAirWaitProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixLankyDSPTryEnd(fighter_gobj, fp))
    {
        return;
    }
    if (ftRemixLankyDSPTryCancel(fighter_gobj, fp, FALSE))
    {
        return;
    }
    ftCommonJumpAerialCheckInterruptCommon(fighter_gobj);
}

/* LankyDSP.move_main_. */
static void ftRemixLankyDSPMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 anim_speed = (fp->physics.vel_ground.x * LANKY_DSP_FSM_SCALE) + LANKY_DSP_FSM_OFFSET;

    gcSetAnimSpeed(fighter_gobj, anim_speed);
    if (ftRemixLankyDSPGroundInterrupt(fighter_gobj, fp))
    {
        return;
    }
    if (ftRemixLankyDSPIsStickNeutral(fp))
    {
        if (fp->physics.vel_air.x == 0.0F)
        {
            ftRemixLankyDSPWaitSetStatus(fighter_gobj);
        }
        return;
    }
    if (!ftRemixLankyDSPIsStickWithDirection(fp))
    {
        ftRemixLankyDSPTurnSetStatus(fighter_gobj);
    }
}

/* LankyDSP.turn_main_. */
static void ftRemixLankyDSPTurnProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        fp->lr = -fp->lr;
        fp->motion_vars.flags.flag1 = 0;
    }
    if (ftRemixLankyDSPGroundInterrupt(fighter_gobj, fp))
    {
        return;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixLankyDSPWaitSetStatus(fighter_gobj);
    }
}

/* Button jumps pressed in the first frames of the squat become short hops. */
static void ftRemixLankyDSPJumpSquatCheckShorthop(FTStruct *fp)
{
    if (fp->status_vars.common.kneebend.anim_frame > LANKY_DSP_SHORTHOP_FRAMES)
    {
        return;
    }
    if ((fp->input.pl.button_release & LANKY_DSP_JUMP_RELEASE_MASK) == 0)
    {
        return;
    }
    fp->status_vars.common.kneebend.is_shorthop = TRUE;
}

/* LankyDSP.jumpsquat_main_: counts the squat, short-hops on a button press, then jumps. */
static void ftRemixLankyDSPJumpSquatProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->status_vars.common.kneebend.anim_frame += 1.0F;
    if (fp->status_vars.common.kneebend.input_source == FTCOMMON_KNEEBEND_INPUT_TYPE_BUTTON)
    {
        ftRemixLankyDSPJumpSquatCheckShorthop(fp);
    }
    if (fp->attr->kneebend_anim_length <= fp->status_vars.common.kneebend.anim_frame)
    {
        ftRemixLankyDSPJumpSetStatus(fighter_gobj);
    }
}

/* LankyDSP.jumpsquat_interrupt_: holding up with A or B cancels the squat. */
static void ftRemixLankyDSPJumpSquatProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.stick_range.y < LANKY_DSP_UP_STICK_Y_LIMIT)
    {
        return;
    }
    if ((fp->input.pl.button_tap & LANKY_DSP_CANCEL_BUTTONS) == 0)
    {
        return;
    }
    ftRemixLankyDSPCancelSetStatus(fighter_gobj, TRUE);
}

/* LankyDSP.jump_main_ (DSPJump and DSPPlatDrop). */
static void ftRemixLankyDSPJumpProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixLankyDSPTryEnd(fighter_gobj, fp))
    {
        return;
    }
    if (ftRemixLankyDSPTryCancel(fighter_gobj, fp, FALSE))
    {
        return;
    }
    if (ftCommonJumpAerialCheckInterruptCommon(fighter_gobj) != FALSE)
    {
        return;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixLankyDSPWaitSetStatus(fighter_gobj);
    }
}

/* LankyDSP.landing_main_ (DSPLanding and DSPTaunt). */
static void ftRemixLankyDSPLandingProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixLankyDSPWaitSetStatus(fighter_gobj);
    }
}

/* LankyDSP.cancel_main_: replays the buffered buttons, then returns to the wait action. */
static void ftRemixLankyDSPCancelProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixLankyDSPCancelVars *vars = ftRemixLankyDSPGetCancelVars(fp);

    vars->buffered_buttons |= fp->input.pl.button_tap;
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    if (vars->is_jumpsquat != FALSE)
    {
        fp->input.pl.stick_range.y = LANKY_DSP_CANCEL_STICK_Y;
    }
    fp->tap_stick_x = vars->tap_stick_x;
    fp->tap_stick_y = vars->tap_stick_y;
    /* LankySpecial.asm cancel_main_ (_change_action): the delay slot of the transition stores the merged buttons to the
     * player struct at 0x1BE (input.pl.button_tap), the same value as the buffered buttons after this frame's OR. */
    fp->input.pl.button_tap = vars->buffered_buttons;
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* Target x velocity for a stick outside the neutral zone (ground_physics_). */
static f32 ftRemixLankyDSPGetStickTargetVelX(FTStruct *fp)
{
    f32 stick_x = (f32)fp->input.pl.stick_range.x;
    f32 target = (stick_x * ftRemixLankyDSPAbsF32(stick_x)) * LANKY_DSP_STICK_SQUARE_SCALE;

    target += (stick_x < 0.0F) ? -LANKY_DSP_MIN_SPEED : LANKY_DSP_MIN_SPEED;

    return target * LANKY_DSP_SPEED_MULTIPLIER;
}

/* Moves vel_x toward target_vel_x by acceleration, then clamps and stores the ground velocity. */
static void ftRemixLankyDSPApproachVelX(FTStruct *fp, f32 target_vel_x, f32 acceleration)
{
    f32 vel_x = fp->physics.vel_air.x;
    f32 diff = target_vel_x - vel_x;

    if (ftRemixLankyDSPAbsF32(diff) >= LANKY_DSP_SETTLE_DIFF)
    {
        vel_x = (diff >= 0.0F) ? (vel_x + acceleration) : (vel_x - acceleration);
    }
    if (ftRemixLankyDSPAbsF32(vel_x) > LANKY_DSP_MAX_SPEED)
    {
        vel_x = (vel_x < 0.0F) ? -LANKY_DSP_MAX_SPEED : LANKY_DSP_MAX_SPEED;
    }
    fp->physics.vel_air.x = vel_x;
    fp->physics.vel_ground.x = vel_x * (f32)fp->lr;
}

/* LankyDSP.ground_physics_. */
static void ftRemixLankyDSPGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 acceleration = LANKY_DSP_ACCELERATION;
    f32 target_vel_x;

    if (ftRemixLankyDSPIsStickNeutral(fp))
    {
        acceleration = LANKY_DSP_NEUTRAL_ACCELERATION;
        if (ftRemixLankyDSPAbsF32(fp->physics.vel_air.x) <= LANKY_DSP_MIN_SPEED)
        {
            *ftRemixFighterWord(fp, LANKY_DSP_N64_MOVE_TIMER) = -1;
            fp->physics.vel_air.x = 0.0F;
            return;
        }
        target_vel_x = 0.0F;
    }
    else
    {
        target_vel_x = ftRemixLankyDSPGetStickTargetVelX(fp);
    }
    ftRemixLankyDSPApproachVelX(fp, target_vel_x, acceleration);
}

const FTRemixProc dFTRemixLankyDSPProcs[] =
{
    { REMIX_SYM_LankyDSP__air_collision_, ftRemixLankyDSPAirProcMap },
    { REMIX_SYM_LankyDSP__air_initial_, ftRemixLankyDSPAirSetStatus },
    { REMIX_SYM_LankyDSP__air_wait_main_, ftRemixLankyDSPAirWaitProcUpdate },
    { REMIX_SYM_LankyDSP__begin_main_, ftRemixLankyDSPBeginProcUpdate },
    { REMIX_SYM_LankyDSP__cancel_main_, ftRemixLankyDSPCancelProcUpdate },
    { REMIX_SYM_LankyDSP__ground_collision_, ftRemixLankyDSPGroundProcMap },
    { REMIX_SYM_LankyDSP__ground_initial_, ftRemixLankyDSPGroundSetStatus },
    { REMIX_SYM_LankyDSP__ground_physics_, ftRemixLankyDSPGroundProcPhysics },
    { REMIX_SYM_LankyDSP__ground_wait_main_, ftRemixLankyDSPGroundWaitProcUpdate },
    { REMIX_SYM_LankyDSP__jump_main_, ftRemixLankyDSPJumpProcUpdate },
    { REMIX_SYM_LankyDSP__jumpsquat_interrupt_, ftRemixLankyDSPJumpSquatProcInterrupt },
    { REMIX_SYM_LankyDSP__jumpsquat_main_, ftRemixLankyDSPJumpSquatProcUpdate },
    { REMIX_SYM_LankyDSP__landing_main_, ftRemixLankyDSPLandingProcUpdate },
    { REMIX_SYM_LankyDSP__move_main_, ftRemixLankyDSPMoveProcUpdate },
    { REMIX_SYM_LankyDSP__turn_main_, ftRemixLankyDSPTurnProcUpdate }
};

const s32 dFTRemixLankyDSPProcsNum = ARRAY_COUNT(dFTRemixLankyDSPProcs);

#endif
