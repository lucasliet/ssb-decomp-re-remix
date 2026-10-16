/**
 * ftremixdededensp.c - C ports of Smash Remix Dedede neutral special (Inhale)
 * routines (.toolchain/pkgs/smashremix-src/src/Dedede/DededeSpecial.asm,
 * scope DededeNSP).
 *
 * Kirby wearing the Dedede hat runs the same routines with Kirby's own status
 * ids (DEDEDE_NSP_* in Kirby.asm); ftRemixDededeNSPStatus picks the pair by fkind.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftchar/ftkirby/ftkirbyfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <it/itcommon/itstarrod.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include <sys/utils.h>

enum
{
    nFTRemixDededeStatusNSPBeginGround = 0xE7,
    nFTRemixDededeStatusNSPLoopGround = 0xE8,
    nFTRemixDededeStatusNSPIdleGround = 0xEC,
    nFTRemixDededeStatusNSPSpitGround = 0xED,
    nFTRemixDededeStatusNSPBeginAir = 0xF0,
    nFTRemixDededeStatusNSPLoopAir = 0xF1,
    nFTRemixDededeStatusNSPFall = 0xF4,
    nFTRemixDededeStatusNSPSpitAir = 0xF5,
    nFTRemixDededeStatusNSPWalk1 = 0xFF
};

/* Kirby ids are 0xDC + index in the Remix KIRBY_action_array, at the entries whose routines are DededeNSP's. */
enum
{
    nFTRemixDededeKirbyStatusNSPBeginGround = 0x16C,
    nFTRemixDededeKirbyStatusNSPLoopGround = 0x16D,
    nFTRemixDededeKirbyStatusNSPIdleGround = 0x170,
    nFTRemixDededeKirbyStatusNSPSpitGround = 0x171,
    nFTRemixDededeKirbyStatusNSPBeginAir = 0x174,
    nFTRemixDededeKirbyStatusNSPLoopAir = 0x175,
    nFTRemixDededeKirbyStatusNSPFall = 0x178,
    nFTRemixDededeKirbyStatusNSPSpitAir = 0x179,
    nFTRemixDededeKirbyStatusNSPWalk1 = 0x17C
};

#define DEDEDE_NSP_WALK_STATUS_BASE 0x0B                /* ftCommonWalkGetWalkStatus returns 0xB..0xD for walk 1..3 */
#define DEDEDE_NSP_SPIT_DAMAGE 16
#define DEDEDE_NSP_SPIT_VELOCITY 240.0F                 /* 0x43700000: spat players' initial speed */
#define DEDEDE_NSP_SPAT_DECELERATION 10.0F              /* 0x41200000 */
#define DEDEDE_NSP_SPIT_JOINT 16                        /* FTStruct.joints[16] at N64 0x0928 */
#define DEDEDE_NSP_SPIT_CAPTURE_IMMUNE_MASK 0x3F        /* asm literal 0x3F */
#define DEDEDE_NSP_ABSORB_FLAGS 0x0006                  /* asm absorb_initial_ argument 4 */
#define DEDEDE_NSP_SPIT_FLAGS (FTSTATUS_PRESERVE_TEXTUREPART | FTSTATUS_PRESERVE_MODELPART | FTSTATUS_PRESERVE_EFFECT) /* 0xA4 */
#define DEDEDE_NSP_LOOP_FLAGS (FTSTATUS_PRESERVE_LOOPSFX | FTSTATUS_PRESERVE_MODELPART | FTSTATUS_PRESERVE_EFFECT)     /* 0x824 */
#define DEDEDE_NSP_INHALE_FLAGS (FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_RUMBLE | FTSTATUS_PRESERVE_LOOPSFX | FTSTATUS_PRESERVE_MODELPART | FTSTATUS_PRESERVE_EFFECT) /* 0x4825 */

/* DededeNSP.ground_walk_initial_2_: all three walk speeds are 1.0F. */
static const f32 sFTRemixDededeNSPWalkSpeed[] =
{
    1.0F,
    1.0F,
    1.0F
};

static void ftRemixDededeNSPAbsorbSetup(FTStruct *fp);

/* Dedede's status id, or Kirby's when the fighter is Kirby wearing the Dedede hat. */
static s32 ftRemixDededeNSPStatus(FTStruct *fp, s32 dedede_status, s32 kirby_status)
{
    if (fp->fkind == REMIX_ID_DEDEDE)
    {
        return dedede_status;
    }
    return kirby_status;
}

/* Shared body of DededeNSP.ground_begin_initial_ and air_begin_initial_. */
static void ftRemixDededeNSPBeginInitial(GObj *fighter_gobj, s32 dedede_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixDededeNSPStatus(fp, dedede_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftKirbySpecialNInitStatusVars(fighter_gobj, FALSE);
    ftKirbySpecialNGotoSetCatchParams(fighter_gobj);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->physics.vel_ground.x = 0.0F;
    fp->physics.vel_ground.y = 0.0F;
}

/* DededeNSP.ground_begin_initial_ */
static void ftRemixDededeNSPGroundBeginInitial(GObj *fighter_gobj)
{
    ftRemixDededeNSPBeginInitial(fighter_gobj, nFTRemixDededeStatusNSPBeginGround, nFTRemixDededeKirbyStatusNSPBeginGround);
}

/* DededeNSP.air_begin_initial_ */
static void ftRemixDededeNSPAirBeginInitial(GObj *fighter_gobj)
{
    ftRemixDededeNSPBeginInitial(fighter_gobj, nFTRemixDededeStatusNSPBeginAir, nFTRemixDededeKirbyStatusNSPBeginAir);
}

/* Shared body of DededeNSP.ground_loop_initial_ and air_loop_initial_. */
static void ftRemixDededeNSPLoopInitial(GObj *fighter_gobj, s32 dedede_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixDededeNSPStatus(fp, dedede_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, DEDEDE_NSP_LOOP_FLAGS);
    ftKirbySpecialNGotoSetCatchParams(fighter_gobj);
    ftRemixDededeNSPAbsorbSetup(ftGetStruct(fighter_gobj));
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* DededeNSP.ground_loop_initial_ (run by ground_begin_main_ when the animation ends) */
static void ftRemixDededeNSPGroundLoopInitial(GObj *fighter_gobj)
{
    ftRemixDededeNSPLoopInitial(fighter_gobj, nFTRemixDededeStatusNSPLoopGround, nFTRemixDededeKirbyStatusNSPLoopGround);
}

/* DededeNSP.air_loop_initial_ (run by air_begin_main_ when the animation ends) */
static void ftRemixDededeNSPAirLoopInitial(GObj *fighter_gobj)
{
    ftRemixDededeNSPLoopInitial(fighter_gobj, nFTRemixDededeStatusNSPLoopAir, nFTRemixDededeKirbyStatusNSPLoopAir);
}

/* DededeNSP.ground_begin_main_ */
static void ftRemixDededeNSPGroundBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixDededeNSPGroundLoopInitial);
}

/* DededeNSP.air_begin_main_ */
static void ftRemixDededeNSPAirBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixDededeNSPAirLoopInitial);
}

/* Shared body of DededeNSP.inhale_loop_ground_to_air_initial_ and inhale_loop_air_to_ground_initial_, after the
 * caller has set the fighter's ground or air kinetics. */
static void ftRemixDededeNSPInhaleLoopSetStatus(GObj *fighter_gobj, s32 dedede_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixDededeNSPStatus(fp, dedede_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, 1.0F, DEDEDE_NSP_INHALE_FLAGS);
    ftKirbySpecialNSetCatchParams(fp);
    ftRemixDededeNSPAbsorbSetup(fp);
}

/* DededeNSP.inhale_loop_ground_to_air_initial_ */
static void ftRemixDededeNSPInhaleLoopGroundToAirInitial(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftRemixDededeNSPInhaleLoopSetStatus(fighter_gobj, nFTRemixDededeStatusNSPLoopAir, nFTRemixDededeKirbyStatusNSPLoopAir);
}

/* DededeNSP.inhale_loop_air_to_ground_initial_ */
static void ftRemixDededeNSPInhaleLoopAirToGroundInitial(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftRemixDededeNSPInhaleLoopSetStatus(fighter_gobj, nFTRemixDededeStatusNSPLoopGround, nFTRemixDededeKirbyStatusNSPLoopGround);
}

/* DededeNSP.inhale_loop_ground_to_air_check_ */
static void ftRemixDededeNSPInhaleLoopGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDededeNSPInhaleLoopGroundToAirInitial);
}

/* DededeNSP.inhale_loop_air_to_ground_check_ */
static void ftRemixDededeNSPInhaleLoopAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDededeNSPInhaleLoopAirToGroundInitial);
}

/* Status id of walk 1..3 for the walk status (0xB..0xD) returned by ftCommonWalkGetWalkStatus. */
static s32 ftRemixDededeNSPWalkStatusId(FTStruct *fp, s32 walk_status)
{
    s32 walk1_status = ftRemixDededeNSPStatus(fp, nFTRemixDededeStatusNSPWalk1, nFTRemixDededeKirbyStatusNSPWalk1);

    return walk1_status + (walk_status - DEDEDE_NSP_WALK_STATUS_BASE);
}

/* DededeNSP.ground_walk_initial_2_. The asm compares the current action against a word read from
 * walk_speed_table + 0x24, which is code rather than an action id, so the walk status is always set here. */
static void ftRemixDededeNSPGroundWalkInitial(GObj *fighter_gobj, f32 start_frame)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 walk_status = ftCommonWalkGetWalkStatus(fp->input.pl.stick_range.x);
    s32 speed_index = walk_status - DEDEDE_NSP_WALK_STATUS_BASE;

    ftMainSetStatus(fighter_gobj, ftRemixDededeNSPWalkStatusId(fp, walk_status), start_frame,
        sFTRemixDededeNSPWalkSpeed[speed_index], FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Stick x as the signed byte ftCommonWalkGetWalkStatus takes, with its sign dropped (asm: abs, then sll/sra 24). */
static s8 ftRemixDededeNSPStickXMagnitude(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;

    if (stick_x < 0)
    {
        stick_x = -stick_x;
    }
    return (s8) stick_x;
}

/* DededeNSP.ground_walk_to_idle_initial_ */
static void ftRemixDededeNSPGroundWalkToIdleInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixDededeNSPStatus(fp, nFTRemixDededeStatusNSPIdleGround, nFTRemixDededeKirbyStatusNSPIdleGround);

    if (fp->ga == nMPKineticsAir)
    {
        mpCommonSetFighterGround(fp);
    }
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* DededeNSP.ground_walk_to_idle_transition_check_: returns TRUE when the fighter stopped walking. */
static sb32 ftRemixDededeNSPGroundWalkToIdleTransitionCheck(GObj *fighter_gobj)
{
    if (!ftCommonWaitCheckInputSuccess(fighter_gobj))
    {
        return FALSE;
    }
    ftRemixDededeNSPGroundWalkToIdleInitial(fighter_gobj);
    return TRUE;
}

/* DededeNSP.ground_walk_transition_check_ (from the inhaled idle, when the stick asks to walk). */
static void ftRemixDededeNSPGroundWalkTransitionCheck(GObj *fighter_gobj)
{
    if (ftCommonWalkCheckInputSuccess(fighter_gobj))
    {
        ftRemixDededeNSPGroundWalkInitial(fighter_gobj, 0.0F);
    }
}

/* DededeNSP.ground_walk_fall_initial_ (run by ground_walk_collision_ when the fighter leaves the floor) */
static void ftRemixDededeNSPGroundWalkFallInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixDededeNSPStatus(fp, nFTRemixDededeStatusNSPFall, nFTRemixDededeKirbyStatusNSPFall);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 0.0F, FTSTATUS_PRESERVE_FASTFALL);
    ftPhysicsClampAirVelXMax(fp);
}

/* DededeNSP.ground_walk_collision_ */
static void ftRemixDededeNSPGroundWalkProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDededeNSPGroundWalkFallInitial);
}

/* Shared body of DededeNSP.ground_spit_transition_ and air_spit_transition_. */
static void ftRemixDededeNSPSpitTransition(GObj *fighter_gobj, s32 dedede_status, s32 kirby_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = ftRemixDededeNSPStatus(fp, dedede_status, kirby_status);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, DEDEDE_NSP_SPIT_FLAGS);
    ftParamSetCaptureImmuneMask(fp, DEDEDE_NSP_SPIT_CAPTURE_IMMUNE_MASK);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* DededeNSP.absorb_initial_ (custom reflect routine 2): spits at once after eating a projectile, on the kinetic state of the fighter. */
void ftRemixDededeNSPAbsorbInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = (fp->ga == nMPKineticsAir) ?
        ftRemixDededeNSPStatus(fp, nFTRemixDededeStatusNSPSpitAir, nFTRemixDededeKirbyStatusNSPSpitAir) :
        ftRemixDededeNSPStatus(fp, nFTRemixDededeStatusNSPSpitGround, nFTRemixDededeKirbyStatusNSPSpitGround);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, DEDEDE_NSP_ABSORB_FLAGS);
    ftParamSetCaptureImmuneMask(fp, FTCATCHKIND_MASK_ALL);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* DededeNSP.absorb_struct: custom reflect id 2 (index into Reflect.custom_reflect_table), reflect type CUSTOM (3). */
static FTSpecialColl sFTRemixDededeNSPAbsorbColl =
{
    (0x0002 << 16) | 0x0003,
    0,
    { 0.0F, 318.0F, 250.0F },
    { 480.0F, 330.0F, 330.0F },
    0x18000000
};

/* DededeNSP.absorb_setup_: enables the reflect bit and points the reflect hitbox at the absorb range. */
static void ftRemixDededeNSPAbsorbSetup(FTStruct *fp)
{
    fp->is_reflect = TRUE;
    fp->special_coll = &sFTRemixDededeNSPAbsorbColl;
}

/* DededeNSP.ground_spit_transition_ */
static void ftRemixDededeNSPGroundSpitTransition(GObj *fighter_gobj)
{
    ftRemixDededeNSPSpitTransition(fighter_gobj, nFTRemixDededeStatusNSPSpitGround, nFTRemixDededeKirbyStatusNSPSpitGround);
}

/* DededeNSP.air_spit_transition_ */
static void ftRemixDededeNSPAirSpitTransition(GObj *fighter_gobj)
{
    ftRemixDededeNSPSpitTransition(fighter_gobj, nFTRemixDededeStatusNSPSpitAir, nFTRemixDededeKirbyStatusNSPSpitAir);
}

/* True when A or B was tapped this frame (asm: check_for_spit_, buttons from input.pl.button_tap). */
static sb32 ftRemixDededeNSPIsSpitPressed(FTStruct *fp)
{
    u16 tapped = fp->input.pl.button_tap;

    return ((tapped & fp->input.button_mask_a) != 0) || ((tapped & fp->input.button_mask_b) != 0);
}

/* DededeNSP.check_for_spit_: when A or B is tapped, hits any captured player and runs proc_spit. */
static sb32 ftRemixDededeNSPCheckForSpit(GObj *fighter_gobj, void (*proc_spit)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!ftRemixDededeNSPIsSpitPressed(fp))
    {
        return FALSE;
    }
    if (fp->catch_gobj != NULL)
    {
        ftKirbySpecialNApplyCaptureDamage(fighter_gobj, fp->catch_gobj, DEDEDE_NSP_SPIT_DAMAGE);
    }
    proc_spit(fighter_gobj);
    return TRUE;
}

/* DededeNSP.spat_deceleration_: proc_physics of a player spat out by Dedede. */
static void ftRemixDededeNSPSpatDeceleration(GObj *spat_gobj)
{
    ftCommonThrownCommonStarUpdatePhysics(spat_gobj, DEDEDE_NSP_SPAT_DECELERATION);
}

/* Launches the captured player out of Dedede's mouth (asm: _shoot_player in spit_shoot_check_). */
static void ftRemixDededeNSPSpitPlayer(GObj *fighter_gobj, FTStruct *fp)
{
    GObj *held_gobj = fp->catch_gobj;
    FTStruct *held_fp = ftGetStruct(held_gobj);

    ftCommonThrownKirbyStarSetStatus(held_gobj);
    ftParamSetThrowParams(held_fp, fighter_gobj);
    held_fp->proc_physics = ftRemixDededeNSPSpatDeceleration;
    held_fp->physics.vel_air.x = (f32) (-held_fp->lr) * DEDEDE_NSP_SPIT_VELOCITY;
    held_fp->physics.vel_air.y = 0.0F;
    held_fp->physics.vel_air.z = 0.0F;
}

/* DededeNSP.spit_shoot_check_: when temp variable 3 is set, spits the captured player or else a star. */
static void ftRemixDededeNSPSpitShootCheck(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f spit_pos = { 0.0F, 0.0F, 0.0F };

    if (fp->motion_vars.flags.flag2 == 0)
    {
        return;
    }
    fp->motion_vars.flags.flag2 = 0;
    if (fp->catch_gobj != NULL)
    {
        ftRemixDededeNSPSpitPlayer(fighter_gobj, fp);
        return;
    }
    /* The asm also unsuspends a projectile stored in last_reflected_object_table here, but Reflect.asm keeps the suspend call
     * commented out ("not suspending projectiles"), so the table stays empty and a star is always spat. */
    gmCollisionGetFighterPartsWorldPosition(fp->joints[DEDEDE_NSP_SPIT_JOINT], &spit_pos);
    /* The asm leaves the is_smash argument (a2) unset, so FALSE is an uncertain choice. */
    itStarRodWeaponStarMakeWeapon(fighter_gobj, &spit_pos, FALSE);
}

/* DededeNSP.ground_spit_main_ and air_spit_main_ */
static void ftRemixDededeNSPSpitProcUpdate(GObj *fighter_gobj)
{
    ftRemixDededeNSPSpitShootCheck(fighter_gobj);
    ftAnimEndSetFall(fighter_gobj);
}

/* DededeNSP.ground_walk_interrupt_ */
static void ftRemixDededeNSPGroundWalkProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 walk_status;

    if (ftRemixDededeNSPCheckForSpit(fighter_gobj, ftRemixDededeNSPGroundSpitTransition))
    {
        return;
    }
    if (ftRemixDededeNSPGroundWalkToIdleTransitionCheck(fighter_gobj))
    {
        return;
    }
    walk_status = ftCommonWalkGetWalkStatus(ftRemixDededeNSPStickXMagnitude(fp));
    if (fp->status_id == ftRemixDededeNSPWalkStatusId(fp, walk_status))
    {
        return;
    }
    ftRemixDededeNSPGroundWalkInitial(fighter_gobj, fighter_gobj->anim_frame);
}

/* DededeNSP.ground_idle_interrupt_ */
static void ftRemixDededeNSPGroundIdleProcInterrupt(GObj *fighter_gobj)
{
    if (ftRemixDededeNSPCheckForSpit(fighter_gobj, ftRemixDededeNSPGroundSpitTransition))
    {
        return;
    }
    if (ftKirbySpecialNTurnCheckGotoTurn(fighter_gobj, ftKirbySpecialNTurnSetStatus))
    {
        return;
    }
    ftRemixDededeNSPGroundWalkTransitionCheck(fighter_gobj);
}

/* DededeNSP.air_fall_interrupt_ */
static void ftRemixDededeNSPAirFallProcInterrupt(GObj *fighter_gobj)
{
    ftRemixDededeNSPCheckForSpit(fighter_gobj, ftRemixDededeNSPAirSpitTransition);
}

/* Remix address -> port routine for every DededeNSP routine the action array or special tables reference. */
const FTRemixProc dFTRemixDededeNSPProcs[] =
{
    { REMIX_SYM_DededeNSP__air_begin_initial_, ftRemixDededeNSPAirBeginInitial },
    { REMIX_SYM_DededeNSP__air_begin_main_, ftRemixDededeNSPAirBeginProcUpdate },
    { REMIX_SYM_DededeNSP__air_fall_interrupt_, ftRemixDededeNSPAirFallProcInterrupt },
    { REMIX_SYM_DededeNSP__air_spit_main_, ftRemixDededeNSPSpitProcUpdate },
    { REMIX_SYM_DededeNSP__ground_begin_initial_, ftRemixDededeNSPGroundBeginInitial },
    { REMIX_SYM_DededeNSP__ground_begin_main_, ftRemixDededeNSPGroundBeginProcUpdate },
    { REMIX_SYM_DededeNSP__ground_idle_interrupt_, ftRemixDededeNSPGroundIdleProcInterrupt },
    { REMIX_SYM_DededeNSP__ground_spit_main_, ftRemixDededeNSPSpitProcUpdate },
    { REMIX_SYM_DededeNSP__ground_walk_collision_, ftRemixDededeNSPGroundWalkProcMap },
    { REMIX_SYM_DededeNSP__ground_walk_interrupt_, ftRemixDededeNSPGroundWalkProcInterrupt },
    { REMIX_SYM_DededeNSP__inhale_loop_air_to_ground_check_, ftRemixDededeNSPInhaleLoopAirProcMap },
    { REMIX_SYM_DededeNSP__inhale_loop_ground_to_air_check_, ftRemixDededeNSPInhaleLoopGroundProcMap }
};

const s32 dFTRemixDededeNSPProcsNum = ARRAY_COUNT(dFTRemixDededeNSPProcs);

/*
 * Dedede's inhale reuses Kirby's vanilla SpecialN routines. DededeSpecial.asm
 * patches the 27 status changes in them (inhale_*_, *_transition_, asm 862-2409)
 * so they set Dedede's action ids instead of Kirby's. The port remaps the
 * status id once in ftMainSetStatus, which covers every one of those sites.
 * Index: vanilla id - nFTKirbyStatusSpecialNStart. 0 means no Dedede action.
 */
static const u16 sFTRemixDededeKirbyInhaleStatus[] =
{
    0xE7, /* SpecialNStart     -> NSP_BEGIN_GROUND */
    0xE8, /* SpecialNLoop      -> NSP_LOOP_GROUND */
    0xEF, /* SpecialNEnd       -> NSP_END_GROUND */
    0xEA, /* SpecialNCatch     -> NSP_PULL_GROUND */
    0xEB, /* SpecialNEat       -> NSP_SWALLOW_GROUND */
    0xED, /* SpecialNThrow     -> NSP_SPIT_GROUND */
    0xEC, /* SpecialNWait      -> NSP_IDLE_GROUND */
    0xEE, /* SpecialNTurn      -> NSP_TURN_GROUND */
    0x00, /* SpecialNCopy */
    0xF0, /* SpecialAirNStart  -> NSP_BEGIN_AIR */
    0xF1, /* SpecialAirNLoop   -> NSP_LOOP_AIR */
    0xF7, /* SpecialAirNEnd    -> NSP_END_AIR */
    0xF2, /* SpecialAirNCatch  -> NSP_PULL_AIR */
    0xF3, /* SpecialAirNEat    -> NSP_SWALLOW_AIR */
    0xF5, /* SpecialAirNThrow  -> NSP_SPIT_AIR */
    0xF4, /* SpecialAirNWait   -> NSP_FALL */
    0xF6, /* SpecialAirNTurn   -> NSP_TURN_AIR */
    0x00  /* SpecialAirNCopy */
};

s32 ftRemixDededeMapKirbyInhaleStatus(FTStruct *fp, s32 status_id)
{
    s32 index = status_id - nFTKirbyStatusSpecialNStart;

    if (fp->fkind != REMIX_ID_DEDEDE)
    {
        return status_id;
    }
    if ((index < 0) || (index >= (s32)ARRAY_COUNT(sFTRemixDededeKirbyInhaleStatus)))
    {
        return status_id;
    }
    if (sFTRemixDededeKirbyInhaleStatus[index] == 0)
    {
        return status_id;
    }
    return sFTRemixDededeKirbyInhaleStatus[index];
}

#endif
