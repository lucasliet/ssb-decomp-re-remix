/**
 * ftremixsheiknsp.c - C ports of Smash Remix Sheik neutral special (Needles)
 * routines (.toolchain/pkgs/smashremix-src/src/Sheik/SheikSpecial.asm, SheikNSP scope).
 *
 * Sheik's NSP charges needles in player-struct fields the decomp does not name
 * (charge count 0x0AE0, transition flag 0x0B18, charge tick 0x0B1C) and fires
 * them through the Remix weapon descriptor below. Kirby and J Kirby copying
 * Sheik use the Kirby action ids of src/Kirby/Kirby.asm.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <ft/ftparam.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <mp/mpcommon.h>
#include <gm/gmcollision.h>
#include <ef/efmanager.h>
#include <wp/weapon.h>
#include <wp/wpmanager.h>
#include <wp/wpmain.h>
#include <it/itcommon/itlgun.h>
#include <wp/wpboss/wpbossbullet.h>
#include <wp/wpsamus/wpsamusbomb.h>

#define FT_REMIX_SHEIK_CHARGE(fp) ((fp)->passive_vars.samus.charge_recoil)
#define FT_REMIX_SHEIK_TRANSITION(fp) ((fp)->status_vars.common.dead.wait)
#define FT_REMIX_SHEIK_CHARGE_TICK(fp) ((fp)->status_vars.common.entry.lr)

#define SHEIK_NSP_CHARGE_TICS 0x12
#define SHEIK_NSP_CHARGE_MAX 6
#define SHEIK_NSP_FGM_NEEDLE 0x0410
#define SHEIK_NSP_CHARGE_GFX_ROUTINE 0x6D
#define SHEIK_NEEDLE_PROJECTILE_KIND 0x23
#define SHEIK_NEEDLE_AERIAL_ROTATION 0.785398F

enum
{
    nFTRemixSheikStatusNSPGroundBegin = 0xEA,
    nFTRemixSheikStatusNSPGroundCharge = 0xEB,
    nFTRemixSheikStatusNSPGroundShoot = 0xEC,
    nFTRemixSheikStatusNSPAirBegin = 0xED,
    nFTRemixSheikStatusNSPAirCharge = 0xEE,
    nFTRemixSheikStatusNSPAirShoot = 0xEF,
    nFTRemixSheikStatusLandingLight = 0x1F
};

enum
{
    nFTRemixSheikKirbyStatusNSPGroundBegin = 0x166,
    nFTRemixSheikKirbyStatusNSPGroundCharge = 0x167,
    nFTRemixSheikKirbyStatusNSPGroundShoot = 0x168,
    nFTRemixSheikKirbyStatusNSPAirBegin = 0x169,
    nFTRemixSheikKirbyStatusNSPAirCharge = 0x16A,
    nFTRemixSheikKirbyStatusNSPAirShoot = 0x16B
};

extern void *func_800269C0_275C0(u16 id);

typedef struct FTRemixSheikNeedleProps
{
    s32 duration;
    f32 max_speed;
    f32 gravity;
    f32 angle_ground;
    f32 angle_air;
    f32 speed;
    s32 o_attributes;
    f32 palette_index;
} FTRemixSheikNeedleProps;

static const FTRemixSheikNeedleProps sFTRemixSheikNeedleProps =
{
    .duration = 18,
    .max_speed = 250.0F,
    .gravity = 0.0F,
    .angle_ground = 0.0F,
    .angle_air = -0.785398F,
    .speed = 225.0F,
    .o_attributes = 0,
    .palette_index = 0.0F
};

static sb32 ftRemixSheikNSPIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Kirby and J Kirby copying Sheik take the Kirby action id, everyone else the Sheik one. */
static s32 ftRemixSheikNSPStatusFor(FTStruct *fp, s32 sheik_status, s32 kirby_status)
{
    return ftRemixSheikNSPIsKirby(fp) ? kirby_status : sheik_status;
}

static sb32 ftRemixSheikNSPIsAttackPressed(FTStruct *fp)
{
    return ((fp->input.pl.button_tap & fp->input.button_mask_a) != 0)
        || ((fp->input.pl.button_tap & fp->input.button_mask_b) != 0);
}

static sb32 ftRemixSheikNSPIsShieldPressed(FTStruct *fp)
{
    return (fp->input.pl.button_tap & fp->input.button_mask_z) != 0;
}

static void ftRemixSheikNSPSetProcDamage(FTStruct *fp)
{
    if (ftRemixSheikNSPIsKirby(fp))
    {
        fp->proc_damage = ftKirbyCopySamusSpecialNProcDamage;
    }
    else fp->proc_damage = ftSamusSpecialNProcDamage;
}

static void ftRemixSheikNSPInitStatusVars(FTStruct *fp)
{
    if (ftRemixSheikNSPIsKirby(fp))
    {
        ftKirbyCopySamusSpecialNInitStatusVars(fp);
    }
    else ftSamusSpecialNStartInitStatusVars(fp);
}

/* First step of SheikNSP.air_shoot_initial_ (copied from Samus's 0x8015DAA8): grounded fighters enter the air. */
static void ftRemixSheikNSPGroundedToAir(FTStruct *fp)
{
    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
        ftPhysicsClampAirVelXMax(fp);
    }
}

/* SheikNSP.begin_initial_ */
static void ftRemixSheikNSPBeginInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSheikNSPInitStatusVars(fp);
    FT_REMIX_SHEIK_TRANSITION(fp) = (FT_REMIX_SHEIK_CHARGE(fp) == SHEIK_NSP_CHARGE_MAX);
}

/* SheikNSP.ground_begin_initial_ */
static void ftRemixSheikNSPGroundBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSheikNSPBeginInitial(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPGroundBegin, nFTRemixSheikKirbyStatusNSPGroundBegin));
}

/* SheikNSP.air_begin_initial_ */
static void ftRemixSheikNSPAirBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSheikNSPBeginInitial(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPAirBegin, nFTRemixSheikKirbyStatusNSPAirBegin));
}

/* SheikNSP.ground_begin_transition_ */
static void ftRemixSheikNSPGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPGroundBegin, nFTRemixSheikKirbyStatusNSPGroundBegin), fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.air_begin_transition_ */
static void ftRemixSheikNSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPAirBegin, nFTRemixSheikKirbyStatusNSPAirBegin), fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.ground_charge_initial_ */
static void ftRemixSheikNSPGroundChargeSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    FT_REMIX_SHEIK_CHARGE_TICK(fp) = SHEIK_NSP_CHARGE_TICS;
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPGroundCharge, nFTRemixSheikKirbyStatusNSPGroundCharge), 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* SheikNSP.air_charge_initial_ */
static void ftRemixSheikNSPAirChargeSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    FT_REMIX_SHEIK_CHARGE_TICK(fp) = SHEIK_NSP_CHARGE_TICS;
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPAirCharge, nFTRemixSheikKirbyStatusNSPAirCharge), 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* SheikNSP.ground_charge_transition_ */
static void ftRemixSheikNSPGroundChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPGroundCharge, nFTRemixSheikKirbyStatusNSPGroundCharge), fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.air_charge_transition_ */
static void ftRemixSheikNSPAirChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPAirCharge, nFTRemixSheikKirbyStatusNSPAirCharge), fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.ground_shoot_initial_ */
static void ftRemixSheikNSPGroundShootSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;
    if (FT_REMIX_SHEIK_CHARGE(fp) == 0)
    {
        FT_REMIX_SHEIK_CHARGE(fp) = 1;
    }
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPGroundShoot, nFTRemixSheikKirbyStatusNSPGroundShoot), 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.air_shoot_initial_ */
static void ftRemixSheikNSPAirShootSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSheikNSPGroundedToAir(fp);
    fp->motion_vars.flags.flag0 = 0;
    if (FT_REMIX_SHEIK_CHARGE(fp) == 0)
    {
        FT_REMIX_SHEIK_CHARGE(fp) = 1;
    }
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPAirShoot, nFTRemixSheikKirbyStatusNSPAirShoot), 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.air_shoot_transition_ */
static void ftRemixSheikNSPAirShootTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSheikNSPStatusFor(fp, nFTRemixSheikStatusNSPAirShoot, nFTRemixSheikKirbyStatusNSPAirShoot), fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.begin_main_: waits for the animation end, then starts charge or shoot. */
static void ftRemixSheikNSPBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_shoot = (FT_REMIX_SHEIK_TRANSITION(fp) != 0);

    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    if (fp->ga == nMPKineticsGround)
    {
        if (is_shoot) ftRemixSheikNSPGroundShootSetStatus(fighter_gobj);
        else ftRemixSheikNSPGroundChargeSetStatus(fighter_gobj);
        return;
    }
    if (is_shoot) ftRemixSheikNSPAirShootSetStatus(fighter_gobj);
    else ftRemixSheikNSPAirChargeSetStatus(fighter_gobj);
}

/* SheikNSP.ground_begin_interrupt_ */
static void ftRemixSheikNSPGroundBeginProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixSheikNSPIsAttackPressed(fp))
    {
        FT_REMIX_SHEIK_TRANSITION(fp) = 1;
    }
    if (ftRemixSheikNSPIsShieldPressed(fp))
    {
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

/* SheikNSP.air_begin_interrupt_ */
static void ftRemixSheikNSPAirBeginProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixSheikNSPIsAttackPressed(fp))
    {
        FT_REMIX_SHEIK_TRANSITION(fp) = 1;
    }
    if (ftRemixSheikNSPIsShieldPressed(fp))
    {
        ftCommonFallSetStatus(fighter_gobj);
    }
}

/* SheikNSP.ground_begin_collision_ */
static void ftRemixSheikNSPGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSheikNSPAirBeginTransition);
}

/* SheikNSP.air_begin_collision_ */
static void ftRemixSheikNSPAirBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSheikNSPGroundBeginTransition);
}

/*
 * SheikNSP.charge_main_ _charge_continue (SheikSpecial.asm 1221-1225): copies the charge level to
 * user_data + 0x2A4 of the GObj stored at FTStruct 0x0B20. 0x0B20 is status_vars.samus.specialn.charge_gobj
 * (Samus 0x8015D5AC layout) and 0x2A4 is weapon_vars.charge_shot.charge_size, which scales the charge shot
 * model. The begin status init (0x8015DB4C) leaves charge_gobj NULL, so the store is skipped in that case.
 */
static void ftRemixSheikNSPSetChargeShotLevel(FTStruct *fp, s32 charge)
{
    GObj *charge_gobj = fp->status_vars.samus.specialn.charge_gobj;

    if (charge_gobj == NULL)
    {
        return;
    }
    wpGetStruct(charge_gobj)->weapon_vars.charge_shot.charge_size = charge;
}

/* SheikNSP.charge_main_: counts the needle charge up every 18 frames and returns to idle at full charge. */
static void ftRemixSheikNSPChargeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 charge_tick = FT_REMIX_SHEIK_CHARGE_TICK(fp) - 1;
    s32 charge;

    FT_REMIX_SHEIK_CHARGE_TICK(fp) = charge_tick;
    if (charge_tick != 0)
    {
        return;
    }
    charge = FT_REMIX_SHEIK_CHARGE(fp);
    FT_REMIX_SHEIK_CHARGE_TICK(fp) = SHEIK_NSP_CHARGE_TICS;
    if (charge >= SHEIK_NSP_CHARGE_MAX)
    {
        return;
    }
    charge += 1;
    FT_REMIX_SHEIK_CHARGE(fp) = charge;
    if (charge == SHEIK_NSP_CHARGE_MAX)
    {
        ftParamCheckSetFighterColAnimID(fighter_gobj, SHEIK_NSP_CHARGE_GFX_ROUTINE, 0);
        mpCommonSetFighterWaitOrFall(fighter_gobj);
        return;
    }
    ftRemixSheikNSPSetChargeShotLevel(fp, charge);
}

/* SheikNSP.ground_charge_interrupt_ */
static void ftRemixSheikNSPGroundChargeProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 escape_status;

    if (ftRemixSheikNSPIsAttackPressed(fp))
    {
        ftRemixSheikNSPGroundShootSetStatus(fighter_gobj);
        return;
    }
    escape_status = ftCommonEscapeGetStatus(fp);
    if (escape_status != -1)
    {
        /* Remix passes the fighter GObj pointer as itemthrow_buffer_tics here (a2 reloaded
         * from 0x20(sp)). A pointer has no tic meaning, so 0 is passed; see the notes file. */
        ftCommonEscapeSetStatus(fighter_gobj, escape_status, 0);
        return;
    }
    if (ftRemixSheikNSPIsShieldPressed(fp))
    {
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

/* SheikNSP.air_charge_interrupt_ */
static void ftRemixSheikNSPAirChargeProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 buttons = fp->input.pl.button_tap;

    if (buttons & (A_BUTTON | B_BUTTON))
    {
        ftRemixSheikNSPAirShootSetStatus(fighter_gobj);
        return;
    }
    if (ftRemixSheikNSPIsShieldPressed(fp))
    {
        ftCommonFallSetStatus(fighter_gobj);
    }
}

/* SheikNSP.ground_charge_collision_ */
static void ftRemixSheikNSPGroundChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSheikNSPAirChargeTransition);
}

/* SheikNSP.air_charge_collision_ */
static void ftRemixSheikNSPAirChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSheikNSPGroundChargeTransition);
}

/* SheikNSP.ground_shoot_collision_ */
static void ftRemixSheikNSPGroundShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSheikNSPAirShootTransition);
}

/* SheikNSP.kirby_ground_begin_collision_. Kirby's copy uses the floor check (0x800DDDDC), not the edge check of Sheik's. */
static void ftRemixSheikNSPKirbyGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixSheikNSPAirBeginTransition);
}

/* SheikNSP.kirby_ground_charge_collision_ (floor check, as above). */
static void ftRemixSheikNSPKirbyGroundChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixSheikNSPAirChargeTransition);
}

/* SheikNSP.kirby_ground_shoot_collision_ (floor check, as above). */
static void ftRemixSheikNSPKirbyGroundShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixSheikNSPAirShootTransition);
}

/* SheikNSP.ground_shoot_transition_: landing from the aerial shot. Remix sets the LandingLight action, not the shoot action. */
static void ftRemixSheikNSPGroundShootTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusLandingLight, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixSheikNSPSetProcDamage(fp);
}

/* SheikNSP.air_shoot_collision_ */
static void ftRemixSheikNSPAirShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSheikNSPGroundShootTransition);
}

/* Weapon proc (WPDesc.proc_update): SheikNSP.needle_main_. Returns TRUE when the needle expires. */
static sb32 ftRemixSheikNeedleProcUpdate(GObj *needle_gobj)
{
    WPStruct *wp = wpGetStruct(needle_gobj);

    if (wpMainDecLifeCheckExpire(wp))
    {
        efManagerDustExpandSmallMakeEffect(&DObjGetStruct(needle_gobj)->translate.vec.f, 1.0F);
        return TRUE;
    }
    wpMainApplyGravityClampTVel(wp, sFTRemixSheikNeedleProps.gravity, sFTRemixSheikNeedleProps.max_speed);
    return FALSE;
}

/* Weapon proc for every hit, shield, absorb and setoff: SheikNSP.needle_destruction_. */
static sb32 ftRemixSheikNeedleProcDestruction(GObj *needle_gobj)
{
    efManagerDustExpandSmallMakeEffect(&DObjGetStruct(needle_gobj)->translate.vec.f, 1.0F);
    return TRUE;
}

/* Remix needle_projectile_struct as a WPDesc. p_weapon and o_attributes are filled per
 * spawn; transform_types {0x12, 0x47, 0x00} is the big-endian word 0x12470000. */
static WPDesc sFTRemixSheikNeedleDesc =
{
    .flags = 0,
    .kind = SHEIK_NEEDLE_PROJECTILE_KIND,
    .p_weapon = NULL,
    .o_attributes = 0,
    .transform_types = { 0x12, 0x47, 0x00 },
    .proc_update = ftRemixSheikNeedleProcUpdate,
    .proc_map = itLGunWeaponAmmoProcMap,
    .proc_hit = ftRemixSheikNeedleProcDestruction,
    .proc_shield = ftRemixSheikNeedleProcDestruction,
    .proc_hop = wpBossBulletProcHop,
    .proc_setoff = ftRemixSheikNeedleProcDestruction,
    .proc_reflector = wpSamusBombProcReflector,
    .proc_absorb = ftRemixSheikNeedleProcDestruction
};

/* SheikNSP.needle_stage_setting_: spawns one needle from the hand at spawn_pos. */
static void ftRemixSheikNeedleStageSetting(GObj *fighter_gobj, Vec3f *spawn_pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *needle_gobj;
    WPStruct *needle_wp;
    DObj *needle_dobj;
    f32 angle;
    f32 palette_index;
    sb32 is_aerial = (fp->ga == nMPKineticsAir);

    func_800269C0_275C0(SHEIK_NSP_FGM_NEEDLE);
    sFTRemixSheikNeedleDesc.p_weapon = ftRemixPowerData(fp)->p_file_special1;
    sFTRemixSheikNeedleDesc.o_attributes = sFTRemixSheikNeedleProps.o_attributes;
    needle_gobj = wpManagerMakeWeapon(fighter_gobj, &sFTRemixSheikNeedleDesc, spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (needle_gobj == NULL)
    {
        return;
    }
    needle_wp = wpGetStruct(needle_gobj);
    needle_wp->lifetime = sFTRemixSheikNeedleProps.duration;
    angle = is_aerial ? sFTRemixSheikNeedleProps.angle_air : sFTRemixSheikNeedleProps.angle_ground;
    needle_wp->physics.vel_air.z = 0.0F;
    needle_wp->physics.vel_air.x = (__cosf(angle) * sFTRemixSheikNeedleProps.speed) * (f32)fp->lr;
    needle_wp->physics.vel_air.y = __sinf(angle) * sFTRemixSheikNeedleProps.speed;

    needle_dobj = DObjGetStruct(needle_gobj);
    palette_index = (f32)fp->lr;
    if (is_aerial)
    {
        needle_dobj->rotate.vec.f.x = SHEIK_NEEDLE_AERIAL_ROTATION;
        palette_index = sFTRemixSheikNeedleProps.palette_index;
    }
    needle_dobj->mobj->palette_id = palette_index;
    wpMainVelSetModelPitch(needle_gobj);
}

/* SheikNSP.shoot_main_: fires one needle per frame flagged by moveset temp variable 1. */
static void ftRemixSheikNSPShootProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f spawn_pos = { 0.0F, 0.0F, 0.0F };

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        if (FT_REMIX_SHEIK_CHARGE(fp) != 0)
        {
            FT_REMIX_SHEIK_CHARGE(fp) -= 1;
            gmCollisionGetFighterPartsWorldPosition(fp->joints[15], &spawn_pos);
            spawn_pos.z = 0.0F;
            ftRemixSheikNeedleStageSetting(fighter_gobj, &spawn_pos);
        }
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
        fp->motion_vars.flags.flag0 = 0;
    }
}

/* Remix address -> port routine for every SheikNSP routine the Sheik action array
 * and special tables reference. */
const FTRemixProc dFTRemixSheikNSPProcs[] =
{
    { REMIX_SYM_SheikNSP__air_begin_collision_, ftRemixSheikNSPAirBeginProcMap },
    { REMIX_SYM_SheikNSP__air_begin_initial_, ftRemixSheikNSPAirBeginSetStatus },
    { REMIX_SYM_SheikNSP__air_begin_interrupt_, ftRemixSheikNSPAirBeginProcInterrupt },
    { REMIX_SYM_SheikNSP__air_charge_collision_, ftRemixSheikNSPAirChargeProcMap },
    { REMIX_SYM_SheikNSP__air_charge_interrupt_, ftRemixSheikNSPAirChargeProcInterrupt },
    { REMIX_SYM_SheikNSP__begin_main_, ftRemixSheikNSPBeginProcUpdate },
    { REMIX_SYM_SheikNSP__charge_main_, ftRemixSheikNSPChargeProcUpdate },
    { REMIX_SYM_SheikNSP__ground_begin_collision_, ftRemixSheikNSPGroundBeginProcMap },
    { REMIX_SYM_SheikNSP__ground_begin_initial_, ftRemixSheikNSPGroundBeginSetStatus },
    { REMIX_SYM_SheikNSP__ground_begin_interrupt_, ftRemixSheikNSPGroundBeginProcInterrupt },
    { REMIX_SYM_SheikNSP__ground_charge_collision_, ftRemixSheikNSPGroundChargeProcMap },
    { REMIX_SYM_SheikNSP__ground_charge_interrupt_, ftRemixSheikNSPGroundChargeProcInterrupt },
    { REMIX_SYM_SheikNSP__ground_shoot_collision_, ftRemixSheikNSPGroundShootProcMap },
    { REMIX_SYM_SheikNSP__shoot_main_, ftRemixSheikNSPShootProcUpdate },
    { REMIX_SYM_SheikNSP__kirby_ground_begin_collision_, ftRemixSheikNSPKirbyGroundBeginProcMap },
    { REMIX_SYM_SheikNSP__kirby_ground_charge_collision_, ftRemixSheikNSPKirbyGroundChargeProcMap },
    { REMIX_SYM_SheikNSP__kirby_ground_shoot_collision_, ftRemixSheikNSPKirbyGroundShootProcMap },
    { REMIX_SYM_SheikNSP__air_shoot_collision_, ftRemixSheikNSPAirShootProcMap }
};

const s32 dFTRemixSheikNSPProcsNum = ARRAY_COUNT(dFTRemixSheikNSPProcs);

#endif
