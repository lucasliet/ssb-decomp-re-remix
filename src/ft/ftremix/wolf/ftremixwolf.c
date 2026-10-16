/**
 * ftremixwolf.c - C ports of Smash Remix Wolf routines
 * (.toolchain/pkgs/smashremix-src/src/Wolf/WolfSpecial.asm).
 *
 * Up special temp variables: flag2 (0x184) is the phase, flag1 (0x180) is the
 * turn request and flag0 (0x17C) is the slash graphics state. The neutral
 * special's blaster is spawned by ftRemixWolfSpecialNCreateBlaster.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>
#include <ft/ftchar/ftfox/ftfox.h>
#include <ft/ftchar/ftfox/ftfoxfunctions.h>
#include <ft/ftchar/ftmario/ftmariofunctions.h>
#include <ef/efmanager.h>
#include <ft/ftparam.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <it/itcommon/itlgun.h>
#include <mp/mpcommon.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpboss/wpbossbullet.h>
#include <wp/wpmain.h>
#include <wp/wpmanager.h>

/* Kirby.Action.WOLF_NSP_Ground (Kirby.asm add_new_action) */
#define nFTRemixWolfStatusKirbyNSPGround 0xEB

#define WOLF_USP_X_SPEED 220.0F                /* 0x435C0000 (the asm comment says 100.0) */
#define WOLF_USP_Y_SPEED 120.0F                /* 0x42F00000 (the asm comment says 40.0) */
#define WOLF_USP_Y_INPUT_SCALE 0.796875F       /* 0x3F4C0000 (the asm comment says 0.5) */
#define WOLF_USP_X_PER_Y_SCALE 0.21875F        /* 0x3E600000 */
#define WOLF_USP_LANDING_LAG 0.25F             /* 0x3E800000 */
#define WOLF_USP_VEL_DECAY 0.875F              /* 0x3F600000 */
#define WOLF_USP_AIR_ACCEL_MOVE 0.0234375F     /* 0x3CC00000 */
#define WOLF_USP_AIR_ACCEL_END_MOVE 0.0078125F /* 0x3C000000 */
#define WOLF_USP_AIR_CONTROL_STICK_MIN 8
#define WOLF_USP_SHORTEN_FRAME 18.0F           /* 0x41900000 */
#define WOLF_USP_TURN_REQUEST 2
#define WOLF_USP_SLASH_STOP 2

enum
{
    nFTRemixWolfStatusNSPGround = 0xE1,
    nFTRemixWolfStatusUSPInitialAir = 0xE3,
    nFTRemixWolfStatusUSPInitialGround = 0xE4,
    nFTRemixWolfStatusUSPPartTwoAir = 0xE6,
    nFTRemixWolfStatusUSPPartTwoGround = 0xE8
};

enum
{
    WOLF_USP_PHASE_BEGIN = 1,
    WOLF_USP_PHASE_BEGIN_MOVE = 2,
    WOLF_USP_PHASE_MOVE = 3,
    WOLF_USP_PHASE_END_MOVE = 4
};

/* WolfUSP.button_press_buffer: last frame's tapped buttons per port. */
static u16 sFTRemixWolfButtonPressBuffer[MAXCONTROLLERS];

/* Returns this frame's or last frame's tapped buttons and records this frame's. */
static u16 ftRemixWolfUpdateButtonBuffer(FTStruct *fp)
{
    u16 buffered = sFTRemixWolfButtonPressBuffer[fp->player];

    sFTRemixWolfButtonPressBuffer[fp->player] = fp->input.pl.button_tap;

    return fp->input.pl.button_tap | buffered;
}

/* WolfNSP.air_to_ground_: lands into the ground blaster, Kirby and J Kirby use their own action. */
static void ftRemixWolfSpecialNAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = nFTRemixWolfStatusNSPGround;

    mpCommonSetFighterGround(fp);

    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        status_id = nFTRemixWolfStatusKirbyNSPGround;
    }
    /* Argument 4 = 1 keeps the hitbox continuing (asm: "continue hitbox"). */
    ftMainSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, 1.0F, 1);
}

/* WolfNSP.air_collision_ (ProcMap of the aerial blaster). */
static void ftRemixWolfSpecialNProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixWolfSpecialNAirToGround);
}

/* Raw kind value 0 as written in the Remix blaster descriptor (asm _blaster_projectile_struct). */
#define WOLF_BLASTER_KIND 0
/* Byte 0x48 of the descriptor's transform word 0x12480000 (asm _blaster_projectile_struct). */
#define WOLF_BLASTER_TRANSFORM_TK2 0x48
/* FTStruct.joints[16] at N64 0x0928, the hand joint the blaster starts from (asm projectile_stage_setting). */
#define WOLF_BLASTER_SPAWN_JOINT 16
#define WOLF_BLASTER_DURATION 100              /* WolfNSP.blaster_fireball_struct 0x0000 */
#define WOLF_BLASTER_MAX_SPEED 200.0F          /* 0x43480000, fireball struct 0x0004 */
#define WOLF_BLASTER_INITIAL_SPEED 22.0F       /* 0x41B00000, fireball struct 0x0008 and 0x0020 */
#define WOLF_BLASTER_ACCEL 1.03125F            /* 0x3F840000 */
#define WOLF_BLASTER_PI_2 1.5707964F           /* 0x3FC90FDB, also the Wolf reflect multiplier */
#define WOLF_BLASTER_NO_REFLECT_MUL 1.0F       /* 0x3F800000 */
#define WOLF_BLASTER_SIZE_SCALE 0.25F          /* 0x3E800000 */
#define WOLF_BLASTER_ANGLE_GROUND 0.0F         /* fireball struct 0x0018 */
#define WOLF_BLASTER_ANGLE_AIR 0.0F            /* fireball struct 0x001C */
#define WOLF_BLASTER_PALETTE_ID 0.0F           /* fireball struct 0x002C */

/* Returns the blaster's speed multiplier. Remix keeps it as a float at N64 WPStruct 0x29C, which is the
 * start of weapon_vars (asm 0x029C). */
static f32 *ftRemixWolfBlasterSpeedMul(WPStruct *wp)
{
    return (f32 *)&wp->weapon_vars;
}

/* Clamps a signed speed to the range [-max_speed, max_speed]. */
static f32 ftRemixWolfBlasterClampSpeed(f32 speed, f32 max_speed)
{
    if (speed > max_speed)
    {
        return max_speed;
    }
    if (speed < -max_speed)
    {
        return -max_speed;
    }
    return speed;
}

/* Sets the blaster's x and y scale from its speed: faster means a longer, thinner blaster. */
static void ftRemixWolfBlasterSetSize(DObj *dobj, f32 speed)
{
    f32 abs_speed = ABSF(speed);
    f32 x_scale = abs_speed + WOLF_BLASTER_INITIAL_SPEED;

    x_scale += WOLF_BLASTER_INITIAL_SPEED;
    x_scale += WOLF_BLASTER_INITIAL_SPEED;
    x_scale *= WOLF_BLASTER_SIZE_SCALE;
    x_scale /= WOLF_BLASTER_INITIAL_SPEED;

    dobj->scale.vec.f.x = x_scale;
    dobj->scale.vec.f.y = 1.0F / ((x_scale * 0.5F) + 0.5F);
}

/* WolfNSP.blaster_duration (proc_update): accelerates the blaster up to its max speed and resizes it. */
static sb32 ftRemixWolfSpecialNBlasterProcUpdate(GObj *blaster_gobj)
{
    WPStruct *wp = wpGetStruct(blaster_gobj);
    f32 max_speed = WOLF_BLASTER_MAX_SPEED * (*ftRemixWolfBlasterSpeedMul(wp));

    wp->physics.vel_air.x = ftRemixWolfBlasterClampSpeed(wp->physics.vel_air.x * WOLF_BLASTER_ACCEL, max_speed);
    ftRemixWolfBlasterSetSize(DObjGetStruct(blaster_gobj), wp->physics.vel_air.x);

    return FALSE;
}

/* WolfNSP.blaster_reflection (proc_reflector): a Wolf reflection speeds the blaster up and turns it around. */
static sb32 ftRemixWolfSpecialNBlasterProcReflector(GObj *blaster_gobj)
{
    WPStruct *wp = wpGetStruct(blaster_gobj);
    FTStruct *fp = ftGetStruct(wp->owner_gobj);
    DObj *dobj = DObjGetStruct(blaster_gobj);
    f32 reflect_mul = WOLF_BLASTER_NO_REFLECT_MUL;

    /* The asm tests a register left over from the caller; the owner (the reflecting fighter after the
     * reflection) is used here instead, see the notes. */
    if (fp->fkind == REMIX_ID_WOLF)
    {
        reflect_mul = WOLF_BLASTER_PI_2;
    }
    *ftRemixWolfBlasterSpeedMul(wp) = reflect_mul;
    wp->lifetime = WOLF_BLASTER_DURATION;
    wp->physics.vel_air.x *= reflect_mul;

    wpMainReflectorSetLR(wp, fp);

    dobj->rotate.vec.f.y = (wp->physics.vel_air.x >= 0.0F) ? WOLF_BLASTER_PI_2 : -WOLF_BLASTER_PI_2;

    return FALSE;
}

/* Fills the blaster's weapon descriptor (asm _blaster_projectile_struct); p_weapon is the file-6 pointer. */
static void ftRemixWolfSpecialNFillBlasterDesc(WPDesc *blaster_desc, FTStruct *fp)
{
    blaster_desc->flags = 0;
    blaster_desc->kind = WOLF_BLASTER_KIND;
    blaster_desc->p_weapon = ftRemixPowerData(fp)->p_file_special1;
    blaster_desc->o_attributes = 0;
    blaster_desc->transform_types.tk1 = nGCMatrixKindTra;
    blaster_desc->transform_types.tk2 = WOLF_BLASTER_TRANSFORM_TK2;
    blaster_desc->transform_types.tk3 = 0;
    blaster_desc->proc_update = ftRemixWolfSpecialNBlasterProcUpdate;
    blaster_desc->proc_map = itLGunWeaponAmmoProcMap;
    blaster_desc->proc_hit = itLGunWeaponAmmoProcHit;
    blaster_desc->proc_shield = itLGunWeaponAmmoProcHit;
    blaster_desc->proc_hop = wpBossBulletProcHop;
    blaster_desc->proc_setoff = itLGunWeaponAmmoProcHit;
    blaster_desc->proc_reflector = ftRemixWolfSpecialNBlasterProcReflector;
    blaster_desc->proc_absorb = itLGunWeaponAmmoProcHit;
}

/* Sets the blaster's launch velocity from its facing and the fighter's air or ground state. The asm copies
 * this from the vanilla Fox blaster (copy_segment 0xE3268, 0x2C). The cos and sin calls are 0x80035CD0 and
 * 0x800303F0. */
static void ftRemixWolfBlasterSetVelocity(WPStruct *wp, FTStruct *fp)
{
    f32 angle = WOLF_BLASTER_ANGLE_GROUND;
    f32 speed = WOLF_BLASTER_INITIAL_SPEED;

    if (fp->ga == nMPKineticsAir)
    {
        angle = WOLF_BLASTER_ANGLE_AIR;
    }
    wp->physics.vel_air.z = 0.0F;
    wp->physics.vel_air.x = __cosf(angle) * speed * (f32) fp->lr;
    wp->physics.vel_air.y = __sinf(angle) * speed;
}

/* Initialises a freshly made blaster: multiplier, lifetime, velocity, palette and model pitch. */
static void ftRemixWolfBlasterInit(GObj *blaster_gobj, FTStruct *fp)
{
    WPStruct *wp = wpGetStruct(blaster_gobj);

    *ftRemixWolfBlasterSpeedMul(wp) = WOLF_BLASTER_NO_REFLECT_MUL;
    wp->lifetime = WOLF_BLASTER_DURATION;
    ftRemixWolfBlasterSetVelocity(wp, fp);
    DObjGetStruct(blaster_gobj)->mobj->palette_id = WOLF_BLASTER_PALETTE_ID;
    wpMainVelSetModelPitch(blaster_gobj);
}

/* WolfNSP.projectile_stage_setting: spawns one blaster at the hand joint. */
static void ftRemixWolfSpecialNCreateBlaster(GObj *fighter_gobj, FTStruct *fp)
{
    WPDesc blaster_desc;
    Vec3f spawn_pos = { 0.0F, 0.0F, 0.0F };
    GObj *blaster_gobj;

    gmCollisionGetFighterPartsWorldPosition(fp->joints[WOLF_BLASTER_SPAWN_JOINT], &spawn_pos);
    ftRemixWolfSpecialNFillBlasterDesc(&blaster_desc, fp);
    blaster_gobj = wpManagerMakeWeapon(fighter_gobj, &blaster_desc, &spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (blaster_gobj == NULL)
    {
        return;
    }
    ftRemixWolfBlasterInit(blaster_gobj, fp);
}

/* WolfNSP.main (ProcUpdate of the neutral special). */
static void ftRemixWolfSpecialNProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixWolfSpecialNCreateBlaster(fighter_gobj, fp);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* WolfUSP.initial_ground / initial_air: shared body, the status id differs. */
static void ftRemixWolfSpecialUInitStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);

    fp = ftGetStruct(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = WOLF_USP_PHASE_BEGIN;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
    fp->physics.vel_air.y = fp->attr->gravity;
}

/* WolfUSP.initial_ground (special entry, grounded start). */
static void ftRemixWolfSpecialUInitialGround(GObj *fighter_gobj)
{
    ftRemixWolfSpecialUInitStatus(fighter_gobj, nFTRemixWolfStatusUSPInitialGround);
}

/* WolfUSP.initial_air (special entry, aerial start). */
static void ftRemixWolfSpecialUInitialAir(GObj *fighter_gobj)
{
    ftRemixWolfSpecialUInitStatus(fighter_gobj, nFTRemixWolfStatusUSPInitialAir);
}

/* WolfUSP.usp_2_transition_ground / usp_2_transition_air: enters part two of the special. */
static void ftRemixWolfSpecialUTransition(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 3);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftFoxSpecialHiHoldInitStatusVars(fighter_gobj);
}

static void ftRemixWolfSpecialUTransitionGround(GObj *fighter_gobj)
{
    ftRemixWolfSpecialUTransition(fighter_gobj, nFTRemixWolfStatusUSPPartTwoGround);
}

static void ftRemixWolfSpecialUTransitionAir(GObj *fighter_gobj)
{
    ftRemixWolfSpecialUTransition(fighter_gobj, nFTRemixWolfStatusUSPPartTwoAir);
}

/* WolfUSP.main_ground / main_air shared body: shortens into part two on anim end,
 * or on a B press after frame 18. */
static void ftRemixWolfSpecialUInterrupt(GObj *fighter_gobj, void (*proc_transition)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 buttons = ftRemixWolfUpdateButtonBuffer(fp);

    ftAnimEndCheckSetStatus(fighter_gobj, proc_transition);

    if (fighter_gobj->anim_frame <= WOLF_USP_SHORTEN_FRAME)
    {
        return;
    }
    if (!(buttons & B_BUTTON))
    {
        return;
    }
    proc_transition(fighter_gobj);
}

/* WolfUSP.main_ground (ProcInterrupt, grounded). */
static void ftRemixWolfSpecialUProcInterruptGround(GObj *fighter_gobj)
{
    ftRemixWolfSpecialUInterrupt(fighter_gobj, ftRemixWolfSpecialUTransitionGround);
}

/* WolfUSP.main_air (ProcInterrupt, aerial). */
static void ftRemixWolfSpecialUProcInterruptAir(GObj *fighter_gobj)
{
    ftRemixWolfSpecialUInterrupt(fighter_gobj, ftRemixWolfSpecialUTransitionAir);
}

/* WolfUSP.change_direction_: turns the fighter when moveset command 580000XX requests it. */
static void ftRemixWolfSpecialUProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != WOLF_USP_TURN_REQUEST)
    {
        return;
    }
    /* 0x80160370: Captain Falcon's turn subroutine, copied by the Remix code. */
    ftCaptainSpecialHiProcInterrupt(fighter_gobj);
}

/* Air acceleration for the current phase, the attributes value outside move phases. */
static f32 ftRemixWolfSpecialUAirAccel(FTStruct *fp, FTAttributes *attr)
{
    if (fp->motion_vars.flags.flag2 == WOLF_USP_PHASE_MOVE)
    {
        return WOLF_USP_AIR_ACCEL_MOVE;
    }
    if (fp->motion_vars.flags.flag2 == WOLF_USP_PHASE_END_MOVE)
    {
        return WOLF_USP_AIR_ACCEL_END_MOVE;
    }
    return attr->air_accel;
}

/* WolfUSP air control: stick drift during the move, a smaller one during end move. */
static void ftRemixWolfSpecialUAirControl(FTStruct *fp, FTAttributes *attr)
{
    if (fp->motion_vars.flags.flag2 == WOLF_USP_PHASE_BEGIN)
    {
        /* 0x800D8FA8: the asm uses the air clamp check itself here, which disallows control. */
        ftPhysicsCheckClampAirVelXDecMax(fp, attr);
        return;
    }
    ftPhysicsClampAirVelXStickRange(fp, WOLF_USP_AIR_CONTROL_STICK_MIN, ftRemixWolfSpecialUAirAccel(fp, attr),
                                    attr->air_speed_max_x);
}

/* Sets the launch velocity from the stick's y and moves into the move phase. */
static void ftRemixWolfSpecialUPhaseBeginMove(FTStruct *fp)
{
    f32 x_speed = WOLF_USP_X_SPEED;
    f32 y_speed = WOLF_USP_Y_SPEED + (fp->input.pl.stick_range.y * WOLF_USP_Y_INPUT_SCALE);

    x_speed -= WOLF_USP_X_PER_Y_SCALE * y_speed;

    fp->physics.vel_air.x = fp->lr * x_speed;
    fp->physics.vel_air.y = y_speed;
    fp->motion_vars.flags.flag2 = WOLF_USP_PHASE_MOVE;
    fp->jumps_used = fp->attr->jumps_max;
}

/* Cancels the gravity during the move so the rise holds its height. */
static void ftRemixWolfSpecialUPhaseMove(FTStruct *fp)
{
    fp->physics.vel_air.y += fp->attr->gravity;
}

/* Ends the move, clears the colour overlay and decays the velocity. */
static void ftRemixWolfSpecialUPhaseEndMove(FTStruct *fp)
{
    fp->colanim.is_use_color1 = FALSE;
    fp->physics.vel_air.x *= WOLF_USP_VEL_DECAY;
    fp->physics.vel_air.y *= WOLF_USP_VEL_DECAY;
    /* The asm copies the tail of the vanilla air drift (0x800D90E0), uncertain, see the notes. */
    ftPhysicsApplyAirVelDrift(fp->fighter_gobj);
}

/* Freezes the fighter in place on the first phase frame. */
static void ftRemixWolfSpecialUPhaseBegin(FTStruct *fp)
{
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
}

/* Runs the velocity logic of the current up special phase. */
static void ftRemixWolfSpecialUApplyPhase(FTStruct *fp)
{
    switch (fp->motion_vars.flags.flag2)
    {
        case WOLF_USP_PHASE_BEGIN:
            ftRemixWolfSpecialUPhaseBegin(fp);
            break;
        case WOLF_USP_PHASE_BEGIN_MOVE:
            ftRemixWolfSpecialUPhaseBeginMove(fp);
            break;
        case WOLF_USP_PHASE_MOVE:
            ftRemixWolfSpecialUPhaseMove(fp);
            break;
        case WOLF_USP_PHASE_END_MOVE:
            ftRemixWolfSpecialUPhaseEndMove(fp);
            break;
        default:
            break;
    }
}

/* WolfUSP.physics_ (ProcPhysics). The first vanilla air drift steps (gravity and the
 * air velocity clamp check) run first, air control and friction only when not clamped. */
static void ftRemixWolfSpecialUProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    (fp->is_fastfall) ? ftPhysicsApplyFastFall(fp, attr) : ftPhysicsApplyGravityDefault(fp, attr);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftRemixWolfSpecialUAirControl(fp, attr);
        ftPhysicsApplyAirVelXFriction(fp, attr);
    }
    ftRemixWolfSpecialUApplyPhase(fp);
}

/* WolfUSP.collision_ (ProcMap): Mario's up special collision with Wolf's landing lag. */
static void ftRemixWolfSpecialUProcMap(GObj *fighter_gobj)
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
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, WOLF_USP_LANDING_LAG);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* WolfUSP.main_2 slash graphics: starts them once (flag0 = 0), stops them when flag0 = 2.
 * Returns flag0 as it was on entry. */
static u32 ftRemixWolfSpecialUSlashGraphics(FTStruct *fp)
{
    u32 slash_state = fp->motion_vars.flags.flag0;

    if (slash_state == 0)
    {
        /* REMIX-TODO(PokemonAnnouncer): PokemonAnnouncer.slash_announcement_ is not ported. */
        efManagerCaptainFalconPunchMakeEffect(fp->fighter_gobj);
        fp->is_effect_attach = TRUE;
        fp->motion_vars.flags.flag0 = 1;
    }
    else if (slash_state == WOLF_USP_SLASH_STOP)
    {
        ftParamProcStopEffect(fp->fighter_gobj);
    }
    return slash_state;
}

/* WolfUSP.main_2 (ProcUpdate of part two): ends the special into a fall when the animation ends. */
static void ftRemixWolfSpecialUPartTwoProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 slash_state = ftRemixWolfSpecialUSlashGraphics(fp);

    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, WOLF_USP_LANDING_LAG, slash_state != 0);
}

/* WolfDSP.physics_ (ProcPhysics): Fox's down special air physics. The asm skips the
 * gravity call with an unconditional branch, so only the delay and the clamp remain. */
static void ftRemixWolfSpecialDProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    if (fp->status_vars.fox.speciallw.gravity_delay != 0)
    {
        fp->status_vars.fox.speciallw.gravity_delay--;
    }
    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftPhysicsApplyAirVelXFriction(fp, attr);
    }
}

/* Remix address -> port routine for every Wolf routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixWolfProcs[] =
{
    { REMIX_SYM_WolfNSP__main, ftRemixWolfSpecialNProcUpdate },
    { REMIX_SYM_WolfNSP__air_collision_, ftRemixWolfSpecialNProcMap },
    { REMIX_SYM_WolfUSP__initial_ground, ftRemixWolfSpecialUInitialGround },
    { REMIX_SYM_WolfUSP__initial_air, ftRemixWolfSpecialUInitialAir },
    { REMIX_SYM_WolfUSP__main_ground, ftRemixWolfSpecialUProcInterruptGround },
    { REMIX_SYM_WolfUSP__main_air, ftRemixWolfSpecialUProcInterruptAir },
    { REMIX_SYM_WolfUSP__change_direction_, ftRemixWolfSpecialUProcUpdate },
    { REMIX_SYM_WolfUSP__physics_, ftRemixWolfSpecialUProcPhysics },
    { REMIX_SYM_WolfUSP__collision_, ftRemixWolfSpecialUProcMap },
    { REMIX_SYM_WolfUSP__main_2, ftRemixWolfSpecialUPartTwoProcUpdate },
    { REMIX_SYM_WolfDSP__physics_, ftRemixWolfSpecialDProcPhysics }
};

const s32 dFTRemixWolfProcsNum = ARRAY_COUNT(dFTRemixWolfProcs);

#endif
