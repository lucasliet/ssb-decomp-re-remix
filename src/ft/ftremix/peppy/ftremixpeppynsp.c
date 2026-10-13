/**
 * ftremixpeppynsp.c - C ports of Smash Remix Peppy neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Peppy/PeppySpecial.asm, scope PeppyNSP).
 *
 * The move is a Samus-style charge shot. Charge is passive_vars.samus.charge_recoil
 * (N64 0x0AE0), the release bool is status_vars.samus.specialn.is_release (0x0B18),
 * the charge timer is charge_int (0x0B1C) and the spark GObj is charge_gobj (0x0B20).
 * Temp variable 1 (motion_vars.flags.flag0, 0x017C) is set by the moveset to fire.
 * The laser is a weapon made with wpManagerMakeWeapon (0x801655C8).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/effect.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <lb/lbparticle.h>
#include <mp/mpcommon.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpfox/wpfoxblaster.h>
#include <wp/wpmanager.h>
#include <wp/wpmap.h>

/* Peppy.Action (Peppy.asm) and Kirby.Action (Kirby.asm, Character.add_new_action_params
 * slots). The Kirby ids are the remix ROM action array entries for PEPPY_NSP_* (checked
 * against the routine addresses in the built ROM). */
enum
{
    nFTRemixPeppyStatusNSPGroundBegin = 0xE1,
    nFTRemixPeppyStatusNSPGroundCharge = 0xE2,
    nFTRemixPeppyStatusNSPGroundShoot = 0xED,
    nFTRemixPeppyStatusNSPAirBegin = 0xEE,
    nFTRemixPeppyStatusNSPAirCharge = 0xF2,
    nFTRemixPeppyStatusNSPAirShoot = 0xF3
};

enum
{
    nFTRemixPeppyKirbyStatusNSPGroundBegin = 0x195,
    nFTRemixPeppyKirbyStatusNSPGroundCharge = 0x196,
    nFTRemixPeppyKirbyStatusNSPGroundShoot = 0x197,
    nFTRemixPeppyKirbyStatusNSPAirBegin = 0x198,
    nFTRemixPeppyKirbyStatusNSPAirCharge = 0x199,
    nFTRemixPeppyKirbyStatusNSPAirShoot = 0x19A
};

#define PEPPY_NSP_CHARGE_TICS 0x12
#define PEPPY_NSP_CHARGE_MAX 5
#define PEPPY_NSP_CHARGE_GFX_ROUTINE 0x6D
#define PEPPY_NSP_BLAST_GFX_SCRIPT 0x8D
#define PEPPY_NSP_LASER_SPEED 300.0F
#define PEPPY_NSP_LASER_TRANSFORM_TK1 0x1C
#define PEPPY_NSP_LASER_JOINT 17
#define PEPPY_NSP_SHOOT_END_FRAME 32.0F
#define PEPPY_NSP_SHOOT_START_FRAME 24.0F
#define PEPPY_NSP_NO_ESCAPE (-1)
#define PEPPY_NSP_JOYPAD_A 0x8000
#define PEPPY_NSP_JOYPAD_B 0x4000

#define PEPPY_NSP_FLAGS_COLANIM FTSTATUS_PRESERVE_COLANIM
#define PEPPY_NSP_FLAGS_TRANSITION (FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX)

/* Remix laser spawn offsets (asm 0x43000000, 0x42500000 for Peppy; 0x43200000, 0xC2020000 for Kirby). */
#define PEPPY_NSP_LASER_X_PEPPY 128.0F
#define PEPPY_NSP_LASER_Y_PEPPY 52.0F
#define PEPPY_NSP_LASER_X_KIRBY 160.0F
#define PEPPY_NSP_LASER_Y_KIRBY -32.5F

static sb32 ftRemixPeppyNSPIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Picks Kirby's action id for Kirby and J Kirby, Peppy's otherwise. */
static s32 ftRemixPeppyNSPSelectStatus(FTStruct *fp, s32 peppy_status, s32 kirby_status)
{
    if (ftRemixPeppyNSPIsKirby(fp))
    {
        return kirby_status;
    }
    return peppy_status;
}

/* Stores the on-hit handler the asm writes to 0x09EC (Kirby's copy handler or Samus's). */
static void ftRemixPeppyNSPSetProcDamage(FTStruct *fp)
{
    if (ftRemixPeppyNSPIsKirby(fp))
    {
        fp->proc_damage = ftKirbyCopySamusSpecialNProcDamage;
        return;
    }
    fp->proc_damage = ftSamusSpecialNProcDamage;
}

/* Every Peppy NSP status change uses animation speed 1.0 (asm a3 = 0x3F80). */
static void ftRemixPeppyNSPSetStatus(GObj *fighter_gobj, s32 status_id, f32 frame_begin, u32 flags)
{
    ftMainSetStatus(fighter_gobj, status_id, frame_begin, 1.0F, flags);
}

/* Starts a status at the current animation frame (asm a2 = 0x0078(a0)). */
static void ftRemixPeppyNSPSetStatusFromCurrentFrame(GObj *fighter_gobj, s32 status_id, u32 flags)
{
    ftRemixPeppyNSPSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, flags);
}

/* Button tap test with the fighter's A or B mask (asm 0x1B4 and 0x1B6). */
static sb32 ftRemixPeppyNSPMaskButtonPressed(FTStruct *fp)
{
    return ((fp->input.pl.button_tap & fp->input.button_mask_b) != 0) ||
           ((fp->input.pl.button_tap & fp->input.button_mask_a) != 0);
}

/* Aerial A or B test with the hardware Joypad masks (asm andi Joypad.A and Joypad.B). */
static sb32 ftRemixPeppyNSPJoypadButtonPressed(FTStruct *fp)
{
    return ((fp->input.pl.button_tap & PEPPY_NSP_JOYPAD_B) != 0) ||
           ((fp->input.pl.button_tap & PEPPY_NSP_JOYPAD_A) != 0);
}

/* Shield (Z) tap test (asm 0x1B8 and 0x1BE). */
static sb32 ftRemixPeppyNSPShieldPressed(FTStruct *fp)
{
    return (fp->input.pl.button_tap & fp->input.button_mask_z) != 0;
}

/* Begin: sets the release bool (shoot) when A or B is tapped during the begin action. */
static void ftRemixPeppyNSPBeginCheckRelease(FTStruct *fp)
{
    if (ftRemixPeppyNSPMaskButtonPressed(fp))
    {
        fp->status_vars.samus.specialn.is_release = TRUE;
    }
}

/* Shared entry for the grounded and aerial begin (asm begin_initial_). */
static void ftRemixPeppyNSPBeginInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp;

    ftRemixPeppyNSPSetStatus(fighter_gobj, status_id, 0.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp = ftGetStruct(fighter_gobj);
    if (ftRemixPeppyNSPIsKirby(fp))
    {
        ftKirbyCopySamusSpecialNInitStatusVars(fp);
    }
    else
    {
        ftSamusSpecialNStartInitStatusVars(fp);
    }

    if (fp->passive_vars.samus.charge_recoil != PEPPY_NSP_CHARGE_MAX)
    {
        fp->status_vars.samus.specialn.is_release = FALSE;
        return;
    }
    fp->passive_vars.samus.charge_recoil = PEPPY_NSP_CHARGE_MAX;
    fp->status_vars.samus.specialn.is_release = TRUE;
}

/* PeppyNSP.ground_begin_initial_: special entry for a grounded neutral special. */
static void ftRemixPeppyNSPGroundBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeppyNSPBeginInitial(fighter_gobj, ftRemixPeppyNSPSelectStatus(fp,
        nFTRemixPeppyStatusNSPGroundBegin, nFTRemixPeppyKirbyStatusNSPGroundBegin));
}

/* PeppyNSP.air_begin_initial_: special entry for an aerial neutral special. */
static void ftRemixPeppyNSPAirBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeppyNSPBeginInitial(fighter_gobj, ftRemixPeppyNSPSelectStatus(fp,
        nFTRemixPeppyStatusNSPAirBegin, nFTRemixPeppyKirbyStatusNSPAirBegin));
}

/* Charge entry shared by the grounded and aerial charge: resets the charge timer. */
static void ftRemixPeppyNSPChargeInitial(GObj *fighter_gobj, s32 peppy_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->status_vars.samus.specialn.charge_int = PEPPY_NSP_CHARGE_TICS;
    ftRemixPeppyNSPSetStatus(fighter_gobj, ftRemixPeppyNSPSelectStatus(fp, peppy_status, kirby_status),
        0.0F, PEPPY_NSP_FLAGS_COLANIM);
}

/* PeppyNSP.ground_charge_initial_ */
static void ftRemixPeppyNSPGroundChargeInitial(GObj *fighter_gobj)
{
    ftRemixPeppyNSPChargeInitial(fighter_gobj, nFTRemixPeppyStatusNSPGroundCharge,
        nFTRemixPeppyKirbyStatusNSPGroundCharge);
}

/* PeppyNSP.air_charge_initial_ */
static void ftRemixPeppyNSPAirChargeInitial(GObj *fighter_gobj)
{
    ftRemixPeppyNSPChargeInitial(fighter_gobj, nFTRemixPeppyStatusNSPAirCharge,
        nFTRemixPeppyKirbyStatusNSPAirCharge);
}

/* Shoot entry shared by the grounded and aerial shoot (asm copy of 0x8015DA60 and 0x8015DAA8 frames). */
static void ftRemixPeppyNSPShootInitial(GObj *fighter_gobj, s32 peppy_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;
    if (fp->passive_vars.samus.charge_recoil == 0)
    {
        fp->passive_vars.samus.charge_recoil = 1;
    }
    ftRemixPeppyNSPSetStatus(fighter_gobj, ftRemixPeppyNSPSelectStatus(fp, peppy_status, kirby_status),
        0.0F, PEPPY_NSP_FLAGS_COLANIM);
    ftRemixPeppyNSPSetProcDamage(fp);
}

/* PeppyNSP.ground_shoot_initial_ */
static void ftRemixPeppyNSPGroundShootInitial(GObj *fighter_gobj)
{
    ftRemixPeppyNSPShootInitial(fighter_gobj, nFTRemixPeppyStatusNSPGroundShoot,
        nFTRemixPeppyKirbyStatusNSPGroundShoot);
}

/* PeppyNSP.air_shoot_initial_: lands the aerial shot on the ground-facing aerial state when grounded. */
static void ftRemixPeppyNSPAirShootInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
        ftPhysicsClampAirVelXMax(fp);
    }
    ftRemixPeppyNSPShootInitial(fighter_gobj, nFTRemixPeppyStatusNSPAirShoot,
        nFTRemixPeppyKirbyStatusNSPAirShoot);
}

/* Dispatches a begin that has started: shoot when released, charge otherwise, by ground or air. */
static void ftRemixPeppyNSPBeginDispatch(GObj *fighter_gobj, FTStruct *fp)
{
    sb32 is_release = fp->status_vars.samus.specialn.is_release;

    if (fp->ga == nMPKineticsGround)
    {
        if (is_release)
        {
            ftRemixPeppyNSPGroundShootInitial(fighter_gobj);
            return;
        }
        ftRemixPeppyNSPGroundChargeInitial(fighter_gobj);
        return;
    }
    if (is_release)
    {
        ftRemixPeppyNSPAirShootInitial(fighter_gobj);
        return;
    }
    ftRemixPeppyNSPAirChargeInitial(fighter_gobj);
}

/* PeppyNSP.begin_main_ (ProcUpdate of NSP_Ground_Begin and NSP_Air_Begin). */
static void ftRemixPeppyNSPBeginMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->input.pl.button_tap & PEPPY_NSP_JOYPAD_B) && (fp->passive_vars.samus.charge_recoil != 0))
    {
        fp->status_vars.samus.specialn.is_release = TRUE;
    }
    else if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftRemixPeppyNSPBeginDispatch(fighter_gobj, fp);
}

/* PeppyNSP.ground_begin_interrupt_ */
static void ftRemixPeppyNSPGroundBeginInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeppyNSPBeginCheckRelease(fp);
    if (ftRemixPeppyNSPShieldPressed(fp))
    {
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

/* PeppyNSP.air_begin_interrupt_ */
static void ftRemixPeppyNSPAirBeginInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeppyNSPBeginCheckRelease(fp);
    if (ftRemixPeppyNSPShieldPressed(fp))
    {
        ftCommonFallSetStatus(fighter_gobj);
    }
}

/* Shared tail of the collision transitions: starts the status at the current frame and sets the on-hit handler. */
static void ftRemixPeppyNSPTransitionStatus(GObj *fighter_gobj, s32 peppy_status, s32 kirby_status, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeppyNSPSetStatusFromCurrentFrame(fighter_gobj, ftRemixPeppyNSPSelectStatus(fp, peppy_status,
        kirby_status), flags);
    ftRemixPeppyNSPSetProcDamage(fp);
}

/* PeppyNSP.ground_begin_transition_ used by air_begin_collision */
static void ftRemixPeppyNSPGroundBeginTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixPeppyNSPTransitionStatus(fighter_gobj, nFTRemixPeppyStatusNSPGroundBegin,
        nFTRemixPeppyKirbyStatusNSPGroundBegin, PEPPY_NSP_FLAGS_COLANIM);
}

/* PeppyNSP.air_begin_transition_ used by ground_begin_collision */
static void ftRemixPeppyNSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixPeppyNSPTransitionStatus(fighter_gobj, nFTRemixPeppyStatusNSPAirBegin,
        nFTRemixPeppyKirbyStatusNSPAirBegin, PEPPY_NSP_FLAGS_COLANIM);
}

/* PeppyNSP.ground_charge_transition_ used by air_charge_collision */
static void ftRemixPeppyNSPGroundChargeTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixPeppyNSPTransitionStatus(fighter_gobj, nFTRemixPeppyStatusNSPGroundCharge,
        nFTRemixPeppyKirbyStatusNSPGroundCharge, PEPPY_NSP_FLAGS_TRANSITION);
}

/* PeppyNSP.air_charge_transition_ used by ground_charge_collision */
static void ftRemixPeppyNSPAirChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixPeppyNSPTransitionStatus(fighter_gobj, nFTRemixPeppyStatusNSPAirCharge,
        nFTRemixPeppyKirbyStatusNSPAirCharge, PEPPY_NSP_FLAGS_TRANSITION);
}

/* PeppyNSP.ground_shoot_transition_ used by air_shoot_collision */
static void ftRemixPeppyNSPGroundShootTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixPeppyNSPTransitionStatus(fighter_gobj, nFTRemixPeppyStatusNSPGroundShoot,
        nFTRemixPeppyKirbyStatusNSPGroundShoot, PEPPY_NSP_FLAGS_TRANSITION);
}

/* PeppyNSP.air_shoot_transition_ used by ground_shoot_collision */
static void ftRemixPeppyNSPAirShootTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixPeppyNSPTransitionStatus(fighter_gobj, nFTRemixPeppyStatusNSPAirShoot,
        nFTRemixPeppyKirbyStatusNSPAirShoot, PEPPY_NSP_FLAGS_COLANIM);
}

/* PeppyNSP.ground_begin_collision_ */
static void ftRemixPeppyNSPGroundBeginCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixPeppyNSPAirBeginTransition);
}

/* PeppyNSP.air_begin_collision_ */
static void ftRemixPeppyNSPAirBeginCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPeppyNSPGroundBeginTransition);
}

/* PeppyNSP.ground_charge_collision_ */
static void ftRemixPeppyNSPGroundChargeCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixPeppyNSPAirChargeTransition);
}

/* PeppyNSP.air_charge_collision_ */
static void ftRemixPeppyNSPAirChargeCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPeppyNSPGroundChargeTransition);
}

/* PeppyNSP.ground_shoot_collision_ */
static void ftRemixPeppyNSPGroundShootCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixPeppyNSPAirShootTransition);
}

/* PeppyNSP.air_shoot_collision_ */
static void ftRemixPeppyNSPAirShootCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPeppyNSPGroundShootTransition);
}

/* PeppyNSP.kirby_ground_begin_collision_: Kirby's grounded begin slides off a ledge (0x800DDDDC, not 0x800DDE84). */
static void ftRemixPeppyNSPKirbyGroundBeginCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixPeppyNSPAirBeginTransition);
}

/* PeppyNSP.kirby_ground_charge_collision_: Kirby's grounded charge slides off a ledge (0x800DDDDC, not 0x800DDE84). */
static void ftRemixPeppyNSPKirbyGroundChargeCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixPeppyNSPAirChargeTransition);
}

/* PeppyNSP.kirby_ground_shoot_collision_: Kirby's grounded shoot slides off a ledge (0x800DDDDC, not 0x800DDE84). */
static void ftRemixPeppyNSPKirbyGroundShootCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixPeppyNSPAirShootTransition);
}

/* PeppyNSP.ground_charge_interrupt_: shoot on A or B, otherwise escape or wait on shield. */
static void ftRemixPeppyNSPGroundChargeInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 escape_status;

    if (ftRemixPeppyNSPMaskButtonPressed(fp))
    {
        ftRemixPeppyNSPGroundShootInitial(fighter_gobj);
        return;
    }
    escape_status = ftCommonEscapeGetStatus(fp);
    if (escape_status != PEPPY_NSP_NO_ESCAPE)
    {
        /* The asm passes the fighter GObj pointer (a2) as itemthrow tics; vanilla Samus passes 0. */
        ftCommonEscapeSetStatus(fighter_gobj, escape_status, 0);
        return;
    }
    if (ftRemixPeppyNSPShieldPressed(fp))
    {
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

/* PeppyNSP.air_charge_interrupt_: shoot on A or B, otherwise fall on shield. */
static void ftRemixPeppyNSPAirChargeInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixPeppyNSPJoypadButtonPressed(fp))
    {
        ftRemixPeppyNSPAirShootInitial(fighter_gobj);
        return;
    }
    if (ftRemixPeppyNSPShieldPressed(fp))
    {
        ftCommonFallSetStatus(fighter_gobj);
    }
}

/* Creates the orange blast particle at pos (asm create_orange_blast_gfx_). */
static void ftRemixPeppyNSPCreateBlastGfx(Vec3f *pos)
{
    LBParticle *particle = lbParticleMakeCommon(gEFManagerParticleBankID, PEPPY_NSP_BLAST_GFX_SCRIPT);

    if (particle != NULL)
    {
        particle->pos.x = pos->x;
        particle->pos.y = pos->y;
        particle->pos.z = pos->z;
    }
}

/* PeppyNSP.blaster_projectile_collision_check_: map collision blasts and destroys the laser. */
static sb32 ftRemixPeppyNSPLaserProcMap(GObj *weapon_gobj)
{
    if (wpMapTestAllCheckCollEnd(weapon_gobj) == FALSE)
    {
        return FALSE;
    }
    ftRemixPeppyNSPCreateBlastGfx(&DObjGetStruct(weapon_gobj)->translate.vec.f);
    return TRUE;
}

/* PeppyNSP.blaster_projectile_collision: hurtbox, shield, clang and absorb hits destroy the laser. */
static sb32 ftRemixPeppyNSPLaserProcHit(GObj *weapon_gobj)
{
    ftRemixPeppyNSPCreateBlastGfx(&DObjGetStruct(weapon_gobj)->translate.vec.f);
    return TRUE;
}

/* PeppyNSP.blaster_projectile_collision_2: shield edge bounce turns the laser about the contact normal. */
static sb32 ftRemixPeppyNSPLaserProcHop(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    DObj *dobj = DObjGetStruct(weapon_gobj);

    syVectorRotateAbout3D(&wp->physics.vel_air, &wp->shield_collide_dir, wp->shield_collide_angle * 2.0F);
    dobj->rotate.vec.f.z = syUtilsArcTan2(wp->physics.vel_air.y, wp->physics.vel_air.x);
    dobj->scale.vec.f.x = 1.0F;
    ftRemixPeppyNSPCreateBlastGfx(&dobj->translate.vec.f);
    return FALSE;
}

/* Fills the laser weapon descriptor (asm projectile_struct); p_weapon is the file-6 pointer. */
static void ftRemixPeppyNSPFillLaserDesc(WPDesc *laser_desc, FTStruct *fp)
{
    laser_desc->flags = 0;
    laser_desc->kind = nWPKindBlaster;
    laser_desc->p_weapon = fp->data->p_file_special1;
    laser_desc->o_attributes = 0;
    laser_desc->transform_types.tk1 = PEPPY_NSP_LASER_TRANSFORM_TK1;
    laser_desc->transform_types.tk2 = 0;
    laser_desc->transform_types.tk3 = 0;
    laser_desc->proc_update = wpFoxBlasterProcUpdate;
    laser_desc->proc_map = ftRemixPeppyNSPLaserProcMap;
    laser_desc->proc_hit = ftRemixPeppyNSPLaserProcHit;
    laser_desc->proc_shield = ftRemixPeppyNSPLaserProcHit;
    laser_desc->proc_hop = ftRemixPeppyNSPLaserProcHop;
    laser_desc->proc_setoff = ftRemixPeppyNSPLaserProcHit;
    laser_desc->proc_reflector = wpFoxBlasterProcReflector;
    laser_desc->proc_absorb = ftRemixPeppyNSPLaserProcHit;
}

/* PeppyNSP.laser_stage_setting_: spawns the laser at pos, moving at 300 along the fighter's facing. */
static void ftRemixPeppyNSPLaserStageSetting(GObj *fighter_gobj, FTStruct *fp, Vec3f *pos)
{
    WPDesc laser_desc;
    WPStruct *wp;
    GObj *laser_gobj;

    ftRemixPeppyNSPFillLaserDesc(&laser_desc, fp);
    laser_gobj = wpManagerMakeWeapon(fighter_gobj, &laser_desc, pos,
        WEAPON_FLAG_COLLPROJECT | WEAPON_FLAG_PARENT_FIGHTER);
    if (laser_gobj == NULL)
    {
        return;
    }
    wp = wpGetStruct(laser_gobj);
    wp->physics.vel_air.x = (f32) wp->lr * PEPPY_NSP_LASER_SPEED;
    DObjGetStruct(laser_gobj)->rotate.vec.f.z = syUtilsArcTan2(wp->physics.vel_air.y, wp->physics.vel_air.x);
    ftRemixPeppyNSPCreateBlastGfx(pos);
}

/* Laser spawn point: a fixed offset chosen by Peppy or Kirby, then the gun joint's world position. */
static void ftRemixPeppyNSPGetLaserPosition(FTStruct *fp, Vec3f *pos)
{
    if (fp->fkind == REMIX_ID_PEPPY)
    {
        pos->x = PEPPY_NSP_LASER_X_PEPPY;
        pos->y = PEPPY_NSP_LASER_Y_PEPPY;
    }
    else
    {
        pos->x = PEPPY_NSP_LASER_X_KIRBY;
        pos->y = PEPPY_NSP_LASER_Y_KIRBY;
    }
    pos->z = 0.0F;
    gmCollisionGetFighterPartsWorldPosition(fp->joints[PEPPY_NSP_LASER_JOINT], pos);
    pos->z = 0.0F;
}

/* Fires one laser and spends one charge level (asm shoot_main_ temp variable 1 branch). */
static void ftRemixPeppyNSPShootFire(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f pos;

    fp->motion_vars.flags.flag0 = 0;
    fp->passive_vars.samus.charge_recoil--;
    ftRemixPeppyNSPGetLaserPosition(fp, &pos);
    ftRemixPeppyNSPLaserStageSetting(fighter_gobj, fp, &pos);
}

/* shoot_main_ end: the shot continues as the grounded or aerial shot end (Kirby keeps its own action). */
static void ftRemixPeppyNSPShootEnd(GObj *fighter_gobj, FTStruct *fp)
{
    s32 status_id;

    if (fp->fkind == REMIX_ID_PEPPY)
    {
        status_id = (fp->status_id == nFTRemixPeppyStatusNSPGroundShoot) ?
            nFTRemixPeppyStatusNSPGroundShoot : nFTRemixPeppyStatusNSPAirShoot;
    }
    else
    {
        status_id = (fp->status_id == nFTRemixPeppyKirbyStatusNSPGroundShoot) ?
            nFTRemixPeppyKirbyStatusNSPGroundShoot : nFTRemixPeppyKirbyStatusNSPAirShoot;
    }
    /* The asm leaves argument 4 (flags) as an unset stack word, so 0 is used here. */
    ftRemixPeppyNSPSetStatus(fighter_gobj, status_id, PEPPY_NSP_SHOOT_START_FRAME, FTSTATUS_PRESERVE_NONE);
}

/* Idle end of the shoot: returns to wait or fall once the animation has ended. */
static void ftRemixPeppyNSPShootIdle(GObj *fighter_gobj, FTStruct *fp)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
}

/* PeppyNSP.shoot_main_: fires on the temp variable 1 request and ends the shot at frame 32. */
static void ftRemixPeppyNSPShootMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->passive_vars.samus.charge_recoil == 0)
    {
        ftRemixPeppyNSPShootIdle(fighter_gobj, fp);
        return;
    }
    if (fp->motion_vars.flags.flag0 != 0)
    {
        ftRemixPeppyNSPShootFire(fighter_gobj, fp);
    }
    if (fighter_gobj->anim_frame == PEPPY_NSP_SHOOT_END_FRAME)
    {
        ftRemixPeppyNSPShootEnd(fighter_gobj, fp);
        return;
    }
    ftRemixPeppyNSPShootIdle(fighter_gobj, fp);
}

/* Charge tick: adds one charge level from the moveset request, then syncs the spark GObj. */
static void ftRemixPeppyNSPChargeSyncSpark(FTStruct *fp)
{
    WPStruct *wp;

    if (fp->status_vars.samus.specialn.charge_gobj == NULL)
    {
        return;
    }
    wp = wpGetStruct(fp->status_vars.samus.specialn.charge_gobj);
    wp->weapon_vars.charge_shot.charge_size = fp->passive_vars.samus.charge_recoil;
}

/* PeppyNSP.charge_main_ (ProcUpdate of NSP_Ground_Charge and NSP_Air_Charge). */
static void ftRemixPeppyNSPChargeMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_charge_tick = (fp->motion_vars.flags.flag0 != 0);

    fp->motion_vars.flags.flag0 = 0;
    if (!is_charge_tick)
    {
        return;
    }
    fp->status_vars.samus.specialn.charge_int = PEPPY_NSP_CHARGE_TICS;
    if (fp->passive_vars.samus.charge_recoil >= PEPPY_NSP_CHARGE_MAX)
    {
        return;
    }
    fp->passive_vars.samus.charge_recoil++;
    if (fp->passive_vars.samus.charge_recoil == PEPPY_NSP_CHARGE_MAX)
    {
        ftParamCheckSetFighterColAnimID(fighter_gobj, PEPPY_NSP_CHARGE_GFX_ROUTINE, 0);
        mpCommonSetFighterWaitOrFall(fighter_gobj);
        return;
    }
    ftRemixPeppyNSPChargeSyncSpark(fp);
}

const FTRemixProc dFTRemixPeppyNSPProcs[] =
{
    { REMIX_SYM_PeppyNSP__air_begin_collision_, ftRemixPeppyNSPAirBeginCollision },
    { REMIX_SYM_PeppyNSP__air_begin_initial_, ftRemixPeppyNSPAirBeginInitial },
    { REMIX_SYM_PeppyNSP__air_begin_interrupt_, ftRemixPeppyNSPAirBeginInterrupt },
    { REMIX_SYM_PeppyNSP__air_charge_collision_, ftRemixPeppyNSPAirChargeCollision },
    { REMIX_SYM_PeppyNSP__air_charge_interrupt_, ftRemixPeppyNSPAirChargeInterrupt },
    { REMIX_SYM_PeppyNSP__air_shoot_collision_, ftRemixPeppyNSPAirShootCollision },
    { REMIX_SYM_PeppyNSP__begin_main_, ftRemixPeppyNSPBeginMain },
    { REMIX_SYM_PeppyNSP__charge_main_, ftRemixPeppyNSPChargeMain },
    { REMIX_SYM_PeppyNSP__ground_begin_collision_, ftRemixPeppyNSPGroundBeginCollision },
    { REMIX_SYM_PeppyNSP__ground_begin_initial_, ftRemixPeppyNSPGroundBeginInitial },
    { REMIX_SYM_PeppyNSP__ground_begin_interrupt_, ftRemixPeppyNSPGroundBeginInterrupt },
    { REMIX_SYM_PeppyNSP__ground_charge_collision_, ftRemixPeppyNSPGroundChargeCollision },
    { REMIX_SYM_PeppyNSP__ground_charge_interrupt_, ftRemixPeppyNSPGroundChargeInterrupt },
    { REMIX_SYM_PeppyNSP__ground_shoot_collision_, ftRemixPeppyNSPGroundShootCollision },
    { REMIX_SYM_PeppyNSP__kirby_ground_begin_collision_, ftRemixPeppyNSPKirbyGroundBeginCollision },
    { REMIX_SYM_PeppyNSP__kirby_ground_charge_collision_, ftRemixPeppyNSPKirbyGroundChargeCollision },
    { REMIX_SYM_PeppyNSP__kirby_ground_shoot_collision_, ftRemixPeppyNSPKirbyGroundShootCollision },
    { REMIX_SYM_PeppyNSP__shoot_main_, ftRemixPeppyNSPShootMain }
};

const s32 dFTRemixPeppyNSPProcsNum = ARRAY_COUNT(dFTRemixPeppyNSPProcs);

#endif
