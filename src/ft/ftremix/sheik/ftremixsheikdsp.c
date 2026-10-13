/**
 * ftremixsheikdsp.c - C ports of Smash Remix Sheik down special routines
 * (.toolchain/pkgs/smashremix-src/src/Sheik/SheikSpecial.asm, scope SheikDSP).
 *
 * The DSP keeps its state in motion_vars.flags (temp variables 1-3 in Remix
 * terms): flag0 is the wall-recoil request, flag1 selects reduced gravity
 * while 0, and flag2 is the sub-phase (BEGIN/ATTACK/RECOIL/LANDING) set by
 * the moveset. Sheik's once-per-airtime DSP flag lives in the generic passive
 * word at 0x0ADC.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

#define SHEIK_DSP_GRAVITY_MULTIPLIER 0.23F      /* 0x3E6B851F */
#define SHEIK_DSP_VERTICAL_VELOCITY 50.0F       /* 0x42480000 */
#define SHEIK_DSP_RECOIL_VERTICAL_VELOCITY 65.0F /* 0x42820000 */
#define SHEIK_DSP_SPEED_MAX 40.0F
#define SHEIK_DSP_SPEED_MIN 20.0F
#define SHEIK_DSP_RECOIL_SPEED 30.0F
#define SHEIK_DSP_RECOIL_SPEED_MAX 40.0F
#define SHEIK_DSP_RECOIL_SPEED_MIN 20.0F
#define SHEIK_DSP_STICK_DEADZONE 8
#define SHEIK_DSP_SHORTEN_FRAME 10.0F
#define SHEIK_DSP_RECOIL_FRAME_MIN 15.0F
#define SHEIK_DSP_RECOIL_FRAME_MAX 40.0F

enum
{
    nFTRemixSheikStatusDSPBegin = 0xF0,
    nFTRemixSheikStatusDSPAttack = 0xF1,
    nFTRemixSheikStatusDSPLanding = 0xF2,
    nFTRemixSheikStatusDSPRecoil = 0xF3
};

enum
{
    SHEIK_DSP_FLAG2_BEGIN = 1
};

/* SheikDSP.button_press_buffer: last frame's tapped buttons per port. The asm
 * keeps only the high byte of button_tap; B_BUTTON lives in that byte, so the
 * 16-bit buffer tests the same bit. */
static u16 sFTRemixSheikDSPButtonPressBuffer[MAXCONTROLLERS];

/* Returns this frame's or last frame's tapped buttons and records this frame's. */
static u16 ftRemixSheikDSPUpdateButtonBuffer(FTStruct *fp)
{
    u16 buffered = sFTRemixSheikDSPButtonPressBuffer[fp->player];

    sFTRemixSheikDSPButtonPressBuffer[fp->player] = fp->input.pl.button_tap;

    return fp->input.pl.button_tap | buffered;
}

/* SheikDSP.dsp_on_hit_ (proc_damage while the DSP runs): re-arms the DSP. */
static void ftRemixSheikDSPOnHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->passive_vars.purin.unk_0x0 = 0;
}

/* SheikDSP.attack_transition (shared helper): switches to the DSP attack phase. */
static void ftRemixSheikDSPAttackTransition(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusDSPAttack, 0.0F, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* SheikDSP.air_to_ground_: landing transition used by the DSP collisions. */
static void ftRemixSheikDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusDSPLanding, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* SheikDSP.check_recoil_: starts the recoil when the wall flag was set.
 * Returns TRUE when the recoil started, and the caller must stop. */
static sb32 ftRemixSheikDSPCheckRecoil(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 == 0)
    {
        return FALSE;
    }
    fp->physics.vel_air.y = SHEIK_DSP_RECOIL_VERTICAL_VELOCITY;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusDSPRecoil, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    return TRUE;
}

/* Clamps `value` to [min, max]. */
static f32 ftRemixSheikDSPClampSpeed(f32 value, f32 min, f32 max)
{
    if (value > max)
    {
        return max;
    }
    if (value < min)
    {
        return min;
    }
    return value;
}

/* SheikDSP.physics_ vertical part: reduced gravity while flag1 is 0, then the
 * shared vertical clamp. Returns the result of ftPhysicsCheckClampAirVelXDecMax. */
static sb32 ftRemixSheikDSPPhysicsVertical(FTStruct *fp)
{
    f32 gravity = fp->attr->gravity;

    if (fp->motion_vars.flags.flag1 == 0)
    {
        gravity *= SHEIK_DSP_GRAVITY_MULTIPLIER;
    }
    ftPhysicsApplyGravityClampTVel(fp, gravity, fp->attr->tvel_base);

    return ftPhysicsCheckClampAirVelXDecMax(fp, fp->attr);
}

/* SheikDSP.physics_ horizontal part: stick adds to the speed, clamped to
 * [20, 40] facing right or [-40, -20] facing left. */
static void ftRemixSheikDSPPhysicsHorizontal(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    sb32 is_facing_right = (fp->lr >= 0);
    s32 stick_mag = (stick_x < 0) ? -stick_x : stick_x;

    if (stick_mag < SHEIK_DSP_STICK_DEADZONE)
    {
        fp->physics.vel_air.x = is_facing_right ? SHEIK_DSP_SPEED_MAX : -SHEIK_DSP_SPEED_MAX;
        return;
    }
    fp->physics.vel_air.x += (f32)stick_x;
    if (is_facing_right)
    {
        fp->physics.vel_air.x = ftRemixSheikDSPClampSpeed(fp->physics.vel_air.x, SHEIK_DSP_SPEED_MIN, SHEIK_DSP_SPEED_MAX);
    }
    else
    {
        fp->physics.vel_air.x = ftRemixSheikDSPClampSpeed(fp->physics.vel_air.x, -SHEIK_DSP_SPEED_MAX, -SHEIK_DSP_SPEED_MIN);
    }
}

/* SheikDSP.recoil_physics_ horizontal part. The asm applies the stick with
 * the opposite sign and clamps with inverted comparisons relative to
 * physics_; it is ported literally. */
static void ftRemixSheikDSPRecoilPhysicsHorizontal(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    sb32 is_facing_right = (fp->lr >= 0);
    s32 stick_mag = (stick_x < 0) ? -stick_x : stick_x;

    if (stick_mag < SHEIK_DSP_STICK_DEADZONE)
    {
        fp->physics.vel_air.x = is_facing_right ? -SHEIK_DSP_RECOIL_SPEED : SHEIK_DSP_RECOIL_SPEED;
        return;
    }
    fp->physics.vel_air.x -= (f32)stick_x;
    if (is_facing_right)
    {
        if (fp->physics.vel_air.x > -SHEIK_DSP_RECOIL_SPEED_MAX)
        {
            fp->physics.vel_air.x = -SHEIK_DSP_RECOIL_SPEED_MAX;
        }
        else if (fp->physics.vel_air.x < -SHEIK_DSP_RECOIL_SPEED_MIN)
        {
            fp->physics.vel_air.x = -SHEIK_DSP_RECOIL_SPEED_MIN;
        }
        return;
    }
    if (fp->physics.vel_air.x < SHEIK_DSP_RECOIL_SPEED_MAX)
    {
        fp->physics.vel_air.x = SHEIK_DSP_RECOIL_SPEED_MAX;
    }
    else if (fp->physics.vel_air.x > SHEIK_DSP_RECOIL_SPEED_MIN)
    {
        fp->physics.vel_air.x = SHEIK_DSP_RECOIL_SPEED_MIN;
    }
}

/* SheikDSP.initial_ (special entry for the ground and air DSP). */
static void ftRemixSheikDSPInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusDSPBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    if (fp->passive_vars.purin.unk_0x0 == 0)
    {
        fp->physics.vel_air.y = SHEIK_DSP_VERTICAL_VELOCITY;
    }
    fp->passive_vars.purin.unk_0x0 = 1;
    fp->proc_damage = ftRemixSheikDSPOnHit;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = SHEIK_DSP_FLAG2_BEGIN;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* SheikDSP.main_ (ProcUpdate of DSP begin): shortens into the attack after
 * frame 10 when B is pressed. */
static void ftRemixSheikDSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 buttons = ftRemixSheikDSPUpdateButtonBuffer(fp);

    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixSheikDSPAttackTransition);

    if (fighter_gobj->anim_frame <= SHEIK_DSP_SHORTEN_FRAME)
    {
        return;
    }
    if (!(buttons & B_BUTTON))
    {
        return;
    }
    ftRemixSheikDSPAttackTransition(fighter_gobj);
}

/* SheikDSP.physics_ (ProcPhysics of DSP begin and attack). */
static void ftRemixSheikDSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixSheikDSPPhysicsVertical(fp))
    {
        return;
    }
    ftRemixSheikDSPPhysicsHorizontal(fp);
}

/* SheikDSP.air_collision_ (ProcMap of DSP begin). */
static void ftRemixSheikDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSheikDSPAirToGround);
}

/* SheikDSP.attack_collision_ (ProcMap of DSP attack): wall flag, recoil,
 * landing, then ledge grab. */
static void ftRemixSheikDSPAttackProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->coll_data.mask_prev & (MAP_FLAG_LWALL | MAP_FLAG_RWALL))
    {
        fp->motion_vars.flags.flag0 = 1;
    }
    if (ftRemixSheikDSPCheckRecoil(fighter_gobj))
    {
        return;
    }
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSheikDSPAirToGround);

    if (!mpCommonCheckFighterCeilHeavyCliff(fighter_gobj))
    {
        return;
    }
    if (!(fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK))
    {
        return;
    }
    ftCommonCliffCatchSetStatus(fighter_gobj);
}

/* SheikDSP.recoil_main_ (ProcUpdate of DSP recoil): falls after frame 40 and
 * re-attacks between frames 15 and 40 when B is pressed, flipping facing. */
static void ftRemixSheikDSPRecoilProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 buttons = ftRemixSheikDSPUpdateButtonBuffer(fp);

    ftAnimEndCheckSetStatus(fighter_gobj, ftCommonFallSetStatus);

    if (fighter_gobj->anim_frame > SHEIK_DSP_RECOIL_FRAME_MAX)
    {
        return;
    }
    if (fighter_gobj->anim_frame <= SHEIK_DSP_RECOIL_FRAME_MIN)
    {
        return;
    }
    if (!(buttons & B_BUTTON))
    {
        return;
    }
    fp->lr = (fp->lr == 1) ? -1 : 1;
    ftRemixSheikDSPAttackTransition(fighter_gobj);
}

/* SheikDSP.recoil_physics_ (ProcPhysics of DSP recoil). The asm does not
 * return early after the vertical part, so the horizontal part always runs. */
static void ftRemixSheikDSPRecoilProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSheikDSPPhysicsVertical(fp);
    ftRemixSheikDSPRecoilPhysicsHorizontal(fp);
}

/* Remix address -> port routine for every Sheik DSP routine in the worklist. */
const FTRemixProc dFTRemixSheikDSPProcs[] =
{
    { REMIX_SYM_SheikDSP__initial_, ftRemixSheikDSPInitial },
    { REMIX_SYM_SheikDSP__main_, ftRemixSheikDSPProcUpdate },
    { REMIX_SYM_SheikDSP__physics_, ftRemixSheikDSPProcPhysics },
    { REMIX_SYM_SheikDSP__air_collision_, ftRemixSheikDSPAirProcMap },
    { REMIX_SYM_SheikDSP__attack_collision_, ftRemixSheikDSPAttackProcMap },
    { REMIX_SYM_SheikDSP__recoil_main_, ftRemixSheikDSPRecoilProcUpdate },
    { REMIX_SYM_SheikDSP__recoil_physics_, ftRemixSheikDSPRecoilProcPhysics }
};

const s32 dFTRemixSheikDSPProcsNum = ARRAY_COUNT(dFTRemixSheikDSPProcs);

#endif
