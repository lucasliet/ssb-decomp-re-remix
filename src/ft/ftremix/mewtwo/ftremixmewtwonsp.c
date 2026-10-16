/**
 * ftremixmewtwonsp.c - C ports of Smash Remix Mewtwo neutral special (Shadow Ball)
 * routines (.toolchain/pkgs/smashremix-src/src/Mewtwo/MewtwoSpecial.asm, scope MewtwoNSP).
 *
 * Mewtwo's charge and shoot states are Samus's Charge Shot states (ftsamusspecialn.c)
 * with the Shadow Ball projectile. Shadow Ball keeps its state in the WPStruct
 * weapon_vars block (see MewtwoShadowBallVars); the Samus fighter state
 * (status_vars.samus.specialn, passive_vars.samus) is used as in Samus.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include <stddef.h>
#include "remix/remix_symbols.h"
#include "remix/remix_roster_data.h"
#include <ft/fighter.h>
#include <wp/weapon.h>
#include <ef/efmanager.h>
#include <mp/mpcommon.h>
#include <ft/ftchar/ftsamus/ftsamus.h>
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <wp/wpsamus/wpsamuschargeshot.h>
#include <wp/wpmap.h>
#include <sys/utils.h>
#include <lb/lbremixgfx.h>

#define MTWO_SHADOW_BALL_CHARGE_COLANIM 0x58 /* Mewtwo's charge gfx id (hard coded in MewtwoSpecial.asm) */
#define MTWO_KIRBY_SHADOW_BALL_CHARGE_COLANIM 0x64 /* Kirby's copy charge gfx id (hard coded in kirby_charge_main_) */
#define MTWO_GFX_BLUE_EXPLOSION 0x5C /* Remix GFX ids, src/GFX.asm */
#define MTWO_GFX_BLUE_BOMB_EXPLOSION 0x5D
#define MTWO_GFX_DARK_CROSS 0x6C
#define MTWO_SHADOW_BALL_FREQUENCY 3.0F
#define MTWO_SHADOW_BALL_AMPLITUDE 100.0F
#define MTWO_SHADOW_BALL_RAD_PER_TIC 0.10471976F /* 0x3DD67750 = 2*pi/60 */
#define MTWO_SHADOW_BALL_CHARGE_FLAGS 0x0802     /* COLANIM plus bit 0x800, as the asm literal */

/* Status ids from scope Action in Mewtwo.asm (0xDC + i). */
enum
{
    nFTRemixMewtwoStatusShadowBallStart = 0xDE,
    nFTRemixMewtwoStatusShadowBallCharge = 0xDF,
    nFTRemixMewtwoStatusShadowBallShoot = 0xE0,
    nFTRemixMewtwoStatusShadowBallStartAir = 0xE1,
    nFTRemixMewtwoStatusShadowBallChargeAir = 0xE2,
    nFTRemixMewtwoStatusShadowBallShootAir = 0xE4
};

/* Kirby.Action ids for Kirby copying this move, allocated by add_new_action in Kirby.asm
 * (0xDC + Kirby's base action count + slot). Values read from the Remix ROM action array
 * for KIRBY (entries 110..115 point at the MewtwoNSP routines). */
enum
{
    nFTRemixKirbyStatusMewtwoNSPGroundBegin = 0x14A,
    nFTRemixKirbyStatusMewtwoNSPGroundCharge = 0x14B,
    nFTRemixKirbyStatusMewtwoNSPGroundShoot = 0x14C,
    nFTRemixKirbyStatusMewtwoNSPAirBegin = 0x14D,
    nFTRemixKirbyStatusMewtwoNSPAirCharge = 0x14E,
    nFTRemixKirbyStatusMewtwoNSPAirShoot = 0x14F
};

extern alSoundEffect *func_800269C0_275C0(u16);

/* Shadow Ball state in the WPStruct weapon_vars block (WPStruct 0x29C onwards). It
 * overlays wpSamusWeaponVarsChargeShot, which the shared Samus code writes: is_release
 * is frame_count, is_full_charge is begin_shot, charge_size is charge_level. owner_gobj
 * keeps its pointer type so the LP64 layout stays aligned with the Samus struct. */
typedef struct MewtwoShadowBallVars
{
    s32 frame_count;        /* 0x29C: 0 while charging, frames since the shot started afterwards */
    s32 begin_shot;         /* 0x2A0: set when the shot is released */
    s32 charge_level;       /* 0x2A4: index into dFTRemixMewtwoShadowBallLevels */
    GObj *owner_gobj;       /* 0x2A8: wpSamusWeaponVarsChargeShot.owner_gobj */
    f32 base_y;             /* 0x2AC: y position the bobbing is centred on */
    s32 is_shield_bounced;  /* 0x2B0 */
} MewtwoShadowBallVars;

_Static_assert(offsetof(MewtwoShadowBallVars, owner_gobj) == offsetof(wpSamusWeaponVarsChargeShot, owner_gobj),
               "Shadow Ball vars must overlay wpSamusWeaponVarsChargeShot");

/* Shadow Ball charge levels, same layout as the Charge Shot attributes (MewtwoSpecial.asm charge_level_array). */
static const wpSamusChargeShotAttributes sFTRemixMewtwoShadowBallLevels[] =
{
    { 130.0F, 70.0F, 2, 90, 10, 0x1A, 0x3C9, 0x1C, 1 },
    { 160.0F, 72.0F, 4, 110, 10, 0x1A, 0x3CA, 0x1C, 1 },
    { 200.0F, 74.0F, 6, 130, 10, 0x1A, 0x3CB, 0x1B, 1 },
    { 260.0F, 76.0F, 9, 150, 10, 0x3C, 0x3CC, 0x1B, 1 },
    { 330.0F, 78.0F, 12, 170, 10, 0x3C, 0x3CD, 0x1B, 1 },
    { 410.0F, 80.0F, 15, 190, 10, 0x3C, 0x3CE, 0x19, 1 },
    { 500.0F, 82.0F, 18, 210, 10, 0x3C, 0x3CF, 0x19, 1 },
    { 600.0F, 84.0F, 22, 230, 10, 0xC1, 0x3D0, 0x19, 2 }
};

/* Returns TRUE when the fighter is Kirby or J Kirby, which copy Mewtwo's power. */
static sb32 ftRemixMewtwoSpecialNIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Kirby (and J Kirby) use their own action id for the same move. */
static s32 ftRemixMewtwoSpecialNPickStatus(FTStruct *fp, s32 mewtwo_status, s32 kirby_status)
{
    if (ftRemixMewtwoSpecialNIsKirby(fp))
    {
        return kirby_status;
    }
    return mewtwo_status;
}

/* Charge level of the power in use: Kirby copies it into passive_vars.kirby.copysamus_charge_level. */
static s32 ftRemixMewtwoSpecialNChargeLevel(FTStruct *fp)
{
    if (ftRemixMewtwoSpecialNIsKirby(fp))
    {
        return fp->passive_vars.kirby.copysamus_charge_level;
    }
    return fp->passive_vars.samus.charge_level;
}

/* Sets the on-hit routine: Kirby uses the copied-Samus handler, Mewtwo the Samus one. */
static void ftRemixMewtwoSpecialNSetProcDamage(FTStruct *fp)
{
    if (ftRemixMewtwoSpecialNIsKirby(fp))
    {
        fp->proc_damage = ftKirbyCopySamusSpecialNProcDamage;
        return;
    }
    fp->proc_damage = ftSamusSpecialNProcDamage;
}

/* Sets a status with speed 1.0 and the Mewtwo on-hit routine, as the Remix transitions do. */
static void ftRemixMewtwoSpecialNChangeStatus(GObj *fighter_gobj, s32 status_id, f32 start_frame, sb32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, start_frame, 1.0F, flags);
    ftRemixMewtwoSpecialNSetProcDamage(fp);
}

/* Shadow Ball state for the weapon's WPStruct. */
static MewtwoShadowBallVars *ftRemixMewtwoShadowBallGetVars(GObj *weapon_gobj)
{
    return (MewtwoShadowBallVars *) &wpGetStruct(weapon_gobj)->weapon_vars;
}

/* Scales the Shadow Ball sprite for its current charge level. */
static void ftRemixMewtwoShadowBallSetChargeScale(GObj *weapon_gobj, MewtwoShadowBallVars *sb)
{
    f32 scale = sFTRemixMewtwoShadowBallLevels[sb->charge_level].gfx_size / WPCHARGESHOT_GFX_SIZE_DIV;

    DObjGetStruct(weapon_gobj)->scale.vec.f.x = scale;
    DObjGetStruct(weapon_gobj)->scale.vec.f.y = scale;
}

/* Applies the charge level's speed, damage, hitbox and sounds to the launched Shadow Ball. */
static void ftRemixMewtwoShadowBallApplyChargeLevel(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    MewtwoShadowBallVars *sb = ftRemixMewtwoShadowBallGetVars(weapon_gobj);
    const wpSamusChargeShotAttributes *level = &sFTRemixMewtwoShadowBallLevels[sb->charge_level];
    f32 coll_size = level->coll_size * 0.5F;

    wp->physics.vel_air.x = level->vel_x * wp->lr;

    wp->attack_coll.damage = level->damage;
    wp->attack_coll.size = level->attack_size * 0.5F;

    wp->coll_data.map_coll.width = coll_size;
    wp->coll_data.map_coll.top = coll_size;
    wp->coll_data.map_coll.bottom = -coll_size;

    func_800269C0_275C0(level->shoot_fgm_id);

    wp->attack_coll.fgm_id = level->attack_fgm_id;
    wp->attack_coll.priority = level->priority;
}

/* Shadow Ball bobs up and down around base_y while charging, once it has not bounced off a shield. */
static void ftRemixMewtwoShadowBallBob(GObj *weapon_gobj, MewtwoShadowBallVars *sb)
{
    f32 angle = MTWO_SHADOW_BALL_FREQUENCY * MTWO_SHADOW_BALL_RAD_PER_TIC * (f32) sb->frame_count;

    DObjGetStruct(weapon_gobj)->translate.vec.f.y = sb->base_y + (MTWO_SHADOW_BALL_AMPLITUDE * __sinf(angle));
}

/* Shadow Ball ProcUpdate: charges (scale and bob) until begin_shot, then launches once. Based on 0x80168BFC. */
static sb32 ftRemixMewtwoShadowBallProcUpdate(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    MewtwoShadowBallVars *sb = ftRemixMewtwoShadowBallGetVars(weapon_gobj);

    if (sb->frame_count != 0)
    {
        if (sb->is_shield_bounced == FALSE)
        {
            ftRemixMewtwoShadowBallBob(weapon_gobj, sb);
        }
        sb->frame_count++;
        return FALSE;
    }
    ftRemixMewtwoShadowBallSetChargeScale(weapon_gobj, sb);

    if (sb->begin_shot == FALSE)
    {
        return FALSE;
    }
    sb->base_y = DObjGetStruct(weapon_gobj)->translate.vec.f.y;
    sb->is_shield_bounced = FALSE;
    ftRemixMewtwoShadowBallApplyChargeLevel(weapon_gobj);

    wp->attack_coll.attack_state = nGMAttackStateNew;
    wpProcessUpdateHitPositions(weapon_gobj);

    sb->frame_count = 1;
    return FALSE;
}

/* Shadow Ball explosion on map collision: full charge is the large one. */
static void ftRemixMewtwoShadowBallExplode(GObj *weapon_gobj, s32 charge_level)
{
    Vec3f *pos = &DObjGetStruct(weapon_gobj)->translate.vec.f;

    if (charge_level == FTSAMUS_CHARGE_MAX)
    {
        lbRemixGfxSetNextEffect(MTWO_GFX_BLUE_BOMB_EXPLOSION);
        efManagerSparkleWhiteMultiExplodeMakeEffect(pos);
        lbRemixGfxClearOverride();
        efManagerQuakeMakeEffect(1);
        func_800269C0_275C0(1);
        return;
    }
    lbRemixGfxSetNextEffect(MTWO_GFX_BLUE_EXPLOSION);
    efManagerSparkleWhiteMakeEffect(pos);
    lbRemixGfxClearOverride();
    efManagerQuakeMakeEffect(0);
    func_800269C0_275C0(0);
}

/* Shadow Ball ProcMap: explodes on map collision once launched. Based on 0x801688C4. */
static sb32 ftRemixMewtwoShadowBallProcMap(GObj *weapon_gobj)
{
    MewtwoShadowBallVars *sb = ftRemixMewtwoShadowBallGetVars(weapon_gobj);

    if (sb->frame_count == 0)
    {
        return FALSE;
    }
    if (wpMapTestAllCheckCollEnd(weapon_gobj) == FALSE)
    {
        return FALSE;
    }
    ftRemixMewtwoShadowBallExplode(weapon_gobj, sb->charge_level);

    return TRUE;
}

/* Shadow Ball ProcHit, ProcShield, ProcSetOff and ProcAbsorb: dark cross effect and destroy. Based on 0x80168D24. */
static sb32 ftRemixMewtwoShadowBallProcHit(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);

    lbRemixGfxSetNextEffect(MTWO_GFX_DARK_CROSS);
    efManagerDamageFireMakeEffect(&DObjGetStruct(weapon_gobj)->translate.vec.f, wp->attack_coll.damage);
    lbRemixGfxClearOverride();
    func_800269C0_275C0(0x3C);

    return TRUE;
}

/* Shadow Ball ProcHop: marks the shield bounce, then runs the Charge Shot hop. Based on 0x80168D54. */
static sb32 ftRemixMewtwoShadowBallProcHop(GObj *weapon_gobj)
{
    MewtwoShadowBallVars *sb = ftRemixMewtwoShadowBallGetVars(weapon_gobj);

    sb->is_shield_bounced = TRUE;

    return wpSamusChargeShotProcHop(weapon_gobj);
}

/*
 * Shadow Ball weapon descriptor (Remix projectile struct, MewtwoNSP.projectile_struct).
 * p_weapon points at the fighter's file, so ftRemixMewtwoShadowBallMakeWeapon
 * copies it and sets ftRemixPowerData(fp)->p_file_special3 per spawn.
 */
static const WPDesc dFTRemixMewtwoShadowBallWeaponDesc =
{
    0x00,                                   /* Render flags */
    nWPKindChargeShot,                      /* Remix projectile id 2 */
    NULL,                                   /* Mewtwo file 8 (p_file_special3), set per spawn */
    0x00000000,                             /* Attributes offset */
    { nGCMatrixKindTra, 0x2E, 0 },          /* DObj transform types (0x122E0000) */
    ftRemixMewtwoShadowBallProcUpdate,
    ftRemixMewtwoShadowBallProcMap,
    ftRemixMewtwoShadowBallProcHit,
    ftRemixMewtwoShadowBallProcHit,
    ftRemixMewtwoShadowBallProcHop,
    ftRemixMewtwoShadowBallProcHit,
    wpSamusChargeShotProcReflector,
    ftRemixMewtwoShadowBallProcHit
};

/*
 * Charge Shot stage setting (0x80168DDC) with Remix's Mewtwo patches applied:
 * get_chargeinstructions_struct_ (samusshared.asm:81) swaps in the Shadow Ball
 * descriptor and charge_fgm_fix_ (MewtwoSpecial.asm:1074) reads the Mewtwo FGM.
 * Mewtwo never spawns a released ball here; the release goes through the
 * attached ball (ftSamusSpecialNEndProcUpdate).
 */
static GObj *ftRemixMewtwoShadowBallMakeWeapon(GObj *fighter_gobj, Vec3f *pos, s32 charge_level)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    WPDesc desc = dFTRemixMewtwoShadowBallWeaponDesc;
    GObj *weapon_gobj;
    WPStruct *wp;
    MewtwoShadowBallVars *sb;
    f32 scale;

    desc.p_weapon = ftRemixPowerData(fp)->p_file_special3;

    weapon_gobj = wpManagerMakeWeapon(fighter_gobj, &desc, pos, WEAPON_FLAG_PARENT_FIGHTER);

    if (weapon_gobj == NULL)
    {
        return NULL;
    }
    wp = wpGetStruct(weapon_gobj);
    sb = ftRemixMewtwoShadowBallGetVars(weapon_gobj);

    sb->frame_count = 0;
    sb->charge_level = charge_level;

    wp->attack_coll.attack_state = nGMAttackStateOff;
    sb->begin_shot = FALSE;
    sb->owner_gobj = fighter_gobj;

    ftParamPlayLoopSFX(fp, sFTRemixMewtwoShadowBallLevels[charge_level].charge_fgm_id);

    scale = sFTRemixMewtwoShadowBallLevels[charge_level].gfx_size / WPCHARGESHOT_GFX_SIZE_DIV;
    DObjGetStruct(weapon_gobj)->scale.vec.f.x = DObjGetStruct(weapon_gobj)->scale.vec.f.y = scale;

    wp->proc_dead = wpSamusChargeShotProcDead;

    return weapon_gobj;
}

/* MewtwoNSP.begin_initial_: common start of the neutral special (ground and air). Based on 0x8015DB64. */
static void ftRemixMewtwoSpecialNBeginInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);

    if (ftRemixMewtwoSpecialNIsKirby(fp))
    {
        ftKirbyCopySamusSpecialNInitStatusVars(fp);
    }
    else ftSamusSpecialNStartInitStatusVars(fp);

    fp->status_vars.samus.specialn.is_release = (ftRemixMewtwoSpecialNChargeLevel(fp) == FTSAMUS_CHARGE_MAX);
}

/* MewtwoNSP.ground_begin_initial_ (special entry). */
static void ftRemixMewtwoSpecialNGroundBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMewtwoSpecialNBeginInitial(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallStart, nFTRemixKirbyStatusMewtwoNSPGroundBegin));
}

/* MewtwoNSP.air_begin_initial_ (special entry). */
static void ftRemixMewtwoSpecialAirNBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMewtwoSpecialNBeginInitial(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallStartAir, nFTRemixKirbyStatusMewtwoNSPAirBegin));
}

/* MewtwoNSP.attach_shadow_ball_: spawns the Shadow Ball at the hand (Charge Shot spawn at 0x80168DDC). */
static void ftRemixMewtwoSpecialNAttachShadowBall(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos;

    ftRemixMewtwoSpecialNSetProcDamage(fp);
    fp->status_vars.samus.specialn.charge_int = FTSAMUS_CHARGE_INT;

    ftSamusSpecialNGetChargeShotPosition(fp, &pos);
    fp->status_vars.samus.specialn.charge_gobj = ftRemixMewtwoShadowBallMakeWeapon(fighter_gobj, &pos, ftRemixMewtwoSpecialNChargeLevel(fp));
}

/* MewtwoNSP.ground_charge_initial_ and air_charge_initial_: starts charging and attaches the ball. */
static void ftRemixMewtwoSpecialNChargeInitial(GObj *fighter_gobj, s32 mewtwo_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, mewtwo_status, kirby_status), 0.0F, FTSTATUS_PRESERVE_COLANIM);
    ftRemixMewtwoSpecialNAttachShadowBall(fighter_gobj);
}

/* MewtwoNSP.ground_shoot_initial_: fires the Shadow Ball from the grounded shoot state. Based on 0x8015DA60. */
static void ftRemixMewtwoSpecialNGroundShootInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallShoot, nFTRemixKirbyStatusMewtwoNSPGroundShoot), 0.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* MewtwoNSP.air_shoot_initial_: fires the Shadow Ball from the aerial shoot state. Based on 0x8015DAA8. */
static void ftRemixMewtwoSpecialAirNShootInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
        ftPhysicsClampAirVelXMax(fp);
    }
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallShootAir, nFTRemixKirbyStatusMewtwoNSPAirShoot), 0.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* MewtwoNSP.begin_main_: waits for the begin animation, then charges or shoots. */
static void ftRemixMewtwoSpecialNBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    if (fp->ga == nMPKineticsAir)
    {
        if (fp->status_vars.samus.specialn.is_release != FALSE)
        {
            ftRemixMewtwoSpecialNAttachShadowBall(fighter_gobj);
            ftRemixMewtwoSpecialAirNShootInitial(fighter_gobj);
            return;
        }
        ftRemixMewtwoSpecialNChargeInitial(fighter_gobj, nFTRemixMewtwoStatusShadowBallChargeAir, nFTRemixKirbyStatusMewtwoNSPAirCharge);
        return;
    }
    if (fp->status_vars.samus.specialn.is_release != FALSE)
    {
        ftRemixMewtwoSpecialNAttachShadowBall(fighter_gobj);
        ftRemixMewtwoSpecialNGroundShootInitial(fighter_gobj);
        return;
    }
    ftRemixMewtwoSpecialNChargeInitial(fighter_gobj, nFTRemixMewtwoStatusShadowBallCharge, nFTRemixKirbyStatusMewtwoNSPGroundCharge);
}

/* MewtwoNSP.ground_begin_transition_: landing into the grounded begin state. */
static void ftRemixMewtwoSpecialNGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallStart, nFTRemixKirbyStatusMewtwoNSPGroundBegin), fighter_gobj->anim_frame, FTSTATUS_PRESERVE_COLANIM);
}

/* MewtwoNSP.air_begin_transition_: leaving the ground into the aerial begin state. */
static void ftRemixMewtwoSpecialAirNBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallStartAir, nFTRemixKirbyStatusMewtwoNSPAirBegin), fighter_gobj->anim_frame, FTSTATUS_PRESERVE_COLANIM);
}

/* MewtwoNSP.ground_charge_transition_: landing into the grounded charge state. */
static void ftRemixMewtwoSpecialNGroundChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallCharge, nFTRemixKirbyStatusMewtwoNSPGroundCharge), fighter_gobj->anim_frame, MTWO_SHADOW_BALL_CHARGE_FLAGS);
}

/* MewtwoNSP.air_charge_transition_: leaving the ground into the aerial charge state. */
static void ftRemixMewtwoSpecialAirNChargeTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallChargeAir, nFTRemixKirbyStatusMewtwoNSPAirCharge), fighter_gobj->anim_frame, MTWO_SHADOW_BALL_CHARGE_FLAGS);
}

/* MewtwoNSP.ground_shoot_transition_: landing into the grounded shoot state. */
static void ftRemixMewtwoSpecialNGroundShootTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallShoot, nFTRemixKirbyStatusMewtwoNSPGroundShoot), fighter_gobj->anim_frame, FTSTATUS_PRESERVE_COLANIM);
}

/* MewtwoNSP.air_shoot_transition_: leaving the ground into the aerial shoot state. */
static void ftRemixMewtwoSpecialAirNShootTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixMewtwoSpecialNChangeStatus(fighter_gobj, ftRemixMewtwoSpecialNPickStatus(fp, nFTRemixMewtwoStatusShadowBallShootAir, nFTRemixKirbyStatusMewtwoNSPAirShoot), fighter_gobj->anim_frame, FTSTATUS_PRESERVE_COLANIM);
}

/* MewtwoNSP.ground_begin_collision_: walking off an edge while beginning in the air. */
static void ftRemixMewtwoSpecialNGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMewtwoSpecialAirNBeginTransition);
}

/* MewtwoNSP.air_begin_collision_: landing from the aerial begin state. */
static void ftRemixMewtwoSpecialAirNBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMewtwoSpecialNGroundBeginTransition);
}

/* MewtwoNSP.ground_charge_collision_: walking off an edge while charging on the ground. */
static void ftRemixMewtwoSpecialNGroundChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMewtwoSpecialAirNChargeTransition);
}

/* MewtwoNSP.air_charge_collision_: landing from the aerial charge state. */
static void ftRemixMewtwoSpecialAirNChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMewtwoSpecialNGroundChargeTransition);
}

/* MewtwoNSP.ground_shoot_collision_: walking off an edge while shooting on the ground. */
static void ftRemixMewtwoSpecialNGroundShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMewtwoSpecialAirNShootTransition);
}

/* MewtwoNSP.air_shoot_collision_: landing from the aerial shoot state. */
static void ftRemixMewtwoSpecialAirNShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMewtwoSpecialNGroundShootTransition);
}

/* Charge loop shared by Samus's copy (0x8015D5AC) and Kirby's copy (0x80157114): counts the
 * charge down, raises the level and, at max, sets the charge gfx and waits. Both copies
 * use the same charge_gobj offset (0x0B20) and the Shadow Ball's level (0x2A4). */
static void ftRemixMewtwoChargeLoopStep(GObj *fighter_gobj, s32 *charge_int, s32 *charge_level, s32 colanim_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    (*charge_int)--;
    if (*charge_int != 0)
    {
        return;
    }
    *charge_int = FTSAMUS_CHARGE_INT;

    if (*charge_level > (FTSAMUS_CHARGE_MAX - 1))
    {
        return;
    }
    (*charge_level)++;

    if (*charge_level == FTSAMUS_CHARGE_MAX)
    {
        ftParamCheckSetFighterColAnimID(fighter_gobj, colanim_id, 0);
        ftSamusSpecialNDestroyChargeShot(fp);
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
    else if (fp->status_vars.samus.specialn.charge_gobj != NULL)
    {
        MewtwoShadowBallVars *sb = ftRemixMewtwoShadowBallGetVars(fp->status_vars.samus.specialn.charge_gobj);

        sb->charge_level = *charge_level;
    }
}

/* MewtwoNSP.charge_main_: Samus's charge loop, with Mewtwo's charge gfx and the Shadow Ball's charge level. Based on 0x8015D5AC. */
static void ftRemixMewtwoSpecialNChargeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftSamusSpecialNSetChargeShotPosition(fp);
    ftRemixMewtwoChargeLoopStep(fighter_gobj, &fp->status_vars.samus.specialn.charge_int, &fp->passive_vars.samus.charge_level, MTWO_SHADOW_BALL_CHARGE_COLANIM);
}

/* MewtwoNSP.kirby_charge_main_: Kirby's charge loop (0x80157114) with Kirby's charge level and Mewtwo's charge gfx 0x64. */
static void ftRemixMewtwoKirbyNChargeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftSamusSpecialNSetChargeShotPosition(fp);
    ftRemixMewtwoChargeLoopStep(fighter_gobj, &fp->status_vars.kirby.copysamus_specialn.charge_int, &fp->passive_vars.kirby.copysamus_charge_level, MTWO_KIRBY_SHADOW_BALL_CHARGE_COLANIM);
}

/* MewtwoNSP.ground_charge_interrupt_: B or A shoots, shield escapes, Z returns to wait. Based on 0x8015D640. */
static void ftRemixMewtwoSpecialNGroundChargeProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id;

    if ((fp->input.pl.button_tap & fp->input.button_mask_b) || (fp->input.pl.button_tap & fp->input.button_mask_a))
    {
        ftRemixMewtwoSpecialNGroundShootInitial(fighter_gobj);
        return;
    }
    status_id = ftCommonEscapeGetStatus(fp);

    if (status_id != -1)
    {
        ftSamusSpecialNDestroyChargeShot(fp);
        ftCommonEscapeSetStatus(fighter_gobj, status_id, 0);
    }
    else if (fp->input.pl.button_tap & fp->input.button_mask_z)
    {
        ftSamusSpecialNDestroyChargeShot(fp);
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

/* MewtwoNSP.air_charge_interrupt_: B or A shoots, shield drops the ball and falls. */
static void ftRemixMewtwoSpecialAirNChargeProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->input.pl.button_tap & fp->input.button_mask_b) || (fp->input.pl.button_tap & fp->input.button_mask_a))
    {
        ftRemixMewtwoSpecialAirNShootInitial(fighter_gobj);
        return;
    }
    if (fp->input.pl.button_tap & fp->input.button_mask_z)
    {
        ftSamusSpecialNDestroyChargeShot(fp);
        ftCommonFallSetStatus(fighter_gobj);
    }
}

/* MewtwoNSP.shoot_main_: updates the ball position and runs Samus's shot end. Based on 0x8015D7AC. */
static void ftRemixMewtwoSpecialNShootProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftSamusSpecialNSetChargeShotPosition(fp);
    ftSamusSpecialNEndProcUpdate(fighter_gobj);
}

/* MewtwoNSP.kirby_shoot_main_: updates the ball position and runs Kirby's copy shot end (0x80157314). */
static void ftRemixMewtwoKirbyNShootProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftSamusSpecialNSetChargeShotPosition(fp);
    ftKirbyCopySamusSpecialNEndProcUpdate(fighter_gobj);
}

/* MewtwoNSP.kirby_ground_begin_collision_: Kirby's grounded begin slides off a ledge
 * rather than stopping on the edge (0x800DDDDC, not 0x800DDE84). */
static void ftRemixMewtwoKirbyNGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixMewtwoSpecialAirNBeginTransition);
}

/* MewtwoNSP.kirby_ground_charge_collision_: Kirby's grounded charge slides off a ledge. */
static void ftRemixMewtwoKirbyNGroundChargeProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixMewtwoSpecialAirNChargeTransition);
}

/* MewtwoNSP.kirby_ground_shoot_collision_: Kirby's grounded shoot slides off a ledge. */
static void ftRemixMewtwoKirbyNGroundShootProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixMewtwoSpecialAirNShootTransition);
}

/* Remix address -> port routine for every Mewtwo NSP routine its action array or
 * special tables reference (Shadow Ball). */
const FTRemixProc dFTRemixMewtwoNSPProcs[] =
{
    { REMIX_SYM_MewtwoNSP__begin_main_, ftRemixMewtwoSpecialNBeginProcUpdate },
    { REMIX_SYM_MewtwoNSP__ground_begin_initial_, ftRemixMewtwoSpecialNGroundBeginInitial },
    { REMIX_SYM_MewtwoNSP__air_begin_initial_, ftRemixMewtwoSpecialAirNBeginInitial },
    { REMIX_SYM_MewtwoNSP__ground_begin_collision_, ftRemixMewtwoSpecialNGroundBeginProcMap },
    { REMIX_SYM_MewtwoNSP__air_begin_collision_, ftRemixMewtwoSpecialAirNBeginProcMap },
    { REMIX_SYM_MewtwoNSP__charge_main_, ftRemixMewtwoSpecialNChargeProcUpdate },
    { REMIX_SYM_MewtwoNSP__ground_charge_interrupt_, ftRemixMewtwoSpecialNGroundChargeProcInterrupt },
    { REMIX_SYM_MewtwoNSP__air_charge_interrupt_, ftRemixMewtwoSpecialAirNChargeProcInterrupt },
    { REMIX_SYM_MewtwoNSP__ground_charge_collision_, ftRemixMewtwoSpecialNGroundChargeProcMap },
    { REMIX_SYM_MewtwoNSP__air_charge_collision_, ftRemixMewtwoSpecialAirNChargeProcMap },
    { REMIX_SYM_MewtwoNSP__shoot_main_, ftRemixMewtwoSpecialNShootProcUpdate },
    { REMIX_SYM_MewtwoNSP__ground_shoot_collision_, ftRemixMewtwoSpecialNGroundShootProcMap },
    { REMIX_SYM_MewtwoNSP__air_shoot_collision_, ftRemixMewtwoSpecialAirNShootProcMap },
    { REMIX_SYM_MewtwoNSP__kirby_ground_begin_collision_, ftRemixMewtwoKirbyNGroundBeginProcMap },
    { REMIX_SYM_MewtwoNSP__kirby_charge_main_, ftRemixMewtwoKirbyNChargeProcUpdate },
    { REMIX_SYM_MewtwoNSP__kirby_ground_charge_collision_, ftRemixMewtwoKirbyNGroundChargeProcMap },
    { REMIX_SYM_MewtwoNSP__kirby_shoot_main_, ftRemixMewtwoKirbyNShootProcUpdate },
    { REMIX_SYM_MewtwoNSP__kirby_ground_shoot_collision_, ftRemixMewtwoKirbyNGroundShootProcMap }
};

const s32 dFTRemixMewtwoNSPProcsNum = ARRAY_COUNT(dFTRemixMewtwoNSPProcs);

/* MewtwoNSP.ball_position_fix_ (0x8015D35C in ftSamusSpecialNGetChargeShotPosition):
 * Mewtwo holds the ball at its grab joint (0x0978, joints[36]) with no offset.
 * Returns TRUE when `pos` was written. */
sb32 ftRemixMewtwoSpecialNGetBallPosition(FTStruct *fp, Vec3f *pos)
{
    if (fp->fkind != REMIX_ID_MTWO)
    {
        return FALSE;
    }
    pos->x = pos->y = pos->z = 0.0F;

    gmCollisionGetFighterPartsWorldPosition(fp->joints[36], pos);

    return TRUE;
}

/* MewtwoNSP.shoot_velocity_patch_ (0x8015D878 in ftSamusSpecialNEndProcUpdate): the
 * aerial recoil base becomes 10 + 5 * charge level and the y recoil step -5 instead
 * of -10. Writes the two constants for Mewtwo and leaves them alone otherwise. */
void ftRemixMewtwoSpecialNGetShootRecoil(FTStruct *fp, f32 *recoil_base, f32 *recoil_y_step)
{
    if (fp->fkind != REMIX_ID_MTWO)
    {
        return;
    }
    *recoil_base += 5.0F * fp->passive_vars.samus.charge_level; /* SHOOT_X_SPEED 0x40A0 */
    *recoil_y_step = -5.0F;                                    /* 0xC0A0 */
}

#endif
