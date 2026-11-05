/**
 * ftremixwario.c - C ports of Smash Remix Wario routines
 * (.toolchain/pkgs/smashremix-src/src/Wario/WarioSpecial.asm).
 *
 * Wario's specials copy Mario's, Fox's and Captain's vanilla code with
 * OS.copy_segment and patch a few instructions. Each port reproduces the
 * copied logic inline, with the Remix patch applied.
 *
 * Temp variables: flag0 = temp 1 (5400XXXX), flag1 = temp 2 (5800XXXX),
 * flag2 = temp 3 (5C00XXXX).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include "ft/ftcommon.h"
#include "ft/ftcommon/ftcommonfunctions.h"
#include "ft/ftmain.h"
#include "ft/ftphysics.h"
#include "ef/efmanager.h"
#include "mp/map.h"
#include "mp/mpcommon.h"
#include "mp/mpdef.h"
#include "sys/utils.h"

extern void *func_800269C0_275C0(u16 id);
extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);
extern void ftCaptainSpecialHiProcInterrupt(GObj *fighter_gobj);
extern f32 ftCaptainSpecialNGetAngle(s32 stick_y);

enum
{
    nFTRemixWarioStatusBodySlam = 0xDF,
    nFTRemixWarioStatusBodySlamAir = 0xE0,
    nFTRemixWarioStatusCorkscrew = 0xE1,
    nFTRemixWarioStatusGroundPoundLanding = 0xE2,
    nFTRemixWarioStatusGroundPound = 0xE3,
    nFTRemixWarioStatusGroundPoundAir = 0xE4,
    nFTRemixWarioStatusNSPRecoilGround = 0xE5,
    nFTRemixWarioStatusNSPRecoilAir = 0xE6,
    nFTRemixWarioKirbyStatusNSPGround = 0x138,
    nFTRemixWarioKirbyStatusNSPAir = 0x139,
    nFTRemixWarioKirbyStatusNSPRecoilGround = 0x13A,
    nFTRemixWarioKirbyStatusNSPRecoilAir = 0x13B
};

enum
{
    WARIO_TEMP3_BEGIN = 1,
    WARIO_TEMP3_MOVE = 2
};

enum
{
    WARIO_USP_TEMP2_TURN = 2,
    WARIO_USP_TEMP3_BEGIN = 1,
    WARIO_USP_TEMP3_BEGIN_MOVE = 2,
    WARIO_USP_TEMP3_MOVE = 3,
    WARIO_USP_TEMP3_END_MOVE = 4
};

#define WARIO_FGM_RECOIL 0x0117
#define WARIO_FGM_JUMP 0x005E

#define WARIO_WALL_COLLISION_L 0x0001
#define WARIO_WALL_COLLISION_R 0x0020

#define WARIO_MOVE_DECAY 0.875F               /* 0x3F60 */
#define WARIO_GROUND_TRACTION 0.5F            /* 0x3F00 */
#define WARIO_AIR_CONTROL_MOVE 0.0234375F     /* 0x3CC00000 */
#define WARIO_AIR_CONTROL_END_MOVE 0.0078125F /* 0x3C000000 */

#define WARIO_NSP_X_SPEED 64.0F
#define WARIO_NSP_Y_SPEED 30.0F
#define WARIO_NSP_JUMP_SPEED 68.0F
#define WARIO_NSP_RECOIL_X_SPEED -40.0F
#define WARIO_NSP_RECOIL_Y_SPEED 40.0F
#define WARIO_NSP_GRAVITY 2.75F
#define WARIO_NSP_MAX_FALL_SPEED 48.0F
#define WARIO_NSP_AIR_FRICTION 2.0F
#define WARIO_NSP_ANGLE_Y_SCALE 0.75F         /* 0x3F40 */
#define WARIO_NSP_RECOIL_X_DECAY 0.96875F     /* 0x3F78 */
#define WARIO_NSP_RECOIL_Y_BOOST 1.25F        /* 0x3FA0 */
#define WARIO_NSP_RECOIL_Y_BOOST_KIRBY 0.55F  /* 0x3F0CCCCD */

#define WARIO_DSP_Y_SPEED -80.0F
#define WARIO_DSP_INITIAL_X_SPEED 90.0F
#define WARIO_DSP_INITIAL_Y_SPEED 180.0F

#define WARIO_USP_Y_SPEED 64.0F
#define WARIO_USP_STICK_X_SCALE 0.5F          /* 0x3F000000 */
#define WARIO_USP_Y_PER_X_SPEED 0.21875F      /* 0x3E600000 */
#define WARIO_USP_LANDING_LAG 0.25F           /* 0x3E800000, Remix LANDING_FSM */
/* Mario's 0x3E8F5C29 with Remix's upper half 0x3E80 (see CollisionLanding) */
#define WARIO_USP_LANDING_LAG_MAP 0.2507030964F /* 0x3E805C29 */

/* Jim's special collision flag, one byte per port (Remix special_jim_flag). */
static u8 sFTRemixWarioSpecialJimFlag[MAXCONTROLLERS];

/* Kirby and J Kirby run Wario's specials under their own action ids. */
static sb32 ftRemixWarioIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

static s32 ftRemixWarioSpecialAction(FTStruct *fp, s32 kirby_action, s32 wario_action)
{
    return ftRemixWarioIsKirby(fp) ? kirby_action : wario_action;
}

static void ftRemixWarioResetSpecialVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = WARIO_TEMP3_BEGIN;
}

/* Clears absorb, absorb direction, goto attack 100 and fast fall (the andi 0x07 on 0x18D). */
static void ftRemixWarioClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Vanilla ftPhysicsApplyAirVelXFriction with the friction passed in (the Remix asm passes a stack copy). */
static void ftRemixWarioApplyAirXFriction(FTStruct *fp, f32 friction)
{
    if (fp->physics.vel_air.x < 0.0F)
    {
        fp->physics.vel_air.x += friction;

        if (fp->physics.vel_air.x >= 0.0F)
        {
            fp->physics.vel_air.x = 0.0F;
        }
    }
    else
    {
        fp->physics.vel_air.x -= friction;

        if (fp->physics.vel_air.x <= 0.0F)
        {
            fp->physics.vel_air.x = 0.0F;
        }
    }
}

/* Air physics that lets the player steer only when temp 2 is set (0x800D90E0 or 0x800D91EC). */
static void ftRemixWarioSpecialAirProcPhysicsControlled(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
    }
    else ftPhysicsApplyAirVelFriction(fighter_gobj);
}

/* Wall bit of mask_prev for the side the fighter faces. */
static sb32 ftRemixWarioHasWallCollision(FTStruct *fp)
{
    u16 wall_mask = (fp->lr >= 0) ? WARIO_WALL_COLLISION_L : WARIO_WALL_COLLISION_R;

    return (fp->coll_data.mask_prev & wall_mask) != 0;
}

/* Grounded wall recoil: arms temp 1 unless Jim's flag is already set. */
static void ftRemixWarioSpecialNGroundCheckWallRecoil(FTStruct *fp)
{
    u8 *jim_flag = &sFTRemixWarioSpecialJimFlag[fp->player];

    if ((fp->motion_vars.flags.flag2 != WARIO_TEMP3_MOVE) || !ftRemixWarioHasWallCollision(fp))
    {
        *jim_flag = FALSE;
        return;
    }
    if (*jim_flag == FALSE)
    {
        fp->motion_vars.flags.flag0 = TRUE;
    }
}

/* Aerial wall flag: sets Jim's flag on a wall hit during the move, which suppresses the grounded recoil. */
static void ftRemixWarioSpecialNAirCheckWallJim(FTStruct *fp)
{
    u8 *jim_flag = &sFTRemixWarioSpecialJimFlag[fp->player];

    if ((fp->motion_vars.flags.flag2 != WARIO_TEMP3_MOVE) || !ftRemixWarioHasWallCollision(fp))
    {
        *jim_flag = FALSE;
        return;
    }
    *jim_flag = TRUE;
}

/* begin_recoil_: plays the recoil FGM, grounded fighters become airborne, then the air recoil action starts. */
static void ftRemixWarioSpecialNBeginRecoil(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPRecoilAir, nFTRemixWarioStatusNSPRecoilAir);

    func_800269C0_275C0(WARIO_FGM_RECOIL);
    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
}

/* check_recoil_: when temp 1 is armed, starts the recoil. TRUE means the caller must stop (the asm returns to its caller). */
static sb32 ftRemixWarioSpecialNCheckRecoil(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 == 0)
    {
        return FALSE;
    }
    ftRemixWarioSpecialNBeginRecoil(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
    fp->physics.vel_air.x = WARIO_NSP_RECOIL_X_SPEED * fp->lr;
    fp->physics.vel_air.y = WARIO_NSP_RECOIL_Y_SPEED;
    return TRUE;
}

/* Remix ground_to_air_: grounded NSP becomes the aerial NSP. */
static void ftRemixWarioSpecialNGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPAir, nFTRemixWarioStatusBodySlamAir);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
}

/* Remix air_to_ground_: aerial NSP lands into the grounded NSP. */
static void ftRemixWarioSpecialNAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPGround, nFTRemixWarioStatusBodySlam);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
}

/* Grounded NSP jump: launches upward with the jump smoke and sound. */
static void ftRemixWarioSpecialNGroundJump(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixWarioSpecialNGroundToAir(fighter_gobj);
    fp->physics.vel_air.y = WARIO_NSP_JUMP_SPEED;
    efManagerDustHeavyDoubleMakeEffect(fp->coll_data.p_translate, 1, 1.0F);
    func_800269C0_275C0(WARIO_FGM_JUMP);
}

/* Remix recoil_ground_transition_: recoil that lands becomes the grounded recoil action. */
static void ftRemixWarioSpecialNRecoilGroundTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPRecoilGround, nFTRemixWarioStatusNSPRecoilGround);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* Remix recoil_air_transition_: recoil that leaves the ground becomes the aerial recoil action. */
static void ftRemixWarioSpecialNRecoilAirTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPRecoilAir, nFTRemixWarioStatusNSPRecoilAir);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
}

/* WarioNSP.ground_initial_ */
static void ftRemixWarioSpecialNGroundSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPGround, nFTRemixWarioStatusBodySlam);

    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixWarioResetSpecialVars(fp);
}

/* WarioNSP.air_initial_ */
static void ftRemixWarioSpecialNAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPAir, nFTRemixWarioStatusBodySlamAir);

    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixWarioResetSpecialVars(fp);
    ftRemixWarioClearAirFlags(fp);
    fp->physics.vel_air.y = WARIO_NSP_GRAVITY;
}

/* WarioNSP.ground_move_ */
static void ftRemixWarioSpecialNGroundProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_BEGIN)
    {
        fp->physics.vel_ground.x *= WARIO_MOVE_DECAY;
    }
    else if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_MOVE)
    {
        fp->physics.vel_ground.x = WARIO_NSP_X_SPEED;
    }
}

/* WarioNSP.air_move_: the stick angle sets the rise when temp 2 is armed. */
static void ftRemixWarioSpecialNAirProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_BEGIN)
    {
        fp->physics.vel_air.x *= WARIO_MOVE_DECAY;
        fp->physics.vel_air.y = WARIO_NSP_GRAVITY;
    }
    else if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_MOVE)
    {
        fp->physics.vel_air.x = WARIO_NSP_X_SPEED * fp->lr;
    }
    if (fp->motion_vars.flags.flag1 != 0)
    {
        fp->motion_vars.flags.flag1 = 0;
        fp->physics.vel_air.y = __sinf(ftCaptainSpecialNGetAngle(fp->input.pl.stick_range.y)) * WARIO_NSP_X_SPEED * WARIO_NSP_ANGLE_Y_SCALE + WARIO_NSP_Y_SPEED;
    }
}

/* WarioNSP.ground_physics_: Mario's ground friction with Wario's fixed traction. */
static void ftRemixWarioSpecialNGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 friction = grRemixSurfaceFriction(fp->coll_data.floor_flags & MAP_VERTEX_MAT_MASK);

    ftPhysicsSetGroundVelFriction(fp, friction * WARIO_GROUND_TRACTION);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/* WarioNSP.air_physics_: gravity with a max fall speed, then the fixed air friction. */
static void ftRemixWarioSpecialNAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftPhysicsApplyGravityClampTVel(fp, WARIO_NSP_GRAVITY, WARIO_NSP_MAX_FALL_SPEED);
    ftRemixWarioApplyAirXFriction(fp, WARIO_NSP_AIR_FRICTION);
}

/* WarioNSP.ground_collision_ */
static void ftRemixWarioSpecialNGroundProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixWarioSpecialAction(fp, nFTRemixWarioKirbyStatusNSPGround, nFTRemixWarioStatusBodySlam);

    ftRemixWarioSpecialNGroundCheckWallRecoil(fp);
    if (ftRemixWarioSpecialNCheckRecoil(fighter_gobj))
    {
        return;
    }
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixWarioSpecialNGroundToAir);
    if (fp->status_id != action)
    {
        return;
    }
    if ((fp->motion_vars.flags.flag2 != WARIO_TEMP3_MOVE) || (ftCommonKneeBendGetInputTypeCommon(fp) == FTCOMMON_KNEEBEND_INPUT_TYPE_NONE))
    {
        return;
    }
    ftRemixWarioSpecialNGroundJump(fighter_gobj);
}

/* WarioNSP.air_collision_ */
static void ftRemixWarioSpecialNAirProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixWarioSpecialNAirCheckWallJim(fp);
    if (ftRemixWarioSpecialNCheckRecoil(fighter_gobj))
    {
        return;
    }
    mpCommonProcFighterCliff(fighter_gobj, ftRemixWarioSpecialNAirToGround);
}

/* WarioNSP.recoil_ground_collision_ */
static void ftRemixWarioSpecialNRecoilGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixWarioSpecialNRecoilAirTransition);
}

/* WarioNSP.recoil_air_collision_ */
static void ftRemixWarioSpecialNRecoilAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixWarioSpecialNRecoilGroundTransition);
}

/* WarioNSP.recoil_move_: the air recoil slows its x speed and gets a rise boost. */
static void ftRemixWarioSpecialNRecoilAirProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 boost = (fp->fkind == REMIX_ID_WARIO) ? WARIO_NSP_RECOIL_Y_BOOST : WARIO_NSP_RECOIL_Y_BOOST_KIRBY;

    if (fp->motion_vars.flags.flag1 != 0)
    {
        return;
    }
    fp->physics.vel_air.x *= WARIO_NSP_RECOIL_X_DECAY;
    fp->physics.vel_air.y += boost;
}

/* WarioUSP.initial_ */
static void ftRemixWarioSpecialHiSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, nFTRemixWarioStatusCorkscrew, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftRemixWarioResetSpecialVars(fp);
    ftRemixWarioClearAirFlags(fp);
    fp->physics.vel_air.y = fp->attr->gravity;
}

/* WarioUSP.main_: Fox's up special end (Mario's copy) with Wario's landing lag and no interrupt. */
static void ftRemixWarioSpecialHiProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, WARIO_USP_LANDING_LAG, FALSE);
    }
}

/* WarioUSP.change_direction_ */
static void ftRemixWarioSpecialHiProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == WARIO_USP_TEMP2_TURN)
    {
        ftCaptainSpecialHiProcInterrupt(fighter_gobj);
    }
}

/* Up special air control: drift is scaled by the move phase (0x800D8FC8 with a2 chosen by temp 3). */
static void ftRemixWarioSpecialHiAirControl(FTStruct *fp, FTAttributes *attr, s32 state)
{
    f32 accel = attr->air_accel;

    if (state == WARIO_USP_TEMP3_MOVE)
    {
        accel = WARIO_AIR_CONTROL_MOVE;
    }
    else if (state == WARIO_USP_TEMP3_END_MOVE)
    {
        accel = WARIO_AIR_CONTROL_END_MOVE;
    }
    ftPhysicsClampAirVelXStickRange(fp, FTPHYSICS_AIRDRIFT_CLAMP_RANGE_MIN, accel, attr->air_speed_max_x);
}

/* Fall step, then air control and friction unless the decrement check already clamped x. */
static void ftRemixWarioSpecialHiApplyAirVel(FTStruct *fp, FTAttributes *attr, s32 state)
{
    if (fp->is_fastfall)
    {
        ftPhysicsApplyFastFall(fp, attr);
    }
    else ftPhysicsApplyGravityDefault(fp, attr);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) != FALSE)
    {
        return;
    }
    if (state != WARIO_USP_TEMP3_BEGIN)
    {
        ftRemixWarioSpecialHiAirControl(fp, attr, state);
    }
    ftPhysicsApplyAirVelXFriction(fp, attr);
}

/* Begin-move: sets the launch velocity from the stick and starts the move. */
static void ftRemixWarioSpecialHiStartMove(FTStruct *fp)
{
    f32 direction = fp->lr;
    f32 stick_toward_facing = fp->input.pl.stick_range.x * direction;
    f32 vel_x = 0.0F;
    f32 vel_y = WARIO_USP_Y_SPEED;

    if (stick_toward_facing >= 0.0F)
    {
        vel_x = stick_toward_facing * WARIO_USP_STICK_X_SCALE;
        vel_y -= vel_x * WARIO_USP_Y_PER_X_SPEED;
    }
    fp->physics.vel_air.x = vel_x * direction;
    fp->physics.vel_air.y = vel_y;
    fp->motion_vars.flags.flag2 = WARIO_USP_TEMP3_MOVE;
    fp->jumps_used = fp->attr->jumps_max;
}

/* Freezes the fall and slows x; used by the begin and end phases. */
static void ftRemixWarioSpecialHiSlowAndFreeze(FTStruct *fp)
{
    fp->physics.vel_air.x *= WARIO_MOVE_DECAY;
    fp->physics.vel_air.y = 0.0F;
}

/* WarioUSP.physics_: Mario's air physics, then the phase-specific velocity. */
static void ftRemixWarioSpecialHiProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;
    s32 state = fp->motion_vars.flags.flag2;

    ftRemixWarioSpecialHiApplyAirVel(fp, attr, state);
    if (state == WARIO_USP_TEMP3_BEGIN)
    {
        ftRemixWarioSpecialHiSlowAndFreeze(fp);
    }
    else if (state == WARIO_USP_TEMP3_BEGIN_MOVE)
    {
        ftRemixWarioSpecialHiStartMove(fp);
    }
    else if (state == WARIO_USP_TEMP3_MOVE)
    {
        fp->physics.vel_air.y += attr->gravity;
    }
    else if (state == WARIO_USP_TEMP3_END_MOVE)
    {
        ftRemixWarioSpecialHiSlowAndFreeze(fp);
    }
}

/* WarioUSP.collision_: Mario's up special collision with Wario's landing lag. */
static void ftRemixWarioSpecialHiProcMap(GObj *fighter_gobj)
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
    if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) == FALSE)
    {
        return;
    }
    if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
    {
        ftCommonCliffCatchSetStatus(fighter_gobj);
    }
    else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, WARIO_USP_LANDING_LAG_MAP);
}

/* WarioDSP.ground_initial_ */
static void ftRemixWarioSpecialLwGroundSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixWarioStatusGroundPound, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixWarioResetSpecialVars(fp);
}

/* WarioDSP.air_initial_ */
static void ftRemixWarioSpecialLwAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixWarioStatusGroundPoundAir, 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixWarioResetSpecialVars(fp);
    ftRemixWarioClearAirFlags(fp);
}

/* WarioDSP.ground_move_ */
static void ftRemixWarioSpecialLwGroundProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->physics.vel_air.x *= WARIO_MOVE_DECAY;
    if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_BEGIN)
    {
        fp->physics.vel_air.y *= WARIO_MOVE_DECAY;
    }
    if (fp->motion_vars.flags.flag0 == 0)
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;
    fp->physics.vel_air.x = WARIO_DSP_INITIAL_X_SPEED * fp->lr;
    fp->physics.vel_air.y = WARIO_DSP_INITIAL_Y_SPEED;
    mpCommonSetFighterAir(fp);
}

/* WarioDSP.air_move_ */
static void ftRemixWarioSpecialLwAirProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->physics.vel_air.x *= WARIO_MOVE_DECAY;
    if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_BEGIN)
    {
        fp->physics.vel_air.y *= WARIO_MOVE_DECAY;
    }
}

/* WarioDSP.physics_: controlled air physics, then the ground-pound fall speed. */
static void ftRemixWarioSpecialLwProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixWarioSpecialAirProcPhysicsControlled(fighter_gobj);
    if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_BEGIN)
    {
        if (fp->physics.vel_air.y < 0.0F)
        {
            fp->physics.vel_air.y = 0.0F;
        }
    }
    else if (fp->motion_vars.flags.flag2 == WARIO_TEMP3_MOVE)
    {
        fp->physics.vel_air.y = WARIO_DSP_Y_SPEED;
    }
}

/* WarioDSP.begin_landing_: Falcon Kick's landing copy, with Wario's landing action. */
static void ftRemixWarioSpecialLwBeginLanding(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixWarioStatusGroundPoundLanding, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* WarioDSP.collision_ */
static void ftRemixWarioSpecialLwProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    if (fp->motion_vars.flags.flag2 != WARIO_TEMP3_MOVE)
    {
        mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
        return;
    }
    mpCommonProcFighterCliff(fighter_gobj, ftRemixWarioSpecialLwBeginLanding);
}

/* Remix address -> port routine for every Wario routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixWarioProcs[] =
{
    { REMIX_SYM_WarioDSP__air_initial_, ftRemixWarioSpecialLwAirSetStatus },
    { REMIX_SYM_WarioDSP__air_move_, ftRemixWarioSpecialLwAirProcUpdate },
    { REMIX_SYM_WarioDSP__collision_, ftRemixWarioSpecialLwProcMap },
    { REMIX_SYM_WarioDSP__ground_initial_, ftRemixWarioSpecialLwGroundSetStatus },
    { REMIX_SYM_WarioDSP__ground_move_, ftRemixWarioSpecialLwGroundProcUpdate },
    { REMIX_SYM_WarioDSP__physics_, ftRemixWarioSpecialLwProcPhysics },
    { REMIX_SYM_WarioNSP__air_collision_, ftRemixWarioSpecialNAirProcMap },
    { REMIX_SYM_WarioNSP__air_initial_, ftRemixWarioSpecialNAirSetStatus },
    { REMIX_SYM_WarioNSP__air_move_, ftRemixWarioSpecialNAirProcUpdate },
    { REMIX_SYM_WarioNSP__air_physics_, ftRemixWarioSpecialNAirProcPhysics },
    { REMIX_SYM_WarioNSP__ground_collision_, ftRemixWarioSpecialNGroundProcMap },
    { REMIX_SYM_WarioNSP__ground_initial_, ftRemixWarioSpecialNGroundSetStatus },
    { REMIX_SYM_WarioNSP__ground_move_, ftRemixWarioSpecialNGroundProcUpdate },
    { REMIX_SYM_WarioNSP__ground_physics_, ftRemixWarioSpecialNGroundProcPhysics },
    { REMIX_SYM_WarioNSP__recoil_air_collision_, ftRemixWarioSpecialNRecoilAirProcMap },
    { REMIX_SYM_WarioNSP__recoil_ground_collision_, ftRemixWarioSpecialNRecoilGroundProcMap },
    { REMIX_SYM_WarioNSP__recoil_move_, ftRemixWarioSpecialNRecoilAirProcUpdate },
    { REMIX_SYM_WarioNSP__recoil_physics_, ftRemixWarioSpecialAirProcPhysicsControlled },
    { REMIX_SYM_WarioUSP__change_direction_, ftRemixWarioSpecialHiProcInterrupt },
    { REMIX_SYM_WarioUSP__collision_, ftRemixWarioSpecialHiProcMap },
    { REMIX_SYM_WarioUSP__initial_, ftRemixWarioSpecialHiSetStatus },
    { REMIX_SYM_WarioUSP__main_, ftRemixWarioSpecialHiProcUpdate },
    { REMIX_SYM_WarioUSP__physics_, ftRemixWarioSpecialHiProcPhysics }
};

const s32 dFTRemixWarioProcsNum = ARRAY_COUNT(dFTRemixWarioProcs);

#endif
