/**
 * ftremixbanjonsp.c - C ports of Smash Remix Banjo neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Banjo/BanjoSpecial.asm, scope BanjoNSP).
 *
 * The egg weapon descriptors are built at run time because their file pointer is
 * the fighter's special file (Character.BANJO_file_9_ptr), so they cannot be static const.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <sys/obj.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpdef.h>
#include <wp/wpmain.h>
#include <wp/wpmanager.h>

enum
{
    nFTRemixBanjoStatusNSPGroundBegin = 0xE1,
    nFTRemixBanjoStatusNSPGroundForward = 0xE2,
    nFTRemixBanjoStatusNSPGroundBackward = 0xE3,
    nFTRemixBanjoStatusNSPAirBegin = 0xE4,
    nFTRemixBanjoStatusNSPAirForward = 0xE5,
    nFTRemixBanjoStatusNSPAirBackward = 0xE6
};

/* Kirby copies these specials under its own ids: 0x12F plus the index of each
 * BANJO_NSP* add_new_action call in Kirby.asm (same formula as ftremixmarth.c). */
enum
{
    nFTRemixBanjoKirbyStatusNSPGroundBegin = 0x19B,
    nFTRemixBanjoKirbyStatusNSPGroundForward = 0x19C,
    nFTRemixBanjoKirbyStatusNSPGroundBackward = 0x19D,
    nFTRemixBanjoKirbyStatusNSPAirBegin = 0x19E,
    nFTRemixBanjoKirbyStatusNSPAirForward = 0x19F,
    nFTRemixBanjoKirbyStatusNSPAirBackward = 0x1A0
};

#define BANJO_NSP_HEAD_JOINT 21                    /* FTStruct.joints[21] at N64 0x093C, Kazooie's head */
#define BANJO_NSP_STICK_DEADZONE 6
#define BANJO_NSP_FORWARD_EGG_OFFSET_X -40.0F      /* 0xC2200000 */
#define BANJO_NSP_FORWARD_EGG_KIRBY_OFFSET_Y -160.0F /* 0xC3200000; Banjo uses 0 */
#define BANJO_EGG_PROJECTILE_ID 0x21               /* Projectile.id.BANJO_EGG (Projectile.asm) */
#define BANJO_EGG_TRANSFORM_TK2 0x47               /* Secondary matrix transformation, as in wpmariofireball */
#define BANJO_EGG_HIT_FGM 0x28
#define BANJO_FORWARD_EGG_DURATION 30
#define BANJO_BACKWARD_EGG_DURATION 120

typedef struct FTRemixBanjoEggProps
{
    s32 duration;
    f32 max_speed;
    f32 gravity;
    f32 rotation_speed;
    f32 angle_ground;
    f32 angle_air;
    f32 speed;
} FTRemixBanjoEggProps;

/* Values from forward_egg_properties_struct and backward_egg_properties_struct. The bounce multiplier and
 * min speed fields are not read by the egg routines, so they are not ported. */
static const FTRemixBanjoEggProps sFTRemixBanjoForwardEggProps =
{
    BANJO_FORWARD_EGG_DURATION,
    250.0F,
    0.1F,
    0.15F,
    0.0F,
    0.0F,
    90.0F
};

/* The angles are the raw float32 128 in the asm, passed straight to cos/sin (radians). */
static const FTRemixBanjoEggProps sFTRemixBanjoBackwardEggProps =
{
    BANJO_BACKWARD_EGG_DURATION,
    250.0F,
    1.2F,
    0.1F,
    128.0F,
    128.0F,
    28.0F
};

extern sb32 itLGunWeaponAmmoProcMap(GObj *weapon_gobj);
extern sb32 wpBossBulletProcHop(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcMap(GObj *weapon_gobj);
extern sb32 wpSamusBombProcReflector(GObj *weapon_gobj);

/* Banjo keeps its own status ids; Kirby copying the special uses its own. */
static s32 ftRemixBanjoNSPStatusForFighter(FTStruct *fp, s32 banjo_status, s32 kirby_status)
{
    if (fp->fkind == REMIX_ID_BANJO)
    {
        return banjo_status;
    }
    return kirby_status;
}

/* Sets the status with FTSTATUS_PRESERVE_COLANIM, the flag the asm passes as argument 4 (value 2). */
static void ftRemixBanjoNSPSetStatusColAnim(GObj *fighter_gobj, s32 banjo_status, s32 kirby_status, f32 start_frame)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixBanjoNSPStatusForFighter(fp, banjo_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, start_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* The asm's "anim_frame <= 0" test that ends the begin, shoot and wait routines. */
static sb32 ftRemixBanjoNSPIsAnimEnded(GObj *fighter_gobj)
{
    return fighter_gobj->anim_frame <= 0.0F;
}

/* Clears temp variables 1 and 2 (motion_vars flag0 and flag1), as the shoot initial routines do. */
static void ftRemixBanjoNSPClearShootVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
}

/* Stick x past the deadzone, as the asm computes it: -1 when pointing left, 1 when pointing right, 0 when centred. */
static s32 ftRemixBanjoNSPStickDirection(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;

    if (stick_x < 0)
    {
        return ((stick_x + BANJO_NSP_STICK_DEADZONE) > 0) ? 0 : -1;
    }
    return ((stick_x - BANJO_NSP_STICK_DEADZONE) < 0) ? 0 : 1;
}

/* A centred stick counts as forward, and so does a stick that points the way Banjo faces. */
static sb32 ftRemixBanjoNSPIsShootForward(FTStruct *fp, s32 stick_dir)
{
    return (stick_dir == fp->lr) || (stick_dir == 0);
}

/* BanjoNSP.begin_initial_: sets the begin status, then plays its animation events. */
static void ftRemixBanjoNSPBeginInitial(GObj *fighter_gobj, s32 banjo_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixBanjoNSPStatusForFighter(fp, banjo_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* BanjoNSP.ground_begin_initial_ */
static void ftRemixBanjoNSPGroundBeginInitial(GObj *fighter_gobj)
{
    ftRemixBanjoNSPBeginInitial(fighter_gobj, nFTRemixBanjoStatusNSPGroundBegin, nFTRemixBanjoKirbyStatusNSPGroundBegin);
}

/* BanjoNSP.air_begin_initial_ */
static void ftRemixBanjoNSPAirBeginInitial(GObj *fighter_gobj)
{
    ftRemixBanjoNSPBeginInitial(fighter_gobj, nFTRemixBanjoStatusNSPAirBegin, nFTRemixBanjoKirbyStatusNSPAirBegin);
}

/* BanjoNSP.air_begin_transition_: moves a grounded begin to the aerial begin. */
static void ftRemixBanjoNSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftPhysicsClampAirVelXMax(fp);
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, nFTRemixBanjoStatusNSPAirBegin, nFTRemixBanjoKirbyStatusNSPAirBegin, fighter_gobj->anim_frame);
}

/* BanjoNSP.ground_begin_transition_: moves an aerial begin to the grounded begin. */
static void ftRemixBanjoNSPGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, nFTRemixBanjoStatusNSPGroundBegin, nFTRemixBanjoKirbyStatusNSPGroundBegin, fighter_gobj->anim_frame);
}

/* BanjoNSP.air_shoot_forward_transition_ */
static void ftRemixBanjoNSPAirShootForwardTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, nFTRemixBanjoStatusNSPAirForward, nFTRemixBanjoKirbyStatusNSPAirForward, fighter_gobj->anim_frame);
}

/* BanjoNSP.air_shoot_backward_transition_ */
static void ftRemixBanjoNSPAirShootBackwardTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, nFTRemixBanjoStatusNSPAirBackward, nFTRemixBanjoKirbyStatusNSPAirBackward, fighter_gobj->anim_frame);
}

/* BanjoNSP.ground_shoot_forward_transition_ */
static void ftRemixBanjoNSPGroundShootForwardTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, nFTRemixBanjoStatusNSPGroundForward, nFTRemixBanjoKirbyStatusNSPGroundForward, fighter_gobj->anim_frame);
}

/* BanjoNSP.ground_shoot_backward_transition_ */
static void ftRemixBanjoNSPGroundShootBackwardTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, nFTRemixBanjoStatusNSPGroundBackward, nFTRemixBanjoKirbyStatusNSPGroundBackward, fighter_gobj->anim_frame);
}

/* Ground shoot initial (asm copy of 0x8015DA60): no state change, the status starts at the current frame. */
static void ftRemixBanjoNSPGroundShootInitial(GObj *fighter_gobj, s32 banjo_status, s32 kirby_status)
{
    ftRemixBanjoNSPClearShootVars(ftGetStruct(fighter_gobj));
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, banjo_status, kirby_status, fighter_gobj->anim_frame);
}

/* Air shoot initial (asm copy of 0x8015DAA8): a grounded fighter becomes aerial first, then the status starts at frame 0. */
static void ftRemixBanjoNSPAirShootInitial(GObj *fighter_gobj, s32 banjo_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
        ftPhysicsClampAirVelXMax(fp);
    }
    ftRemixBanjoNSPClearShootVars(fp);
    ftRemixBanjoNSPSetStatusColAnim(fighter_gobj, banjo_status, kirby_status, 0.0F);
}

/* BanjoNSP.begin_main_: once the begin animation ends, shoots forward or backward on the ground or in the air. */
static void ftRemixBanjoNSPBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_forward;

    if (!ftRemixBanjoNSPIsAnimEnded(fighter_gobj))
    {
        return;
    }
    is_forward = ftRemixBanjoNSPIsShootForward(fp, ftRemixBanjoNSPStickDirection(fp));
    if (fp->ga == nMPKineticsGround)
    {
        if (is_forward)
        {
            ftRemixBanjoNSPGroundShootInitial(fighter_gobj, nFTRemixBanjoStatusNSPGroundForward, nFTRemixBanjoKirbyStatusNSPGroundForward);
            return;
        }
        ftRemixBanjoNSPGroundShootInitial(fighter_gobj, nFTRemixBanjoStatusNSPGroundBackward, nFTRemixBanjoKirbyStatusNSPGroundBackward);
        return;
    }
    if (is_forward)
    {
        ftRemixBanjoNSPAirShootInitial(fighter_gobj, nFTRemixBanjoStatusNSPAirForward, nFTRemixBanjoKirbyStatusNSPAirForward);
        return;
    }
    ftRemixBanjoNSPAirShootInitial(fighter_gobj, nFTRemixBanjoStatusNSPAirBackward, nFTRemixBanjoKirbyStatusNSPAirBackward);
}

/* BanjoNSP.ground_begin_collision_ */
static void ftRemixBanjoNSPGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixBanjoNSPAirBeginTransition);
}

/* BanjoNSP.air_begin_collision_ */
static void ftRemixBanjoNSPAirBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixBanjoNSPGroundBeginTransition);
}

/* BanjoNSP.ground_shoot_forward_collision_ */
static void ftRemixBanjoNSPGroundShootForwardProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixBanjoNSPAirShootForwardTransition);
}

/* BanjoNSP.ground_shoot_backward_collision_ */
static void ftRemixBanjoNSPGroundShootBackwardProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixBanjoNSPAirShootBackwardTransition);
}

/* BanjoNSP.air_shoot_forward_collision_ (landing that allows a ledge grab, vanilla 0x800DE80C) */
static void ftRemixBanjoNSPAirShootForwardProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixBanjoNSPGroundShootForwardTransition);
}

/* BanjoNSP.air_shoot_backward_collision_ (landing that allows a ledge grab, vanilla 0x800DE80C) */
static void ftRemixBanjoNSPAirShootBackwardProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixBanjoNSPGroundShootBackwardTransition);
}

/* BanjoNSP.air_shoot_physics_: drift while temp variable 2 is set, otherwise the physics give no player control.
 * The asm's addiu 0x90E0/0x91EC sign-extends to the vanilla 0x800D90E0 (drift) and 0x800D91EC (friction). */
static void ftRemixBanjoNSPAirShootProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
        return;
    }
    ftPhysicsApplyAirVelFriction(fighter_gobj);
}

/* End of shoot_forward_main_ and shoot_backward_main_: at animation end, clears temp variable 1 and returns to
 * wait or fall. */
static void ftRemixBanjoNSPShootEndCheck(GObj *fighter_gobj, FTStruct *fp)
{
    if (!ftRemixBanjoNSPIsAnimEnded(fighter_gobj))
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* Forward egg (BanjoNSP.forward_egg_main_) and backward egg (BanjoNSP.backward_egg_main_) per-frame update.
 * Returns TRUE when the egg expires. */
static sb32 ftRemixBanjoNSPEggUpdate(GObj *egg_gobj, const FTRemixBanjoEggProps *props)
{
    WPStruct *wp = wpGetStruct(egg_gobj);
    DObj *egg_dobj = DObjGetStruct(egg_gobj);

    if (wpMainDecLifeCheckExpire(wp))
    {
        efManagerDustExpandSmallMakeEffect(&egg_dobj->translate.vec.f, 1.0F);
        return TRUE;
    }
    wpMainApplyGravityClampTVel(wp, props->gravity, props->max_speed);
    egg_dobj->rotate.vec.f.x += props->rotation_speed;
    return FALSE;
}

/* BanjoNSP.forward_egg_main_ */
static sb32 ftRemixBanjoNSPForwardEggProcUpdate(GObj *egg_gobj)
{
    return ftRemixBanjoNSPEggUpdate(egg_gobj, &sFTRemixBanjoForwardEggProps);
}

/* BanjoNSP.backward_egg_main_ */
static sb32 ftRemixBanjoNSPBackwardEggProcUpdate(GObj *egg_gobj)
{
    return ftRemixBanjoNSPEggUpdate(egg_gobj, &sFTRemixBanjoBackwardEggProps);
}

/* BanjoNSP.egg_destruction_: removes the egg with a smoke effect (hurtbox, shield, set-off and absorb procs). */
static sb32 ftRemixBanjoNSPEggDestruction(GObj *egg_gobj)
{
    DObj *egg_dobj = DObjGetStruct(egg_gobj);

    efManagerDustExpandSmallMakeEffect(&egg_dobj->translate.vec.f, 1.0F);
    return TRUE;
}

/* Fills an egg weapon descriptor from the fighter's special file (asm forward/backward_egg_projectile_struct). */
static void ftRemixBanjoNSPFillEggDesc(WPDesc *egg_desc, FTStruct *fp, sb32 (*proc_update)(GObj *), sb32 (*proc_map)(GObj *))
{
    egg_desc->flags = 0;
    egg_desc->kind = BANJO_EGG_PROJECTILE_ID;
    egg_desc->p_weapon = fp->data->p_file_special4;
    egg_desc->o_attributes = 0;
    egg_desc->transform_types.tk1 = nGCMatrixKindTra;
    egg_desc->transform_types.tk2 = BANJO_EGG_TRANSFORM_TK2;
    egg_desc->transform_types.tk3 = 0;
    egg_desc->proc_update = proc_update;
    egg_desc->proc_map = proc_map;
    egg_desc->proc_hit = ftRemixBanjoNSPEggDestruction;
    egg_desc->proc_shield = ftRemixBanjoNSPEggDestruction;
    egg_desc->proc_hop = wpBossBulletProcHop;
    egg_desc->proc_setoff = ftRemixBanjoNSPEggDestruction;
    egg_desc->proc_reflector = wpSamusBombProcReflector;
    egg_desc->proc_absorb = ftRemixBanjoNSPEggDestruction;
}

/* Sets the egg's hit sound, lifetime, trajectory and palette from its properties (asm egg_stage_setting_ tail). */
static void ftRemixBanjoNSPInitEgg(GObj *egg_gobj, FTStruct *fp, const FTRemixBanjoEggProps *props)
{
    WPStruct *wp = wpGetStruct(egg_gobj);
    f32 angle = (fp->ga == nMPKineticsAir) ? props->angle_air : props->angle_ground;

    /* The asm stores the duration at N64 WPStruct 0x268, but the decomp names 0x264 as lifetime, the field that
     * wpMainDecLifeCheckExpire counts down. Writing lifetime follows the expiry check; see the notes file. */
    wp->lifetime = props->duration;
    wp->weapon_vars.charge_shot.is_full_charge = FALSE; /* N64 0x29C, cleared as the asm does */
    wp->attack_coll.element = 0;
    wp->attack_coll.fgm_id = BANJO_EGG_HIT_FGM;
    wp->physics.vel_air.x = __cosf(angle) * props->speed * (f32) fp->lr;
    wp->physics.vel_air.y = __sinf(angle) * props->speed;
    wp->physics.vel_air.z = 0.0F;
    DObjGetStruct(egg_gobj)->mobj->palette_id = 0.0F;
    wpMainVelSetModelPitch(egg_gobj);
}

/* Creates an egg at spawn_pos, then initialises it when the weapon slot was free (asm egg_stage_setting_ and
 * egg_poop_stage_setting_). */
static void ftRemixBanjoNSPCreateEgg(GObj *fighter_gobj, FTStruct *fp, Vec3f *spawn_pos, WPDesc *egg_desc, const FTRemixBanjoEggProps *props)
{
    GObj *egg_gobj = wpManagerMakeWeapon(fighter_gobj, egg_desc, spawn_pos, WEAPON_FLAG_COLLPROJECT);

    if (egg_gobj == NULL)
    {
        return;
    }
    ftRemixBanjoNSPInitEgg(egg_gobj, fp, props);
}

/* Forward egg spawn: starts at Kazooie's head, offset left by 40 units, and lower by 160 units for non-Banjo fighters. */
static void ftRemixBanjoNSPSpawnForwardEgg(GObj *fighter_gobj, FTStruct *fp)
{
    WPDesc egg_desc;
    Vec3f spawn_pos = { BANJO_NSP_FORWARD_EGG_OFFSET_X, 0.0F, 0.0F };

    if (fp->fkind != REMIX_ID_BANJO)
    {
        spawn_pos.y = BANJO_NSP_FORWARD_EGG_KIRBY_OFFSET_Y;
    }
    gmCollisionGetFighterPartsWorldPosition(fp->joints[BANJO_NSP_HEAD_JOINT], &spawn_pos);
    spawn_pos.z = 0.0F;
    ftRemixBanjoNSPFillEggDesc(&egg_desc, fp, ftRemixBanjoNSPForwardEggProcUpdate, itLGunWeaponAmmoProcMap);
    ftRemixBanjoNSPCreateEgg(fighter_gobj, fp, &spawn_pos, &egg_desc, &sFTRemixBanjoForwardEggProps);
}

/* Backward egg spawn: starts at Kazooie's head with no offset. */
static void ftRemixBanjoNSPSpawnBackwardEgg(GObj *fighter_gobj, FTStruct *fp)
{
    WPDesc egg_desc;
    Vec3f spawn_pos = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[BANJO_NSP_HEAD_JOINT], &spawn_pos);
    spawn_pos.z = 0.0F;
    ftRemixBanjoNSPFillEggDesc(&egg_desc, fp, ftRemixBanjoNSPBackwardEggProcUpdate, wpMarioFireballProcMap);
    ftRemixBanjoNSPCreateEgg(fighter_gobj, fp, &spawn_pos, &egg_desc, &sFTRemixBanjoBackwardEggProps);
}

/* BanjoNSP.shoot_forward_main_: spawns the egg when temp variable 1 is set, then ends the move at animation end. */
static void ftRemixBanjoNSPShootForwardProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixBanjoNSPSpawnForwardEgg(fighter_gobj, fp);
    }
    ftRemixBanjoNSPShootEndCheck(fighter_gobj, fp);
}

/* BanjoNSP.shoot_backward_main_: spawns the bouncing egg when temp variable 1 is set, then ends the move at animation end. */
static void ftRemixBanjoNSPShootBackwardProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixBanjoNSPSpawnBackwardEgg(fighter_gobj, fp);
    }
    ftRemixBanjoNSPShootEndCheck(fighter_gobj, fp);
}

/* Remix address -> port routine for every BanjoNSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixBanjoNSPProcs[] =
{
    { REMIX_SYM_BanjoNSP__air_begin_collision_, ftRemixBanjoNSPAirBeginProcMap },
    { REMIX_SYM_BanjoNSP__air_begin_initial_, ftRemixBanjoNSPAirBeginInitial },
    { REMIX_SYM_BanjoNSP__air_shoot_backward_collision_, ftRemixBanjoNSPAirShootBackwardProcMap },
    { REMIX_SYM_BanjoNSP__air_shoot_forward_collision_, ftRemixBanjoNSPAirShootForwardProcMap },
    { REMIX_SYM_BanjoNSP__air_shoot_physics_, ftRemixBanjoNSPAirShootProcPhysics },
    { REMIX_SYM_BanjoNSP__begin_main_, ftRemixBanjoNSPBeginProcUpdate },
    { REMIX_SYM_BanjoNSP__ground_begin_collision_, ftRemixBanjoNSPGroundBeginProcMap },
    { REMIX_SYM_BanjoNSP__ground_begin_initial_, ftRemixBanjoNSPGroundBeginInitial },
    { REMIX_SYM_BanjoNSP__ground_shoot_backward_collision_, ftRemixBanjoNSPGroundShootBackwardProcMap },
    { REMIX_SYM_BanjoNSP__ground_shoot_forward_collision_, ftRemixBanjoNSPGroundShootForwardProcMap },
    { REMIX_SYM_BanjoNSP__shoot_backward_main_, ftRemixBanjoNSPShootBackwardProcUpdate },
    { REMIX_SYM_BanjoNSP__shoot_forward_main_, ftRemixBanjoNSPShootForwardProcUpdate }
};

const s32 dFTRemixBanjoNSPProcsNum = ARRAY_COUNT(dFTRemixBanjoNSPProcs);

#endif
