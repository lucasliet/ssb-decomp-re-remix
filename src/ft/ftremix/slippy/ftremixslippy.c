/**
 * ftremixslippy.c - C ports of Smash Remix Slippy routines
 * (.toolchain/pkgs/smashremix-src/src/Slippy/SlippySpecial.asm).
 *
 * Neutral special: temp variable 1 (motion_vars flag0, set by command 5C0000XX)
 * requests one laser, which is the vanilla Fox blaster descriptor with Slippy's
 * file 6. Up special and down special keep their frame counters in the Fox
 * status_vars words 0x0B20..0x0B28, read through ftremixfields.h.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftchar/ftfox/ftfoxfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpfox/wpfoxblaster.h>
#include <wp/wpmanager.h>

enum
{
    nFTRemixSlippyStatusNSPGround = 0xE1,
    /* Kirby new action 100 (SLIPPY_NSP_Ground): 0x12F + 100 in Kirby.asm add_new_action order. */
    nFTRemixSlippyKirbyStatusNSPGround = 0x193
};

/* N64 FTStruct 0x092C is joints[17] (joints at 0x08E8, 4 bytes each). */
#define SLIPPY_NSP_LASER_SPAWN_JOINT 17
#define SLIPPY_NSP_LASER_SPEED 300.0F               /* 0x43960000 */
#define SLIPPY_NSP_SPAWN_OFFSET_Y 32.0F             /* 0x42000000 */
#define SLIPPY_NSP_SPAWN_OFFSET_Z 340.0F            /* 0x43AA0000 */

#define SLIPPY_USP_MOVE_START_FRAME 2
#define SLIPPY_USP_BASE_SPEED 20.0F                 /* 0x41A00000 */
#define SLIPPY_USP_DECELERATION_FACTOR 24.0F        /* 0x41C00000 */
#define SLIPPY_USP_TURN_SPEED 0.0523599F            /* 0x3D567756 */
#define SLIPPY_USP_STICK_DEADZONE 10.0F             /* 0x41200000 */
#define SLIPPY_USP_TWO_PI 6.28319F                  /* 0x40C90FE4 */
#define SLIPPY_USP_NEG_PI -3.14159F                 /* 0xC0490FD0 */

/* Fox's down special air physics with Slippy's gravity (vanilla Fox uses 0.8F). */
#define SLIPPY_DSP_AIR_GRAVITY 2.5F                 /* 0x40200000 */

/* Remix scratch words in FTStruct status_vars (N64 offsets). */
#define SLIPPY_WORD_USP_ANGLE 0x0B20
#define SLIPPY_WORD_USP_DURATION 0x0B24
#define SLIPPY_WORD_TIMER 0x0B28                    /* USP frame counter, DSP gravity delay */

/* SlippyNSP.laser_stage_setting_: fills the blaster descriptor with Slippy's file 6 and the vanilla Fox blaster procs. */
static void ftRemixSlippyNSPFillLaserDesc(WPDesc *laser_desc, FTStruct *fp)
{
    laser_desc->flags = 0;
    laser_desc->kind = nWPKindBlaster;
    laser_desc->p_weapon = fp->data->p_file_special1;
    laser_desc->o_attributes = 0;
    laser_desc->transform_types.tk1 = nGCMatrixKindTraRotRpyRSca;
    laser_desc->transform_types.tk2 = nGCMatrixKindNull;
    laser_desc->transform_types.tk3 = 0;
    laser_desc->proc_update = wpFoxBlasterProcUpdate;
    laser_desc->proc_map = wpFoxBlasterProcMap;
    laser_desc->proc_hit = wpFoxBlasterProcHit;
    laser_desc->proc_shield = wpFoxBlasterProcHit;
    laser_desc->proc_hop = wpFoxBlasterProcHop;
    laser_desc->proc_setoff = wpFoxBlasterProcHit;
    laser_desc->proc_reflector = wpFoxBlasterProcReflector;
    laser_desc->proc_absorb = wpFoxBlasterProcHit;
}

/* SlippyNSP.laser_stage_setting_: spawns one laser at `spawn_pos`, launched at SLIPPY_NSP_LASER_SPEED in the facing direction. */
static void ftRemixSlippyNSPCreateLaser(GObj *fighter_gobj, FTStruct *fp, Vec3f *spawn_pos)
{
    WPDesc laser_desc;
    GObj *laser_gobj;
    WPStruct *wp;

    ftRemixSlippyNSPFillLaserDesc(&laser_desc, fp);
    laser_gobj = wpManagerMakeWeapon(fighter_gobj, &laser_desc, spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (laser_gobj == NULL)
    {
        return;
    }
    wp = wpGetStruct(laser_gobj);
    wp->physics.vel_air.x = (f32) wp->lr * SLIPPY_NSP_LASER_SPEED;
    DObjGetStruct(laser_gobj)->rotate.vec.f.z = syUtilsArcTan2(wp->physics.vel_air.y, wp->physics.vel_air.x);
    efManagerFoxBlasterGlowMakeEffect(spawn_pos);
}

/* SlippyNSP.air_to_ground_: landing transition; Kirby and J Kirby copy the ground move under their own action id. */
static void ftRemixSlippyNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action_id = nFTRemixSlippyStatusNSPGround;

    mpCommonSetFighterGround(fp);
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        action_id = nFTRemixSlippyKirbyStatusNSPGround;
    }
    ftMainSetStatus(fighter_gobj, action_id, fighter_gobj->anim_frame, 1.0F, 0);
}

/* SlippyNSP.main_ (ProcUpdate of the neutral special). */
static void ftRemixSlippyNSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f spawn_pos = { 0.0F, SLIPPY_NSP_SPAWN_OFFSET_Y, SLIPPY_NSP_SPAWN_OFFSET_Z };

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        gmCollisionGetFighterPartsWorldPosition(fp->joints[SLIPPY_NSP_LASER_SPAWN_JOINT], &spawn_pos);
        ftRemixSlippyNSPCreateLaser(fighter_gobj, fp, &spawn_pos);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* SlippyNSP.air_collision_ (ProcMap of the aerial neutral special). */
static void ftRemixSlippyNSPProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSlippyNSPAirToGround);
}

/* Turns from `prev_angle` toward `stick_angle` by at most SLIPPY_USP_TURN_SPEED per frame. */
static f32 ftRemixSlippyUSPTurnAngle(f32 prev_angle, f32 stick_angle)
{
    f32 diff = stick_angle - prev_angle;
    f32 angle;

    if (!(SLIPPY_USP_NEG_PI < diff))
    {
        diff += SLIPPY_USP_TWO_PI;
    }
    if (ABSF(diff) <= SLIPPY_USP_TURN_SPEED)
    {
        angle = stick_angle;
    }
    else
    {
        angle = prev_angle + ((diff > 0.0F) ? SLIPPY_USP_TURN_SPEED : -SLIPPY_USP_TURN_SPEED);
    }
    if (!(SLIPPY_USP_NEG_PI < angle))
    {
        angle += SLIPPY_USP_TWO_PI;
    }
    return angle;
}

/* Movement angle for this frame: unchanged below the stick deadzone, otherwise a turn toward the stick. */
static f32 ftRemixSlippyUSPNextAngle(FTStruct *fp)
{
    f32 prev_angle = *ftRemixFighterFloat(fp, SLIPPY_WORD_USP_ANGLE);
    f32 stick_x = (f32) (fp->input.pl.stick_range.x * fp->lr);
    f32 stick_y = (f32) fp->input.pl.stick_range.y;
    f32 magnitude = sqrtf((stick_y * stick_y) + (stick_x * stick_x));

    if (magnitude < SLIPPY_USP_STICK_DEADZONE)
    {
        return prev_angle;
    }
    return ftRemixSlippyUSPTurnAngle(prev_angle, syUtilsArcTan2(stick_y, stick_x));
}

/* Speed for this frame: BASE_SPEED scaled up while the duration counts down. */
static f32 ftRemixSlippyUSPSpeed(FTStruct *fp)
{
    f32 frames_left = (f32) *ftRemixFighterWord(fp, SLIPPY_WORD_USP_DURATION);
    f32 multiplier = 1.0F + ((frames_left + 1.0F) / SLIPPY_USP_DECELERATION_FACTOR);

    return SLIPPY_USP_BASE_SPEED * multiplier;
}

/* Stores the new angle and sets the air velocity from it, speed and facing. */
static void ftRemixSlippyUSPApplyMovement(FTStruct *fp)
{
    f32 angle = ftRemixSlippyUSPNextAngle(fp);
    f32 speed;

    *ftRemixFighterFloat(fp, SLIPPY_WORD_USP_ANGLE) = angle;
    speed = ftRemixSlippyUSPSpeed(fp);
    fp->physics.vel_air.x = (speed * __cosf(angle)) * (f32) fp->lr;
    fp->physics.vel_air.y = speed * __sinf(angle);
}

/* SlippyUSP.movement_physics_ (ProcPhysics of the up special). */
static void ftRemixSlippyUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *frame_counter = ftRemixFighterWord(fp, SLIPPY_WORD_TIMER);

    *frame_counter += 1;
    if (*frame_counter >= SLIPPY_USP_MOVE_START_FRAME)
    {
        ftRemixSlippyUSPApplyMovement(fp);
    }
    ftFoxSpecialHiUpdateModelPitch(fighter_gobj);
}

/* SlippyDSP.air_physics_ (ProcPhysics of the down special): Fox's aerial down special physics with SLIPPY_DSP_AIR_GRAVITY. */
static void ftRemixSlippyDSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;
    s32 *gravity_delay = ftRemixFighterWord(fp, SLIPPY_WORD_TIMER);

    if (*gravity_delay != 0)
    {
        *gravity_delay -= 1;
    }
    else ftPhysicsApplyGravityClampTVel(fp, SLIPPY_DSP_AIR_GRAVITY, attr->tvel_base);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftPhysicsApplyAirVelXFriction(fp, attr);
    }
}

/* Remix address -> port routine for every Slippy routine in the worklist (43_SLIPPY). */
const FTRemixProc dFTRemixSlippyProcs[] =
{
    { REMIX_SYM_SlippyNSP__main_, ftRemixSlippyNSPProcUpdate },
    { REMIX_SYM_SlippyNSP__air_collision_, ftRemixSlippyNSPProcMap },
    { REMIX_SYM_SlippyUSP__movement_physics_, ftRemixSlippyUSPProcPhysics },
    { REMIX_SYM_SlippyDSP__air_physics_, ftRemixSlippyDSPProcPhysics }
};

const s32 dFTRemixSlippyProcsNum = ARRAY_COUNT(dFTRemixSlippyProcs);

#endif
