/**
 * ftremixsonicdsp.c - C ports of Smash Remix Sonic Spin Dash routines
 * (.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm, scope SonicDSP).
 *
 * Spin Dash keeps its charge level in status_vars word 0 (0x0B18). Temp
 * variable 1 (flag0, 0x17C) is the release request, temp variable 2 (flag1,
 * 0x180) is charge added this frame, and temp variable 3 (flag2, 0x184) is
 * the latch that hands control back to the player after a move.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftanimend.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <mp/map.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <sys/obj.h>
#include <sys/utils.h>

/* Sonic.Action.DSP_* ids: entries 0xF6-0xFC of the assembled SONIC action
 * array (Character.add_new_action in SonicSpecial/Sonic.asm). */
enum
{
    nFTRemixSonicStatusDSPGroundCharge = 0xF6,
    nFTRemixSonicStatusDSPGroundMove = 0xF7,
    nFTRemixSonicStatusDSPGroundEnd = 0xF8,
    nFTRemixSonicStatusDSPAirCharge = 0xF9,
    nFTRemixSonicStatusDSPAirMove = 0xFA,
    nFTRemixSonicStatusDSPAirJump = 0xFB,
    nFTRemixSonicStatusDSPAirEnd = 0xFC
};

#define SONIC_DSP_MAX_CHARGE_GROUND 24
#define SONIC_DSP_MAX_CHARGE_AIR 18
#define SONIC_DSP_CHARGE_PER_PRESS 3
/* Charge below this on landing starts a fresh charge; the asm compares unsigned (sltiu). */
#define SONIC_DSP_CHARGE_INSTANT_MOVE 6
/* A stick_y below this counts as held down and blocks release. */
#define SONIC_DSP_STICK_DOWN_MIN -39
#define SONIC_DSP_SFX_CHARGE 0x3D8
#define SONIC_DSP_SFX_CHARGE_CLASSIC 0x3DE
#define SONIC_DSP_WALL_COLLISION_L 0x0001
#define SONIC_DSP_WALL_COLLISION_R 0x0020
#define SONIC_DSP_FRAME_SPEED_ONE 0x3F800000u
/* Remix sets TopN's speed to 1.0 plus one ULP so ftMainSetStatus resets it on the next action change. */
#define SONIC_DSP_FRAME_SPEED_ONE_ULP 0x3F800001u

#define SONIC_DSP_FRAME_SPEED_OFFSET 20.0F              /* 0x41A00000 */
#define SONIC_DSP_FRAME_SPEED_SCALE 0.0098876953125F    /* 0x3C220000 (the asm comment says 0.01) */
#define SONIC_DSP_BASE_SPEED 46.0F                      /* 0x42380000 */
#define SONIC_DSP_BASE_SPEED_SS 70.0F                   /* 0x428C0000 */
#define SONIC_DSP_MIN_SPEED 15.0F                       /* 0x41700000 */
#define SONIC_DSP_MIN_SPEED_SS 25.0F                    /* 0x41C80000 */
#define SONIC_DSP_JUMP_SPEED 62.0F                      /* 0x42780000 */
#define SONIC_DSP_JUMP_SPEED_SS 75.0F                   /* 0x42960000 */
#define SONIC_DSP_SLOPE_ACCELERATION 3.5F               /* 0x40600000 */
#define SONIC_DSP_GRAVITY 2.25F                         /* 0x40100000 */
#define SONIC_DSP_MAX_FALL_SPEED 50.0F                  /* 0x42480000 */
#define SONIC_DSP_AIR_FRICTION 3.0F                     /* 0x40400000 */
#define SONIC_DSP_GROUND_TRACTION 0.25F                 /* 0x3E800000 */
#define SONIC_DSP_GROUND_CHARGE_X_SCALE 0.5F            /* 0x3F000000 */
#define SONIC_DSP_AIR_CHARGE_Y_SCALE 0.3125F            /* 0x3EA00000 (the asm comment says 0.31) */

/* REMIX-TODO(classic): Sonic.classic_table (Remix RAM 0x80563BC0, per-port
 * is_classic) is written by Sonic.asm's save/load/reset_classic_* routines,
 * which are not ported. Until they are, it stays 0 and Spin Dash always uses
 * SPINDASH_CHARGE (0x3D8) instead of CLASSIC_SPINDASH_CHARGE (0x3DE). */
static u8 sFTRemixSonicDSPClassicTable[MAXCONTROLLERS];

typedef struct FTRemixSonicDSPStatusVars
{
    s32 charge_level;
} FTRemixSonicDSPStatusVars;

static FTRemixSonicDSPStatusVars *ftRemixSonicDSPStatusVars(FTStruct *fp)
{
    return (FTRemixSonicDSPStatusVars *)&fp->status_vars;
}

static void ftRemixSonicDSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* lbu 0x18D; andi 0x07 keeps only the low three bits (is_magnify_show,
 * is_limit_map_bounds, is_invisible), so the first four bitfields clear. */
static void ftRemixSonicDSPClearAbsorbAndFastFall(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Starts a DSP status at frame 0 (the asm's *initial_ routines). */
static void ftRemixSonicDSPSetStatusFromStart(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* Starts a DSP status at the current animation frame (the asm's transition routines). */
static void ftRemixSonicDSPSetStatusKeepFrame(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
}

static f32 ftRemixSonicDSPBaseSpeed(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_SSONIC) ? SONIC_DSP_BASE_SPEED_SS : SONIC_DSP_BASE_SPEED;
}

static f32 ftRemixSonicDSPMinSpeed(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_SSONIC) ? SONIC_DSP_MIN_SPEED_SS : SONIC_DSP_MIN_SPEED;
}

static f32 ftRemixSonicDSPJumpSpeed(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_SSONIC) ? SONIC_DSP_JUMP_SPEED_SS : SONIC_DSP_JUMP_SPEED;
}

/* (speed + 20) * 0.01 as in the asm; the animation rate follows the body's speed. */
static f32 ftRemixSonicDSPFrameSpeedFromSpeed(f32 speed)
{
    return (speed + SONIC_DSP_FRAME_SPEED_OFFSET) * SONIC_DSP_FRAME_SPEED_SCALE;
}

/* Sets the frame speed of TopN and of the two common joints (joints[4] and joints[5] in the asm). */
static void ftRemixSonicDSPSetFrameSpeed(GObj *fighter_gobj, f32 frame_speed)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    DObj *top_joint = DObjGetStruct(fighter_gobj);

    if (*(u32 *)&top_joint->anim_speed == SONIC_DSP_FRAME_SPEED_ONE)
    {
        *(u32 *)&top_joint->anim_speed = SONIC_DSP_FRAME_SPEED_ONE_ULP;
    }
    fp->joints[nFTPartsJointCommonStart]->anim_speed = frame_speed;
    fp->joints[nFTPartsJointCommonStart + 1]->anim_speed = frame_speed;
}

/* SonicDSP.ground_charge_initial_: special entry for the grounded Spin Dash. */
static void ftRemixSonicDSPGroundChargeSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPGroundCharge);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSonicDSPClearTempVars(fp);
    fp->physics.vel_ground.x *= SONIC_DSP_GROUND_CHARGE_X_SCALE;
    ftRemixSonicDSPStatusVars(fp)->charge_level = 0;
}

/* SonicDSP.air_charge_initial_: special entry for the aerial Spin Dash. */
static void ftRemixSonicDSPAirChargeSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPAirCharge);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSonicDSPClearTempVars(fp);
    fp->physics.vel_air.y *= SONIC_DSP_AIR_CHARGE_Y_SCALE;
    ftRemixSonicDSPClearAbsorbAndFastFall(fp);
    ftRemixSonicDSPStatusVars(fp)->charge_level = 0;
}

/* SonicDSP.ground_move_initial_: starts the grounded release at the charged speed. */
static void ftRemixSonicDSPGroundMoveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 ground_speed;

    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPGroundMove);
    ground_speed = ftRemixSonicDSPBaseSpeed(fp) + (f32)(ftRemixSonicDSPStatusVars(fp)->charge_level * 4);
    fp->physics.vel_ground.x = ground_speed;
    fp->physics.vel_air.x = ground_speed * fp->lr;
}

/* SonicDSP.air_move_initial_: starts the aerial release. */
static void ftRemixSonicDSPAirMoveSetStatus(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPAirMove);
}

/* SonicDSP.air_jump_initial_: starts the jump out of the grounded release. */
static void ftRemixSonicDSPAirJumpSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPAirJump);
    fp->physics.vel_air.y = ftRemixSonicDSPJumpSpeed(fp);
    efManagerDustHeavyDoubleMakeEffect(fp->coll_data.p_translate, 1, 1.0F);
}

/* SonicDSP.ground_end_initial_ */
static void ftRemixSonicDSPGroundEndSetStatus(GObj *fighter_gobj)
{
    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPGroundEnd);
}

/* SonicDSP.air_end_initial_ */
static void ftRemixSonicDSPAirEndSetStatus(GObj *fighter_gobj)
{
    ftRemixSonicDSPSetStatusFromStart(fighter_gobj, nFTRemixSonicStatusDSPAirEnd);
}

/* SonicDSP.ground_charge_transition_: landing while charging. A charge of 6 or more moves at once. */
static void ftRemixSonicDSPGroundChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    if ((u32)ftRemixSonicDSPStatusVars(fp)->charge_level < SONIC_DSP_CHARGE_INSTANT_MOVE)
    {
        ftRemixSonicDSPSetStatusKeepFrame(fighter_gobj, nFTRemixSonicStatusDSPGroundCharge);
        return;
    }
    ftRemixSonicDSPGroundMoveSetStatus(fighter_gobj);
}

/* SonicDSP.air_charge_transition_ */
static void ftRemixSonicDSPAirChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixSonicDSPSetStatusKeepFrame(fighter_gobj, nFTRemixSonicStatusDSPAirCharge);
    ftPhysicsClampAirVelXMax(fp);
}

/* SonicDSP.ground_move_transition_ */
static void ftRemixSonicDSPGroundMoveTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixSonicDSPSetStatusKeepFrame(fighter_gobj, nFTRemixSonicStatusDSPGroundMove);
}

/* SonicDSP.ground_end_transition_ */
static void ftRemixSonicDSPGroundEndTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixSonicDSPSetStatusKeepFrame(fighter_gobj, nFTRemixSonicStatusDSPGroundEnd);
}

/* SonicDSP.air_end_transition_ */
static void ftRemixSonicDSPAirEndTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixSonicDSPSetStatusKeepFrame(fighter_gobj, nFTRemixSonicStatusDSPAirEnd);
    ftPhysicsClampAirVelXMax(fp);
}

/* Charge-state update shared by the grounded and aerial charges: button
 * presses add charge, temp variable 2 adds on top, and the level is capped. */
static void ftRemixSonicDSPChargeAccumulate(FTStruct *fp, s32 max_charge)
{
    FTRemixSonicDSPStatusVars *vars = ftRemixSonicDSPStatusVars(fp);

    if (fp->input.pl.button_tap & (A_BUTTON | B_BUTTON))
    {
        vars->charge_level += SONIC_DSP_CHARGE_PER_PRESS;
        ftParamPlayLoopSFX(fp, sFTRemixSonicDSPClassicTable[fp->player] ? SONIC_DSP_SFX_CHARGE_CLASSIC : SONIC_DSP_SFX_CHARGE);
    }
    vars->charge_level += fp->motion_vars.flags.flag1;
    fp->motion_vars.flags.flag1 = 0;

    if (vars->charge_level > max_charge)
    {
        vars->charge_level = max_charge;
    }
}

/* Release check: temp variable 1 of 0 blocks it, 2 or more forces it, and 1
 * waits until the stick is not held down and B is not held. */
static sb32 ftRemixSonicDSPChargeReleaseAllowed(FTStruct *fp)
{
    u32 release_request = fp->motion_vars.flags.flag0;

    if (release_request == 0)
    {
        return FALSE;
    }
    if (release_request != 1)
    {
        return TRUE;
    }
    if (fp->input.pl.stick_range.y < SONIC_DSP_STICK_DOWN_MIN)
    {
        return FALSE;
    }
    return (fp->input.pl.button_hold & B_BUTTON) == 0;
}

/* SonicDSP.ground_charge_main_ */
static void ftRemixSonicDSPGroundChargeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicDSPChargeAccumulate(fp, SONIC_DSP_MAX_CHARGE_GROUND);
    if (ftRemixSonicDSPChargeReleaseAllowed(fp))
    {
        ftRemixSonicDSPGroundMoveSetStatus(fighter_gobj);
    }
}

/* SonicDSP.air_charge_main_ */
static void ftRemixSonicDSPAirChargeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicDSPChargeAccumulate(fp, SONIC_DSP_MAX_CHARGE_AIR);
    if (ftRemixSonicDSPChargeReleaseAllowed(fp))
    {
        ftRemixSonicDSPAirEndSetStatus(fighter_gobj);
    }
}

/* Adds the slope's pull to the ground speed. The sign comes from the facing direction. */
static void ftRemixSonicDSPApplySlopeSpeed(FTStruct *fp)
{
    f32 slope_angle = ftKirbySpecialLwGetGroundAxisYaw(fp);
    f32 speed_delta = SONIC_DSP_SLOPE_ACCELERATION * __sinf(slope_angle) * -(f32)fp->lr;

    fp->physics.vel_ground.x += speed_delta;
}

/* SonicDSP.ground_move_main_ */
static void ftRemixSonicDSPGroundMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicDSPApplySlopeSpeed(fp);
    ftRemixSonicDSPSetFrameSpeed(fighter_gobj, ftRemixSonicDSPFrameSpeedFromSpeed(fp->physics.vel_ground.x));

    if (fp->physics.vel_ground.x <= ftRemixSonicDSPMinSpeed(fp))
    {
        ftRemixSonicDSPGroundEndSetStatus(fighter_gobj);
        return;
    }
    if (ftCommonKneeBendGetInputTypeCommon(fp) != 0)
    {
        ftRemixSonicDSPAirJumpSetStatus(fighter_gobj);
    }
}

/* SonicDSP.air_move_main_: shared by Air_Move and Air_Jump. */
static void ftRemixSonicDSPAirMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 speed_x = fp->physics.vel_air.x;
    f32 speed_y = fp->physics.vel_air.y;
    f32 absolute_speed;

    ftAnimEndSetFall(fighter_gobj);
    absolute_speed = sqrtf((speed_x * speed_x) + (speed_y * speed_y));
    ftRemixSonicDSPSetFrameSpeed(fighter_gobj, ftRemixSonicDSPFrameSpeedFromSpeed(absolute_speed));
}

/* SonicDSP.air_move_interrupt_: shared by Air_Move and Air_Jump. */
static void ftRemixSonicDSPAirMoveProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 != 0)
    {
        ftCommonJumpProcInterrupt(fighter_gobj);
    }
}

/* SonicDSP.ground_move_physics_: the vanilla ftPhysicsApplyGroundVelFriction
 * (0x800D8BB4) with GROUND_TRACTION in place of the character's traction. */
static void ftRemixSonicDSPGroundMoveProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 friction = dMPCollisionMaterialFrictions[fp->coll_data.floor_flags & MAP_VERTEX_MAT_MASK] * SONIC_DSP_GROUND_TRACTION;

    ftPhysicsSetGroundVelFriction(fp, friction);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/* SonicDSP.air_move_physics_: modified ftPhysicsApplyAirVelFriction (0x800D91EC)
 * with fixed gravity, fall speed and air friction. */
static void ftRemixSonicDSPAirMoveLockedPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes friction_attr = *fp->attr;

    ftPhysicsApplyGravityClampTVel(fp, SONIC_DSP_GRAVITY, SONIC_DSP_MAX_FALL_SPEED);
    friction_attr.air_friction = SONIC_DSP_AIR_FRICTION;
    ftPhysicsApplyAirVelXFriction(fp, &friction_attr);
}

/* SonicDSP.air_movement_physics_: free drift once temp variable 3 is set, locked physics before that. */
static void ftRemixSonicDSPAirMoveProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 != 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
        return;
    }
    ftRemixSonicDSPAirMoveLockedPhysics(fighter_gobj);
}

/* Wall bit that ends the grounded release: lr >= 0 tests the L bit, otherwise the R bit. */
static u16 ftRemixSonicDSPWallCollisionMask(FTStruct *fp)
{
    return (fp->lr >= 0) ? SONIC_DSP_WALL_COLLISION_L : SONIC_DSP_WALL_COLLISION_R;
}

/* SonicDSP.ground_charge_collision_ */
static void ftRemixSonicDSPGroundChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSonicDSPAirChargeTransition);
}

/* SonicDSP.air_charge_collision_ */
static void ftRemixSonicDSPAirChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSonicDSPGroundChargeTransition);
}

/* SonicDSP.ground_end_collision_ */
static void ftRemixSonicDSPGroundEndProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSonicDSPAirEndTransition);
}

/* SonicDSP.air_end_collision_ */
static void ftRemixSonicDSPAirEndProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSonicDSPGroundEndTransition);
}

/* SonicDSP.ground_move_collision_: walking off the floor goes airborne; a wall
 * on the facing side ends the move. */
static void ftRemixSonicDSPGroundMoveProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (mpCommonProcFighterOnFloor(fighter_gobj, ftRemixSonicDSPAirMoveSetStatus) == FALSE)
    {
        return;
    }
    if ((fp->coll_data.mask_prev & ftRemixSonicDSPWallCollisionMask(fp)) != 0)
    {
        ftRemixSonicDSPGroundEndSetStatus(fighter_gobj);
    }
}

/* SonicDSP.air_move_collision_: lands into the ground move while temp variable 3
 * is clear, and into the actionable landing once it is set. */
static void ftRemixSonicDSPAirMoveProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 != 0)
    {
        mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
        return;
    }
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSonicDSPGroundMoveTransition);
}

/* Remix address -> port routine for every Spin Dash routine in the SonicDSP
 * scope of SonicSpecial.asm. */
const FTRemixProc dFTRemixSonicDSPProcs[] =
{
    { REMIX_SYM_SonicDSP__ground_charge_initial_, ftRemixSonicDSPGroundChargeSetStatus },
    { REMIX_SYM_SonicDSP__air_charge_initial_, ftRemixSonicDSPAirChargeSetStatus },
    { REMIX_SYM_SonicDSP__ground_charge_main_, ftRemixSonicDSPGroundChargeProcUpdate },
    { REMIX_SYM_SonicDSP__air_charge_main_, ftRemixSonicDSPAirChargeProcUpdate },
    { REMIX_SYM_SonicDSP__ground_charge_collision_, ftRemixSonicDSPGroundChargeProcMap },
    { REMIX_SYM_SonicDSP__air_charge_collision_, ftRemixSonicDSPAirChargeProcMap },
    { REMIX_SYM_SonicDSP__ground_move_main_, ftRemixSonicDSPGroundMoveProcUpdate },
    { REMIX_SYM_SonicDSP__ground_move_physics_, ftRemixSonicDSPGroundMoveProcPhysics },
    { REMIX_SYM_SonicDSP__ground_move_collision_, ftRemixSonicDSPGroundMoveProcMap },
    { REMIX_SYM_SonicDSP__ground_end_collision_, ftRemixSonicDSPGroundEndProcMap },
    { REMIX_SYM_SonicDSP__air_move_main_, ftRemixSonicDSPAirMoveProcUpdate },
    { REMIX_SYM_SonicDSP__air_move_interrupt_, ftRemixSonicDSPAirMoveProcInterrupt },
    { REMIX_SYM_SonicDSP__air_movement_physics_, ftRemixSonicDSPAirMoveProcPhysics },
    { REMIX_SYM_SonicDSP__air_move_collision_, ftRemixSonicDSPAirMoveProcMap },
    { REMIX_SYM_SonicDSP__air_end_collision_, ftRemixSonicDSPAirEndProcMap }
};

const s32 dFTRemixSonicDSPProcsNum = ARRAY_COUNT(dFTRemixSonicDSPProcs);

#endif
