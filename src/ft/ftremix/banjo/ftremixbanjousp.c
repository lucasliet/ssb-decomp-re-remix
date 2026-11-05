/**
 * ftremixbanjousp.c - C ports of Smash Remix Banjo up special (Shock Spring Jump)
 * routines (.toolchain/pkgs/smashremix-src/src/Banjo/BanjoSpecial.asm, scope BanjoUSP).
 *
 * The moveset commands 0x580000XX and 0x5C0000XX set temp variables 2 and 1
 * (motion_vars.flags.flag1 and flag0) and the routines below read them to pick
 * a phase. Deviations and open questions are in docs/remix/deviations.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>
#include <ft/ftchar/ftfox/ftfoxfunctions.h>
#include <ft/ftchar/ftmario/ftmariofunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <sys/utils.h>

enum
{
    nFTRemixBanjoStatusUSPBegin = 0xEF,
    nFTRemixBanjoStatusUSPAttack = 0xF0,
    nFTRemixBanjoStatusUSPAttackEnd = 0xF1,
    nFTRemixBanjoStatusUSPRecoil = 0xF2,
    nFTRemixBanjoStatusUSPWallSplat = 0xF3
};

enum
{
    BANJO_USP_BEGIN_PHYS_BEGIN = 0,
    BANJO_USP_BEGIN_PHYS_BEGIN_MOVE = 1,
    BANJO_USP_BEGIN_PHYS_MOVE = 2
};

enum
{
    BANJO_USP_ATTACK_PHYS_BEGIN = 0,
    BANJO_USP_ATTACK_PHYS_REAR_BACK = 1,
    BANJO_USP_ATTACK_PHYS_BEGIN_MOVE = 2,
    BANJO_USP_ATTACK_PHYS_MOVE = 3
};

#define BANJO_USP_FLAGS_TRANSITION_ATTACK 3
#define BANJO_USP_FLAGS_RECOIL 3

#define BANJO_USP_BEGIN_Y_SPEED_GROUND 101.0F       /* 0x42CA0000 */
#define BANJO_USP_BEGIN_Y_SPEED_AIR 90.0F           /* 0x42B40000 */
#define BANJO_USP_BEGIN_TRANSITION_FRAME 10.0F      /* 0x41200000 */
#define BANJO_USP_ATTACK_END_FRAME 39.0F            /* 0x421C0000 */
#define BANJO_USP_ATTACK_END_X_SCALE 0.5F           /* 0x3F000000 */
#define BANJO_USP_ATTACK_X_SPEED 80.0F              /* 0x42A00000 */
#define BANJO_USP_ATTACK_REAR_BACK_X_SPEED -20.0F   /* 0xC1A00000 */
#define BANJO_USP_RECOIL_X_SPEED -20.0F             /* 0xC1A00000 */
#define BANJO_USP_RECOIL_Y_SPEED 48.0F              /* 0x42400000 */
#define BANJO_USP_RECOIL_X_DECAY 0.96875F           /* 0x3F780000 */
#define BANJO_USP_RECOIL_Y_RISE 1.25F               /* 0x3FA00000 */
#define BANJO_USP_SPLAT_X_SPEED -15.0F              /* 0xC1700000 */
#define BANJO_USP_SPLAT_Y_SPEED 5.0F                /* 0x40A00000 */
#define BANJO_USP_WALL_ROTATION_DECAY 0.90625F      /* 0x3F680000 */
#define BANJO_USP_WALL_SPLAT_QUAKE_MAGNITUDE 2
#define BANJO_USP_WALL_COLLISION_L 0x0001
#define BANJO_USP_WALL_COLLISION_R 0x0020
#define BANJO_USP_LANDING_LAG 1.0F                  /* 0x3F800000 */
#define BANJO_USP_BEGIN_LANDING_LAG 2.0F            /* 0x40000000 */
/* The copied Mario collision keeps its `ori 0x5C29` low half, so the lag is the
 * Banjo high half (0x3F80 or 0x4000) with 0x5C29 below it. */
#define BANJO_USP_COLLISION_LANDING_LAG 1.00281250476837158203125F       /* 0x3F805C29 */
#define BANJO_USP_BEGIN_COLLISION_LANDING_LAG 2.00562500953674316406250F /* 0x40005C29 */

/* Per-player copies of the player struct words the asm keeps in the status
 * variable union (0xB18 button buffer, 0xB2C grounded flag). */
static u16 sFTRemixBanjoUSPButtonPressBuffer[MAXCONTROLLERS];
static sb32 sFTRemixBanjoUSPIsGrounded[MAXCONTROLLERS];

/* Ors this frame's taps into the per-player buffer and returns it. The buffer is
 * sticky until the next initial_, as in the asm, not a one-frame buffer. */
static u16 ftRemixBanjoUSPBufferButtons(FTStruct *fp)
{
    u16 buttons = fp->input.pl.button_tap | sFTRemixBanjoUSPButtonPressBuffer[fp->player];

    sFTRemixBanjoUSPButtonPressBuffer[fp->player] = buttons;

    return buttons;
}

static void ftRemixBanjoUSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Gravity or fast fall, chosen by the is_fastfall bit. The asm then branches on v0, which
 * the void gravity calls never write (see notes), so the port always takes the run path. */
static void ftRemixBanjoUSPApplyGravity(FTStruct *fp)
{
    if (fp->is_fastfall)
    {
        ftPhysicsApplyFastFall(fp, fp->attr);
    }
    else ftPhysicsApplyGravityDefault(fp, fp->attr);
}

/* y += gravity, which cancels the gravity applied this frame ("negate gravity" in the asm). */
static void ftRemixBanjoUSPNegateGravityVelY(FTStruct *fp)
{
    fp->physics.vel_air.y += fp->attr->gravity;
}

static void ftRemixBanjoUSPFreezeVel(FTStruct *fp)
{
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
}

/* BanjoUSP.begin_recoil_: lands the fighter in the air if grounded, then starts the recoil status. */
static void ftRemixBanjoUSPRecoilSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusUSPRecoil, 0.0F, 1.0F, BANJO_USP_FLAGS_RECOIL);
}

/* BanjoUSP.wall_splat_initial_: starts the wall splat status and the quake. */
static void ftRemixBanjoUSPWallSplatSetStatus(GObj *fighter_gobj, Vec3f *wall_angle)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    DObj *topjoint = (DObj *)fighter_gobj->obj;
    Vec3f pos;

    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusUSPWallSplat, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    pos.x = topjoint->translate.vec.f.x;
    pos.y = topjoint->translate.vec.f.y;
    pos.z = 0.0F;
    ftRemixSizeAttachGroundGfx(efManagerImpactWaveMakeEffect(&pos, SYVECTOR_AXIS_Z, syUtilsArcTan2(-wall_angle->x, wall_angle->y)), fighter_gobj);
    efManagerQuakeMakeEffect(BANJO_USP_WALL_SPLAT_QUAKE_MAGNITUDE);
    fp->physics.vel_air.y = BANJO_USP_SPLAT_Y_SPEED;
    fp->physics.vel_air.x = 0.0F;
    ftRemixBanjoUSPClearTempVars(fp);
}

/* Starts the aerial attack phase and resumes the recoil attack status. Clears the temp variables
 * before the status change, as the asm does. */
static void ftRemixBanjoUSPTransitionAttack(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixBanjoUSPClearTempVars(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusUSPAttack, 0.0F, 1.0F, BANJO_USP_FLAGS_TRANSITION_ATTACK);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftFoxSpecialHiHoldInitStatusVars(fighter_gobj);
}

static void ftRemixBanjoUSPAttackEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusUSPAttackEnd, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.x *= BANJO_USP_ATTACK_END_X_SCALE;
}

/* Temp variable 1 (flag0) set means a wall was hit during the attack. */
static void ftRemixBanjoUSPCheckRecoil(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 == 0)
    {
        return;
    }
    ftRemixBanjoUSPRecoilSetStatus(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
    fp->physics.vel_air.x = BANJO_USP_RECOIL_X_SPEED * (f32)fp->lr;
    fp->physics.vel_air.y = BANJO_USP_RECOIL_Y_SPEED;
}

/* BanjoUSP.initial_ (special entry for ground and air): sets the status and the begin variables. */
static void ftRemixBanjoUSPInitialSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    sFTRemixBanjoUSPIsGrounded[fp->player] = (fp->ga == nMPKineticsGround);
    if (sFTRemixBanjoUSPIsGrounded[fp->player])
    {
        mpCommonSetFighterAir(fp);
    }
    fp->proc_status = ftCaptainSpecialHiProcStatus;
    ftMainSetStatus(fighter_gobj, nFTRemixBanjoStatusUSPBegin, 0.0F, 1.0F, 0);
    ftRemixBanjoUSPClearTempVars(fp);
    sFTRemixBanjoUSPButtonPressBuffer[fp->player] = 0;
    /* The asm passes `lw a0, 0x000(sp)` here, an unset stack slot, so the fighter GObj is
     * used instead; see the notes file. */
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* BanjoUSP.begin_main_: waits for the animation, and goes to the attack when B was pressed after frame 10. */
static void ftRemixBanjoUSPBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 buttons = ftRemixBanjoUSPBufferButtons(fp);

    if ((fighter_gobj->anim_frame > BANJO_USP_BEGIN_TRANSITION_FRAME) && (buttons & B_BUTTON))
    {
        ftRemixBanjoUSPTransitionAttack(fighter_gobj);
        return;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, BANJO_USP_BEGIN_LANDING_LAG, FALSE);
    }
}

/* BanjoUSP.attack_main_: starts the attack end when the animation reaches frame 39. */
static void ftRemixBanjoUSPAttackProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame != BANJO_USP_ATTACK_END_FRAME)
    {
        return;
    }
    ftRemixBanjoUSPAttackEndSetStatus(fighter_gobj);
}

/* BanjoUSP.attack_end_main_: special fall when the animation ends. */
static void ftRemixBanjoUSPAttackEndProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, BANJO_USP_LANDING_LAG, FALSE);
}

/* BanjoUSP.wall_splat_main_: special fall at the end, otherwise eases the top joint's x rotation
 * back to 0 while temp variable 1 is set. */
static void ftRemixBanjoUSPWallSplatProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    DObj *topjoint = (DObj *)fighter_gobj->obj;

    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, BANJO_USP_BEGIN_LANDING_LAG, FALSE);
        return;
    }
    if (fp->motion_vars.flags.flag0 != 0)
    {
        topjoint->rotate.vec.f.x *= BANJO_USP_WALL_ROTATION_DECAY;
    }
}

/* BanjoUSP.attack_interupt_: turns the fighter on flag1 == 2 and checks for a recoil. */
static void ftRemixBanjoUSPAttackProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 2)
    {
        ftCaptainSpecialHiProcInterrupt(fighter_gobj);
    }
    ftRemixBanjoUSPCheckRecoil(fighter_gobj);
}

/* BanjoUSP.recoil_move_ (interrupt of the recoil): slows x and raises y while flag0 is 0. */
static void ftRemixBanjoUSPRecoilProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        return;
    }
    fp->physics.vel_air.x *= BANJO_USP_RECOIL_X_DECAY;
    fp->physics.vel_air.y += BANJO_USP_RECOIL_Y_RISE;
}

/* BanjoUSP.begin_physics_: gravity, air friction, then the BEGIN and BEGIN_MOVE velocity setup. */
static void ftRemixBanjoUSPBeginProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 state = fp->motion_vars.flags.flag2;

    ftRemixBanjoUSPApplyGravity(fp);
    ftPhysicsApplyAirVelXFriction(fp, fp->attr);
    if (state == BANJO_USP_BEGIN_PHYS_BEGIN)
    {
        ftRemixBanjoUSPFreezeVel(fp);
    }
    if (state == BANJO_USP_BEGIN_PHYS_BEGIN_MOVE)
    {
        fp->physics.vel_air.y = sFTRemixBanjoUSPIsGrounded[fp->player] ? BANJO_USP_BEGIN_Y_SPEED_GROUND : BANJO_USP_BEGIN_Y_SPEED_AIR;
        fp->motion_vars.flags.flag2 = BANJO_USP_BEGIN_PHYS_MOVE;
        fp->jumps_used = (u8)fp->attr->jumps_max;
    }
    ftPhysicsClampAirVelXStickDefault(fp, fp->attr);
}

/* BanjoUSP.attack_physics_: gravity, air control, friction, and the REAR_BACK, BEGIN_MOVE and MOVE velocities. */
static void ftRemixBanjoUSPAttackProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 state = fp->motion_vars.flags.flag2;

    ftRemixBanjoUSPApplyGravity(fp);
    ftPhysicsCheckClampAirVelXDecMax(fp, fp->attr);
    ftPhysicsApplyAirVelXFriction(fp, fp->attr);
    if (state == BANJO_USP_ATTACK_PHYS_BEGIN)
    {
        ftRemixBanjoUSPFreezeVel(fp);
    }
    if (state == BANJO_USP_ATTACK_PHYS_REAR_BACK)
    {
        fp->physics.vel_air.x = BANJO_USP_ATTACK_REAR_BACK_X_SPEED * (f32)fp->lr;
        fp->physics.vel_air.y = 0.0F;
        ftRemixBanjoUSPNegateGravityVelY(fp);
        return;
    }
    if (state == BANJO_USP_ATTACK_PHYS_BEGIN_MOVE)
    {
        fp->physics.vel_air.x = BANJO_USP_ATTACK_X_SPEED * (f32)fp->lr;
        fp->physics.vel_air.y = 0.0F;
        fp->motion_vars.flags.flag2 = BANJO_USP_ATTACK_PHYS_MOVE;
    }
    if (fp->motion_vars.flags.flag2 == BANJO_USP_ATTACK_PHYS_MOVE)
    {
        ftRemixBanjoUSPNegateGravityVelY(fp);
    }
}

/* BanjoUSP.recoil_physics_: drift while flag0 is set, friction otherwise. */
static void ftRemixBanjoUSPRecoilProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
    }
    else ftPhysicsApplyAirVelFriction(fighter_gobj);
}

/* BanjoUSP.splat_physics_: sets the splat x velocity once (flag2), then runs recoil_physics_. */
static void ftRemixBanjoUSPSplatProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 != 0)
    {
        fp->motion_vars.flags.flag2 = 0;
        fp->physics.vel_air.x = BANJO_USP_SPLAT_X_SPEED * (f32)fp->lr;
    }
    ftRemixBanjoUSPRecoilProcPhysics(fighter_gobj);
}

/* Mario's up special collision (0x80156358) with the landing lag as a parameter. The asm copies
 * it from ROM 0xD0D98 and 0xD0E14 and replaces only the lui of the lag. */
static void ftRemixBanjoUSPMapCommon(GObj *fighter_gobj, f32 landing_lag)
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
        return;
    }
    ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, landing_lag);
}

/* BanjoUSP.begin_collision_ */
static void ftRemixBanjoUSPBeginProcMap(GObj *fighter_gobj)
{
    ftRemixBanjoUSPMapCommon(fighter_gobj, BANJO_USP_BEGIN_COLLISION_LANDING_LAG);
}

/* BanjoUSP.collision_ */
static void ftRemixBanjoUSPProcMap(GObj *fighter_gobj)
{
    ftRemixBanjoUSPMapCommon(fighter_gobj, BANJO_USP_COLLISION_LANDING_LAG);
}

/* BanjoUSP.attack_collision_: collision_, then a wall splat when moving into a wall during MOVE. */
static void ftRemixBanjoUSPAttackProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    DObj *topjoint = (DObj *)fighter_gobj->obj;
    u16 wall_mask;
    Vec3f *wall_angle;

    ftRemixBanjoUSPProcMap(fighter_gobj);
    if (fp->motion_vars.flags.flag2 != BANJO_USP_ATTACK_PHYS_MOVE)
    {
        return;
    }
    /* The asm reads 0xCC here, which the decomp names coll_data.mask_prev; see notes. */
    wall_mask = fp->coll_data.mask_prev;
    if (fp->lr >= 0)
    {
        wall_mask &= BANJO_USP_WALL_COLLISION_L;
        wall_angle = &fp->coll_data.lwall_angle;
    }
    else
    {
        wall_mask &= BANJO_USP_WALL_COLLISION_R;
        wall_angle = &fp->coll_data.rwall_angle;
    }
    if (wall_mask == 0)
    {
        return;
    }
    ftRemixBanjoUSPWallSplatSetStatus(fighter_gobj, wall_angle);
    topjoint->rotate.vec.f.x = wall_angle->y;
}

/* Remix address -> port routine for every BanjoUSP routine in the worklist. */
const FTRemixProc dFTRemixBanjoUSPProcs[] =
{
    { REMIX_SYM_BanjoUSP__initial_, ftRemixBanjoUSPInitialSetStatus },
    { REMIX_SYM_BanjoUSP__begin_main_, ftRemixBanjoUSPBeginProcUpdate },
    { REMIX_SYM_BanjoUSP__begin_physics_, ftRemixBanjoUSPBeginProcPhysics },
    { REMIX_SYM_BanjoUSP__begin_collision_, ftRemixBanjoUSPBeginProcMap },
    { REMIX_SYM_BanjoUSP__attack_main_, ftRemixBanjoUSPAttackProcUpdate },
    { REMIX_SYM_BanjoUSP__attack_interupt_, ftRemixBanjoUSPAttackProcInterrupt },
    { REMIX_SYM_BanjoUSP__attack_physics_, ftRemixBanjoUSPAttackProcPhysics },
    { REMIX_SYM_BanjoUSP__attack_collision_, ftRemixBanjoUSPAttackProcMap },
    { REMIX_SYM_BanjoUSP__attack_end_main_, ftRemixBanjoUSPAttackEndProcUpdate },
    { REMIX_SYM_BanjoUSP__collision_, ftRemixBanjoUSPProcMap },
    { REMIX_SYM_BanjoUSP__recoil_move_, ftRemixBanjoUSPRecoilProcInterrupt },
    { REMIX_SYM_BanjoUSP__recoil_physics_, ftRemixBanjoUSPRecoilProcPhysics },
    { REMIX_SYM_BanjoUSP__splat_physics_, ftRemixBanjoUSPSplatProcPhysics },
    { REMIX_SYM_BanjoUSP__wall_splat_main_, ftRemixBanjoUSPWallSplatProcUpdate }
};

const s32 dFTRemixBanjoUSPProcsNum = ARRAY_COUNT(dFTRemixBanjoUSPProcs);

#endif
