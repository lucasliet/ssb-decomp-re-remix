/**
 * ftremixgoemonusp.c - C ports of Smash Remix Goemon up special routines
 * (.toolchain/pkgs/smashremix-src/src/Goemon/GoemonSpecial.asm, scope GoemonUSP).
 *
 * The Magic Cloud is the custom Remix item Item.Cloud (it/itremix/itremixcloud.c).
 * See docs/remix/deviations.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include <sys/utils.h>
#include "remix/remix_symbols.h"
#include <ft/ftchar/ftmario/ftmario.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftdef.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <it/item.h>
#include <it/itdisplay.h>
#include <it/itmain.h>
#include <it/itremix/itremixcloud.h>
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixGoemonStatusUSP = 0xDF,
    nFTRemixGoemonStatusUSPAttack = 0xE0,
    nFTRemixGoemonStatusUSPJump = 0xE1,
    nFTRemixGoemonStatusUSPEscape = 0xE2
};

#define GOEMON_USP_GROUND_SPEED 80.0F                 /* 0x42A00000 */
#define GOEMON_USP_JUMP_SPEED 64.0F                   /* 0x42800000 */
#define GOEMON_USP_ESCAPE_SPEED 14.0F                 /* 0x41600000 */
#define GOEMON_USP_MIN_SPEED 3.0F                     /* 0x40400000 */
#define GOEMON_USP_ACCELERATION 2.0F                  /* 0x40000000 */
#define GOEMON_USP_LANDING_FSM 0.25F                  /* 0x3E800000 */
#define GOEMON_USP_LANDING_LAG 0.2507031261920929F    /* 0x3E805C29: Mario's copied landing lag with Goemon's high half */
#define GOEMON_USP_FALL_DRIFT 0.9375F                 /* 0x3F700000 */
#define GOEMON_USP_AIR_INITIAL_Y_SCALE 0.375F         /* 0x3EC00000 */
#define GOEMON_USP_STICK_SCALE 0.53125F               /* 0x3F080000 */
#define GOEMON_USP_MAX_TARGET_SPEED 42.5F             /* 0x422A0000: 80 * 0.53125 */
#define GOEMON_USP_STICK_DEADZONE 11
#define GOEMON_USP_MAX_TIME 80
#define GOEMON_USP_TURN_STICK_THRESHOLD (-39)
#define GOEMON_USP_SPAWN_SFX 0x500
#define GOEMON_USP_FAKE_ITEM_KIND 0x1C                /* suppresses the destroyed item's smoke graphic */

/* The USP keeps its frame timer, attached cloud and charge level in FTStruct slots that the decomp names for
 * other fighters' specials. These macros name the N64 slots the asm reads and writes. */
#define GOEMON_USP_FRAME_TIMER(fp) ((fp)->status_vars.common.dead.wait)                  /* N64 0x0B18 */
#define GOEMON_USP_CLOUD_GOBJ(fp) ((fp)->status_vars.common.tarucann.tarucann_gobj)      /* N64 0x0B20 */
#define GOEMON_USP_CHARGE_LEVEL(fp) ((fp)->passive_vars.kirby.copysamus_charge_recoil)   /* N64 0x0AE4 */

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);
extern alSoundEffect *func_800269C0_275C0(u16 sfx_id);

/* GoemonUSP.destroy_attached_cloud_: destroys the attached cloud without its smoke effect. */
static void ftRemixGoemonUSPDestroyAttachedCloud(FTStruct *fp)
{
    GObj *cloud_gobj = GOEMON_USP_CLOUD_GOBJ(fp);
    ITStruct *cloud_ip;

    if (cloud_gobj == NULL)
    {
        return;
    }
    GOEMON_USP_CLOUD_GOBJ(fp) = NULL;
    cloud_ip = itGetStruct(cloud_gobj);
    cloud_ip->kind = GOEMON_USP_FAKE_ITEM_KIND;
    itMainDestroyItem(cloud_gobj);
}

/* GoemonUSP.goemon_on_hit_subroutine_: runs when Goemon takes damage during the USP. */
static void ftRemixGoemonUSPOnHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixGoemonUSPDestroyAttachedCloud(fp);
    GOEMON_USP_CHARGE_LEVEL(fp) = 0;
}

/* Size.asm item render_routine_ (ECB part): the cloud map collision box is its attribute box times the owner multiplier. */
static void ftRemixGoemonUSPCloudApplySize(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITAttributes *attr = ip->attr;
    f32 multiplier = ftRemixSizeGetMultiplier(ftGetStruct(ip->owner_gobj)->player);

    ip->coll_data.map_coll.top = attr->map_coll_top * multiplier;
    ip->coll_data.map_coll.center = attr->map_coll_center * multiplier;
    ip->coll_data.map_coll.bottom = attr->map_coll_bottom * multiplier;
    ip->coll_data.map_coll.width = attr->map_coll_width * multiplier;
}

/* Size.asm item render_routine_: draws the cloud with the vanilla render, then applies the size to its ECB. */
static void ftRemixGoemonUSPCloudProcDisplay(GObj *item_gobj)
{
    itDisplayColAnimOPAProcDisplay(item_gobj);
    ftRemixGoemonUSPCloudApplySize(item_gobj);
}

/* GoemonUSP.attach_cloud_: installs the on-hit routine and attaches the Magic Cloud. */
static void ftRemixGoemonUSPAttachCloud(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *cloud_gobj;

    fp->proc_damage = ftRemixGoemonUSPOnHit;
    ftSamusSpecialNGetChargeShotPosition(fp, &pos);
    cloud_gobj = itRemixCloudMakeItem(fighter_gobj, &pos, &vel, 0);
    if (cloud_gobj != NULL)
    {
        cloud_gobj->proc_display = ftRemixGoemonUSPCloudProcDisplay;
    }
    GOEMON_USP_CLOUD_GOBJ(fp) = cloud_gobj;
}

/* GoemonUSP.goemon_on_hit_subroutine_establishment_: sets the on-hit routine, clears the attached cloud and temp variable 1. */
static void ftRemixGoemonUSPOnHitEstablishment(FTStruct *fp)
{
    fp->proc_damage = ftRemixGoemonUSPOnHit;
    GOEMON_USP_CLOUD_GOBJ(fp) = NULL;
    fp->motion_vars.flags.flag0 = 0;
}

/* Clears the USP's frame timer, temp variables, absorb and fast fall bits, and refills the jumps. */
static void ftRemixGoemonUSPInitStatusVars(FTStruct *fp)
{
    GOEMON_USP_FRAME_TIMER(fp) = 0;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 1;
    fp->motion_vars.flags.flag2 = 0;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
    fp->jumps_used = fp->attr->jumps_max;
}

/* GoemonUSP.air_initial_ */
static void ftRemixGoemonUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 vel_y;

    ftMainSetStatus(fighter_gobj, nFTRemixGoemonStatusUSP, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixGoemonUSPAttachCloud(fighter_gobj);
    ftRemixGoemonUSPOnHitEstablishment(fp);

    vel_y = fp->physics.vel_air.y * GOEMON_USP_AIR_INITIAL_Y_SCALE;
    ftRemixGoemonUSPInitStatusVars(fp);
    fp->physics.vel_air.y = vel_y;
    func_800269C0_275C0(GOEMON_USP_SPAWN_SFX);
}

/* GoemonUSP.ground_initial_: launches the grounded start upward, then runs the air initial. */
static void ftRemixGoemonUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->physics.vel_air.y = GOEMON_USP_GROUND_SPEED;
    mpCommonSetFighterAir(fp);
    ftRemixGoemonUSPAirInitial(fighter_gobj);
}

/* GoemonUSP.shared_initial_: changes to another USP action and marks temp variable 2. */
static void ftRemixGoemonUSPSharedInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag1 = 1;
}

/* GoemonUSP.check_turn_: turns Goemon around when the stick points against the facing direction. */
static void ftRemixGoemonUSPCheckTurn(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 stick_toward_facing = fp->input.pl.stick_range.x * fp->lr;

    if (stick_toward_facing < GOEMON_USP_TURN_STICK_THRESHOLD)
    {
        fp->lr = -fp->lr;
    }
}

/* Begins the special fall that the USP uses when its animation ends. */
static void ftRemixGoemonUSPBeginFall(GObj *fighter_gobj)
{
    ftCommonFallSpecialSetStatus(fighter_gobj, GOEMON_USP_FALL_DRIFT, FALSE, TRUE, FALSE, GOEMON_USP_LANDING_FSM, FALSE);
}

/* Begins the special fall once the animation has ended (anim_frame <= 0). */
static void ftRemixGoemonUSPBeginFallAtAnimEnd(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixGoemonUSPBeginFall(fighter_gobj);
    }
}

/* GoemonUSP.main_: after MAX_TIME frames, turns to the USPJump action. */
static void ftRemixGoemonUSPMainProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 frame_timer = GOEMON_USP_FRAME_TIMER(fp);

    GOEMON_USP_FRAME_TIMER(fp) = frame_timer + 1;
    if (frame_timer < GOEMON_USP_MAX_TIME)
    {
        return;
    }
    ftRemixGoemonUSPCheckTurn(fighter_gobj);
    ftRemixGoemonUSPSharedInitial(fighter_gobj, nFTRemixGoemonStatusUSPJump);
}

/* GoemonUSP.attack_main_: after MAX_TIME frames, begins the special fall; otherwise returns to USP at animation end. */
static void ftRemixGoemonUSPAttackProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 frame_timer = GOEMON_USP_FRAME_TIMER(fp);

    GOEMON_USP_FRAME_TIMER(fp) = frame_timer + 1;
    if (frame_timer >= GOEMON_USP_MAX_TIME)
    {
        ftRemixGoemonUSPBeginFall(fighter_gobj);
        return;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixGoemonUSPSharedInitial(fighter_gobj, nFTRemixGoemonStatusUSP);
    }
}

/* GoemonUSP.jump_main_: applies the jump velocity once temp variable 1 is 1, then falls at animation end. */
static void ftRemixGoemonUSPJumpProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 == 1)
    {
        fp->motion_vars.flags.flag0 = 2;
        fp->physics.vel_air.y = GOEMON_USP_JUMP_SPEED;
    }
    ftRemixGoemonUSPBeginFallAtAnimEnd(fighter_gobj);
}

/* GoemonUSP.escape_main_: begins the special fall at animation end. */
static void ftRemixGoemonUSPEscapeProcUpdate(GObj *fighter_gobj)
{
    ftRemixGoemonUSPBeginFallAtAnimEnd(fighter_gobj);
}

/* GoemonUSP.interrupt_: A or B starts the attack, Shield starts the escape, and a C button starts the jump. */
static void ftRemixGoemonUSPProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 buttons = fp->input.pl.button_tap;

    if (buttons & (A_BUTTON | B_BUTTON))
    {
        ftRemixGoemonUSPCheckTurn(fighter_gobj);
        ftRemixGoemonUSPSharedInitial(fighter_gobj, nFTRemixGoemonStatusUSPAttack);
        return;
    }
    if (fp->input.button_mask_z & buttons)
    {
        fp->physics.vel_air.y = GOEMON_USP_ESCAPE_SPEED;
        ftRemixGoemonUSPSharedInitial(fighter_gobj, nFTRemixGoemonStatusUSPEscape);
        return;
    }
    if (buttons & (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS))
    {
        ftRemixGoemonUSPCheckTurn(fighter_gobj);
        ftRemixGoemonUSPSharedInitial(fighter_gobj, nFTRemixGoemonStatusUSPJump);
    }
}

/* Returns TRUE when both stick axes are inside the deadzone (|x| and |y| < 11). */
static sb32 ftRemixGoemonUSPIsStickNeutral(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    return (ABS(stick_x) < GOEMON_USP_STICK_DEADZONE) && (ABS(stick_y) < GOEMON_USP_STICK_DEADZONE);
}

/* Stops Goemon when its speed is at most MIN_SPEED. Returns TRUE if it stopped. */
static sb32 ftRemixGoemonUSPStopBelowMinSpeed(FTStruct *fp)
{
    f32 vel_x = fp->physics.vel_air.x;
    f32 vel_y = fp->physics.vel_air.y;
    f32 speed = sqrtf(vel_x * vel_x + vel_y * vel_y);

    if (speed > GOEMON_USP_MIN_SPEED)
    {
        return FALSE;
    }
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    return TRUE;
}

/* Target velocity for the stick, scaled down to MAX_TARGET_SPEED when its length exceeds it. */
static Vec2f ftRemixGoemonUSPGetStickTarget(FTStruct *fp)
{
    Vec2f target;
    f32 target_length;

    target.x = (f32) fp->input.pl.stick_range.x * GOEMON_USP_STICK_SCALE;
    target.y = (f32) fp->input.pl.stick_range.y * GOEMON_USP_STICK_SCALE;
    target_length = sqrtf(target.x * target.x + target.y * target.y);
    if (target_length >= GOEMON_USP_MAX_TARGET_SPEED)
    {
        f32 scale = GOEMON_USP_MAX_TARGET_SPEED / target_length;

        target.x *= scale;
        target.y *= scale;
    }
    return target;
}

/* Adds ACCELERATION along the angle from the current air velocity to target. */
static void ftRemixGoemonUSPAccelerateToward(FTStruct *fp, Vec2f target)
{
    f32 diff_x = target.x - fp->physics.vel_air.x;
    f32 diff_y = target.y - fp->physics.vel_air.y;
    f32 angle = syUtilsArcTan2(diff_y, diff_x);

    fp->physics.vel_air.x += GOEMON_USP_ACCELERATION * __cosf(angle);
    fp->physics.vel_air.y += GOEMON_USP_ACCELERATION * __sinf(angle);
}

/* GoemonUSP.physics_: steers toward the stick's target velocity, or toward rest when the stick is neutral. */
static void ftRemixGoemonUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec2f target = { 0.0F, 0.0F };

    if (ftRemixGoemonUSPIsStickNeutral(fp))
    {
        if (ftRemixGoemonUSPStopBelowMinSpeed(fp))
        {
            return;
        }
    }
    else
    {
        target = ftRemixGoemonUSPGetStickTarget(fp);
    }
    ftRemixGoemonUSPAccelerateToward(fp, target);
}

/* GoemonUSP.jump_physics_: uses the general air physics with player control once temp variable 1 is set. */
static void ftRemixGoemonUSPJumpProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        ftPhysicsApplyAirVelDriftFastFall(fighter_gobj);
        return;
    }
    ftRemixGoemonUSPProcPhysics(fighter_gobj);
}

/* GoemonUSP.collision_: Mario's up special landing check (ftMarioSpecialHiProcMap) with Goemon's landing lag. */
static void ftRemixGoemonUSPProcMap(GObj *fighter_gobj)
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
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, GOEMON_USP_LANDING_LAG);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* Remix address -> port routine for every GoemonUSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixGoemonUSPProcs[] =
{
    { REMIX_SYM_GoemonUSP__air_initial_, ftRemixGoemonUSPAirInitial },
    { REMIX_SYM_GoemonUSP__attack_main_, ftRemixGoemonUSPAttackProcUpdate },
    { REMIX_SYM_GoemonUSP__collision_, ftRemixGoemonUSPProcMap },
    { REMIX_SYM_GoemonUSP__escape_main_, ftRemixGoemonUSPEscapeProcUpdate },
    { REMIX_SYM_GoemonUSP__ground_initial_, ftRemixGoemonUSPGroundInitial },
    { REMIX_SYM_GoemonUSP__interrupt_, ftRemixGoemonUSPProcInterrupt },
    { REMIX_SYM_GoemonUSP__jump_main_, ftRemixGoemonUSPJumpProcUpdate },
    { REMIX_SYM_GoemonUSP__jump_physics_, ftRemixGoemonUSPJumpProcPhysics },
    { REMIX_SYM_GoemonUSP__main_, ftRemixGoemonUSPMainProcUpdate },
    { REMIX_SYM_GoemonUSP__physics_, ftRemixGoemonUSPProcPhysics }
};

const s32 dFTRemixGoemonUSPProcsNum = ARRAY_COUNT(dFTRemixGoemonUSPProcs);

#endif
