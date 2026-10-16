/**
 * ftremixlankynsp.c - C ports of Smash Remix Lanky Kong neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Lanky/LankySpecial.asm, scope LankyNSP).
 *
 * Lanky fires a Grape Shooter grape. Moveset temp variable 1 (motion_vars.flags.flag0)
 * requests a grape, temp variable 2 (flag1) arms the B-press re-fire, and the ammo
 * counter lives in the status_vars scratch word at N64 0x0B20.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <sys/obj.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpdef.h>
#include <wp/wpmain.h>
#include <wp/wpmanager.h>
#include <wp/wpmap.h>

enum
{
    nFTRemixLankyStatusNSPGround = 0xDF,
    nFTRemixLankyStatusNSPAir = 0xE0,
    nFTRemixLankyKirbyStatusNSPGround = 0xE7,
    nFTRemixLankyKirbyStatusNSPAir = 0xE8
};

#define LANKY_NUT_PROJECTILE_ID 0x25                /* Projectile.id.LANKY_NUT (Projectile.asm) */
#define LANKY_NSP_HAND_JOINT 18                     /* FTStruct.joints[18] at N64 0x0930, the grape's hand */
#define LANKY_NSP_KIRBY_HAND_JOINT 17               /* FTStruct.joints[17] at N64 0x092C, Kirby's hand */
#define LANKY_NSP_AMMO_N64_OFFSET 0x0B20            /* status_vars scratch word: grape ammo */
#define LANKY_NSP_AMMO_START 3
#define LANKY_NSP_CANCEL_START_FRAME 14.0F          /* 0x41600000: re-fire starts at this animation frame */
#define LANKY_NSP_FGM_GRAPE_DESTRUCTION 0x5F1       /* FGM 1521 */
#define LANKY_GRAPE_TRANSFORM_TK2 0x47              /* Secondary matrix transformation, as in wpmariofireball */
#define LANKY_GRAPE_MAX_SPEED 50.0F
#define LANKY_GRAPE_GRAVITY 0.0F
#define LANKY_GRAPE_ROTATION_SPEED 0.4F
#define LANKY_GRAPE_INITIAL_ANGLE_GROUND 0.0F
#define LANKY_GRAPE_INITIAL_ANGLE_AIR 0.0F
#define LANKY_GRAPE_INITIAL_SPEED 40.0F
#define LANKY_GRAPE_PALETTE_INDEX 0.0F
#define LANKY_GRAPE_DURATION 70                    /* grape_properties_struct 0x00; N64 WPStruct 0x268 is lifetime, as wpMainDecLifeCheckExpire reads it */

extern alSoundEffect *func_800269C0_275C0(u16 sfx_id);
extern sb32 wpMarioFireballProcHop(GObj *weapon_gobj);
extern sb32 wpSamusBombProcReflector(GObj *weapon_gobj);

static void ftRemixLankyNSPGroundInitial(GObj *fighter_gobj);
static void ftRemixLankyNSPAirInitial(GObj *fighter_gobj);
static sb32 ftRemixLankyGrapeProcUpdate(GObj *grape_gobj);
static sb32 ftRemixLankyGrapeProcMap(GObj *grape_gobj);
static sb32 ftRemixLankyGrapeProcHit(GObj *grape_gobj);
static sb32 ftRemixLankyGrapeProcDestruction(GObj *grape_gobj);

/* Kirby and J Kirby use their own status ids for the same special. */
static sb32 ftRemixLankyNSPIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Returns the Kirby id for Kirby and J Kirby, otherwise the Lanky id. */
static s32 ftRemixLankyNSPStatusForFighter(FTStruct *fp, s32 lanky_status, s32 kirby_status)
{
    if (ftRemixLankyNSPIsKirby(fp))
    {
        return kirby_status;
    }
    return lanky_status;
}

/* Sets the grape special's status for the fighter and plays the anim events, as the asm does. */
static void ftRemixLankyNSPSetStatus(GObj *fighter_gobj, s32 lanky_status, s32 kirby_status, f32 start_frame)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixLankyNSPStatusForFighter(fp, lanky_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, start_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Grape ammo lives in the status_vars scratch word at N64 0x0B20. */
static s32 *ftRemixLankyNSPAmmo(FTStruct *fp)
{
    return ftRemixFighterWord(fp, LANKY_NSP_AMMO_N64_OFFSET);
}

/* Shared body of LankyNSP.ground_initial_ and air_initial_: sets the status, then clears temp variables 1 to 3
 * and resets the ammo. */
static void ftRemixLankyNSPInitialSetup(GObj *fighter_gobj, s32 lanky_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixLankyNSPSetStatus(fighter_gobj, lanky_status, kirby_status, 0.0F);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    *ftRemixLankyNSPAmmo(fp) = LANKY_NSP_AMMO_START;
}

/* LankyNSP.ground_initial_ */
static void ftRemixLankyNSPGroundInitial(GObj *fighter_gobj)
{
    ftRemixLankyNSPInitialSetup(fighter_gobj, nFTRemixLankyStatusNSPGround, nFTRemixLankyKirbyStatusNSPGround);
}

/* LankyNSP.air_initial_ */
static void ftRemixLankyNSPAirInitial(GObj *fighter_gobj)
{
    ftRemixLankyNSPInitialSetup(fighter_gobj, nFTRemixLankyStatusNSPAir, nFTRemixLankyKirbyStatusNSPAir);
}

/* Fills the grape's weapon descriptor (asm grape_projectile_struct) from the fighter's special file. */
static void ftRemixLankyNSPFillGrapeDesc(WPDesc *grape_desc, FTStruct *fp)
{
    grape_desc->flags = 0;
    grape_desc->kind = LANKY_NUT_PROJECTILE_ID;
    grape_desc->p_weapon = ftRemixPowerData(fp)->p_file_special1;
    grape_desc->o_attributes = 0;
    grape_desc->transform_types.tk1 = nGCMatrixKindTra;
    grape_desc->transform_types.tk2 = LANKY_GRAPE_TRANSFORM_TK2;
    grape_desc->transform_types.tk3 = 0;
    grape_desc->proc_update = ftRemixLankyGrapeProcUpdate;
    grape_desc->proc_map = ftRemixLankyGrapeProcMap;
    grape_desc->proc_hit = ftRemixLankyGrapeProcHit;
    grape_desc->proc_shield = ftRemixLankyGrapeProcDestruction;
    grape_desc->proc_hop = wpMarioFireballProcHop;
    grape_desc->proc_setoff = ftRemixLankyGrapeProcDestruction;
    grape_desc->proc_reflector = wpSamusBombProcReflector;
    grape_desc->proc_absorb = ftRemixLankyGrapeProcDestruction;
}

/* Sets the grape's launch velocity from its facing and the fighter's air or ground state, and its palette.
 * The asm computes x as cos(angle) * speed * lr and y as sin(angle) * speed. */
static void ftRemixLankyNSPInitGrape(GObj *grape_gobj, FTStruct *fp)
{
    WPStruct *wp = wpGetStruct(grape_gobj);
    DObj *grape_dobj = DObjGetStruct(grape_gobj);
    f32 angle = (fp->ga == nMPKineticsAir) ? LANKY_GRAPE_INITIAL_ANGLE_AIR : LANKY_GRAPE_INITIAL_ANGLE_GROUND;

    wp->lifetime = LANKY_GRAPE_DURATION;
    wp->physics.vel_air.z = 0.0F;
    wp->physics.vel_air.x = (__cosf(angle) * LANKY_GRAPE_INITIAL_SPEED) * (f32) fp->lr;
    wp->physics.vel_air.y = __sinf(angle) * LANKY_GRAPE_INITIAL_SPEED;
    grape_dobj->mobj->palette_id = LANKY_GRAPE_PALETTE_INDEX;
    wpMainVelSetModelPitch(grape_gobj);
}

/* LankyNSP.grape_stage_setting_: spawns one grape at spawn_pos from the fighter's special file. */
static void ftRemixLankyNSPStageGrape(GObj *fighter_gobj, Vec3f *spawn_pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    WPDesc grape_desc;
    GObj *grape_gobj;

    ftRemixLankyNSPFillGrapeDesc(&grape_desc, fp);
    grape_gobj = wpManagerMakeWeapon(fighter_gobj, &grape_desc, spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (grape_gobj == NULL)
    {
        return;
    }
    ftRemixLankyNSPInitGrape(grape_gobj, fp);
}

/* LankyNSP.main_ spawn path: the grape starts at the hand joint. Kirby and J Kirby use the other hand joint.
 * The asm writes fixed offsets to the vector before the call, but gmCollisionGetFighterPartsWorldPosition
 * overwrites it with the joint's world position, so those offsets never reach the grape. */
static void ftRemixLankyNSPCreateGrape(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f spawn_pos = { 0.0F, 0.0F, 0.0F };
    s32 joint_index = ftRemixLankyNSPIsKirby(fp) ? LANKY_NSP_KIRBY_HAND_JOINT : LANKY_NSP_HAND_JOINT;

    gmCollisionGetFighterPartsWorldPosition(fp->joints[joint_index], &spawn_pos);
    spawn_pos.z = 0.0F;
    ftRemixLankyNSPStageGrape(fighter_gobj, &spawn_pos);
}

/* LankyNSP.main_ cancel check: while temp variable 2 is set and B is pressed with ammo left, the special restarts
 * from the re-fire frame, on the ground or in the air. */
static void ftRemixLankyNSPCancelCheck(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *ammo = ftRemixLankyNSPAmmo(fp);
    s32 lanky_status = (fp->ga == nMPKineticsAir) ? nFTRemixLankyStatusNSPAir : nFTRemixLankyStatusNSPGround;
    s32 kirby_status = (fp->ga == nMPKineticsAir) ? nFTRemixLankyKirbyStatusNSPAir : nFTRemixLankyKirbyStatusNSPGround;

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    if (!(fp->input.pl.button_tap & B_BUTTON))
    {
        return;
    }
    if (*ammo == 0)
    {
        return;
    }
    *ammo -= 1;

    ftRemixLankyNSPSetStatus(fighter_gobj, lanky_status, kirby_status, LANKY_NSP_CANCEL_START_FRAME);
    fp->motion_vars.flags.flag1 = 0;
}

/* LankyNSP.main_ idle check: returns to idle once the animation has ended. */
static void ftRemixLankyNSPIdleCheck(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* LankyNSP.main_: releases a grape when temp variable 1 is set, handles the re-fire cancel, then idles at the end. */
static void ftRemixLankyNSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixLankyNSPCreateGrape(fighter_gobj, fp);
    }
    ftRemixLankyNSPCancelCheck(fighter_gobj);
    ftRemixLankyNSPIdleCheck(fighter_gobj);
}

/* Grape proc_update: LankyNSP.grape_main_. Returns TRUE when the grape is destroyed. */
static sb32 ftRemixLankyGrapeProcUpdate(GObj *grape_gobj)
{
    WPStruct *wp = wpGetStruct(grape_gobj);
    DObj *grape_dobj = DObjGetStruct(grape_gobj);

    if (wpMainDecLifeCheckExpire(wp))
    {
        efManagerDustExpandSmallMakeEffect(&grape_dobj->translate.vec.f, 1.0F);
        return TRUE;
    }
    wpMainApplyGravityClampTVel(wp, LANKY_GRAPE_GRAVITY, LANKY_GRAPE_MAX_SPEED);
    grape_dobj->rotate.vec.f.x += LANKY_GRAPE_ROTATION_SPEED;
    return FALSE;
}

/* Grape proc_map: LankyNSP.grape_collision_. Destroys the grape on a map collision. */
static sb32 ftRemixLankyGrapeProcMap(GObj *grape_gobj)
{
    if (!wpMapTestAllCheckCollEnd(grape_gobj))
    {
        return FALSE;
    }
    return ftRemixLankyGrapeProcDestruction(grape_gobj);
}

/* Grape proc_hit: LankyNSP.grape_hit_. Spawns a hit effect sized by the weapon's damage, then destroys the grape. */
static sb32 ftRemixLankyGrapeProcHit(GObj *grape_gobj)
{
    WPStruct *wp = wpGetStruct(grape_gobj);
    DObj *grape_dobj = DObjGetStruct(grape_gobj);

    efManagerDamageNormalLightMakeEffect(&grape_dobj->translate.vec.f, 0, wp->hit_normal_damage, FALSE);
    return ftRemixLankyGrapeProcDestruction(grape_gobj);
}

/* Grape proc for every shield, set-off and absorb, and the map exit: LankyNSP.grape_destruction_. */
static sb32 ftRemixLankyGrapeProcDestruction(GObj *grape_gobj)
{
    DObj *grape_dobj = DObjGetStruct(grape_gobj);

    efManagerDustExpandSmallMakeEffect(&grape_dobj->translate.vec.f, 1.0F);
    func_800269C0_275C0(LANKY_NSP_FGM_GRAPE_DESTRUCTION);
    return TRUE;
}

/* Remix address -> port routine for every LankyNSP routine the Lanky action array and special tables reference. */
const FTRemixProc dFTRemixLankyNSPProcs[] =
{
    { REMIX_SYM_LankyNSP__air_initial_, ftRemixLankyNSPAirInitial },
    { REMIX_SYM_LankyNSP__ground_initial_, ftRemixLankyNSPGroundInitial },
    { REMIX_SYM_LankyNSP__main_, ftRemixLankyNSPProcUpdate }
};

const s32 dFTRemixLankyNSPProcsNum = ARRAY_COUNT(dFTRemixLankyNSPProcs);

#endif
