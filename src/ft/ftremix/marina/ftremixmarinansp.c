/**
 * ftremixmarinansp.c - C ports of Smash Remix Marina's grab (NSP) and Cargo
 * special routines (.toolchain/pkgs/smashremix-src/src/Marina/MarinaSpecial.asm,
 * scopes MarinaNSP and MarinaCargo).
 *
 * Temp variables: flag0 (0x17C) is the damage to apply / throw-turn gate,
 * flag1 (0x180) is the Cargo B-press flag, flag2 (0x184) is the NSP movement
 * state and the Cargo loop flag.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_symbols.h"
#include "ft/ftchar/ftcaptain/ftcaptain.h"
#include "ft/ftchar/ftdonkey/ftdonkeyfunctions.h"
#include "ft/ftchar/ftyoshi/ftyoshifunctions.h"
#include "ft/ftcommon/ftcommonfunctions.h"
#include "ft/ftmain.h"
#include "ft/ftparam.h"
#include "ft/ftphysics.h"
#include "gm/gmcollision.h"
#include "it/itdef.h"
#include "it/itmanager.h"
#include "mp/map.h"
#include "mp/mpcommon.h"
#include "mp/mpdef.h"

enum
{
    nFTRemixMarinaStatusNSPGround = 0xE1,
    nFTRemixMarinaStatusNSPGroundPull = 0xE2,
    nFTRemixMarinaStatusNSPGroundThrow = 0xE3,
    nFTRemixMarinaStatusNSPGroundThrowU = 0xE4,
    nFTRemixMarinaStatusNSPGroundThrowD = 0xE5,
    nFTRemixMarinaStatusNSPAir = 0xE6,
    nFTRemixMarinaStatusNSPAirPull = 0xE7,
    nFTRemixMarinaStatusNSPAirThrow = 0xE8,
    nFTRemixMarinaStatusNSPAirThrowU = 0xE9,
    nFTRemixMarinaStatusNSPAirThrowD = 0xEA
};

enum
{
    nFTRemixMarinaStatusCargoJump = 0xFA,
    nFTRemixMarinaStatusCargoShake = 0xFB
};

/* Kirby and J Kirby action ids of the copied NSP, consecutive in the Kirby
 * action array (Kirby.asm add_new_action order; derived from remix.z64). */
enum
{
    nFTRemixKirbyStatusMarinaNSPG = 0x17F,
    nFTRemixKirbyStatusMarinaNSPGPull = 0x180,
    nFTRemixKirbyStatusMarinaNSPGThrow = 0x181,
    nFTRemixKirbyStatusMarinaNSPGThrowU = 0x182,
    nFTRemixKirbyStatusMarinaNSPGThrowD = 0x183,
    nFTRemixKirbyStatusMarinaNSPA = 0x184,
    nFTRemixKirbyStatusMarinaNSPAPull = 0x185,
    nFTRemixKirbyStatusMarinaNSPAThrow = 0x186,
    nFTRemixKirbyStatusMarinaNSPAThrowU = 0x187,
    nFTRemixKirbyStatusMarinaNSPAThrowD = 0x188
};

enum
{
    MARINA_NSP_BEGIN_MOVE = 1,
    MARINA_NSP_MOVE = 2,
    MARINA_NSP_END_MOVE = 3,
    MARINA_NSP_END = 4
};

enum
{
    MARINA_NSP_THROW_VARIANT_NORMAL = 0,
    MARINA_NSP_THROW_VARIANT_U = 1,
    MARINA_NSP_THROW_VARIANT_D = 2
};

#define MARINA_NSP_GROUND_X_SPEED 60.0F         /* 0x42700000 */
#define MARINA_NSP_AIR_X_SPEED 48.0F            /* 0x42400000 */
#define MARINA_NSP_THROW_GRAVITY 2.0F           /* 0x40000000 */
#define MARINA_NSP_AIR_FRICTION 3.0F            /* 0x40400000 */
#define MARINA_NSP_GROUND_TRACTION 1.0F         /* 0x3F800000 */
#define MARINA_NSP_GROUND_TURN_TICS 8
#define MARINA_NSP_AIR_TURN_TICS 6
#define MARINA_NSP_CATCH_IMMUNE_MASK 0x3F       /* asm bitflags, not FTCATCHKIND_MASK_ALL */
#define MARINA_NSP_GRAB_JOINT 30                /* 0x960 = joints[30] (joints at 0x8E8) */
#define MARINA_NSP_GEM_ITEM_ID 0x42             /* Item.Gem.id: 0x2D + 21 add_item index in Item.asm */
#define MARINA_NSP_GEM_FALL_SPEED 30.0F         /* 0x41F00000 */

#define MARINA_NSP_STICK_UP_THRESHOLD 40
#define MARINA_NSP_STICK_DOWN_THRESHOLD (-39)

/* Remix 0xB18 turn frame timer, shares the vanilla common status slot. */
#define MARINA_TURN_TICS(fp) ((fp)->status_vars.common.dead.wait)
/* asm 664-665: f8 = 0x41000000. */
#define MARINA_TURN_TIC_DIVISOR 8.0F
/* Remix 0xB28 gem_spawned flag, shares the vanilla turn.lr_turn slot. */
#define MARINA_GEM_SPAWNED(fp) ((fp)->status_vars.common.turn.lr_turn)

void ftRemixMarinaDSPPlayVoice(FTStruct *fp);

static void ftRemixMarinaSpecialNGroundPullSetStatus(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNAirPullSetStatus(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNGroundThrowSetStatus(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNAirThrowSetStatus(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNGroundToAir(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNAirToGround(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNGrabGroundToAir(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNGrabAirToGround(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNGroundEndMovePhysics(GObj *fighter_gobj);
static void ftRemixMarinaSpecialNAirEndMovePhysics(GObj *fighter_gobj);

/** Returns the Kirby action for Kirby and J Kirby, otherwise the Marina action (asm beql on fkind). */
static s32 ftRemixMarinaSpecialNSelectAction(FTStruct *fp, s32 marina_action, s32 kirby_action)
{
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return kirby_action;
    }
    return marina_action;
}

/**
 * Chooses the normal, U or D throw from the stick. A left-facing stick_x past
 * the threshold turns Marina around and sets the turn timer (Remix
 * ground_throw_initial_ / air_throw_initial_).
 */
static s32 ftRemixMarinaSpecialNSelectThrowVariant(FTStruct *fp, s32 turn_tics)
{
    s32 facing_stick_x = fp->input.pl.stick_range.x * fp->lr;

    if (fp->input.pl.stick_range.y >= MARINA_NSP_STICK_UP_THRESHOLD)
    {
        return MARINA_NSP_THROW_VARIANT_U;
    }
    if (fp->input.pl.stick_range.y < MARINA_NSP_STICK_DOWN_THRESHOLD)
    {
        return MARINA_NSP_THROW_VARIANT_D;
    }
    if (facing_stick_x < MARINA_NSP_STICK_DOWN_THRESHOLD)
    {
        MARINA_TURN_TICS(fp) = turn_tics;
        fp->lr = -fp->lr;
    }
    return MARINA_NSP_THROW_VARIANT_NORMAL;
}

static void ftRemixMarinaSpecialNClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/** Copy of the vanilla air friction step (ftPhysicsApplyAirVelXFriction) with a fixed friction value. */
static void ftRemixMarinaSpecialNApplyAirXFriction(FTStruct *fp, f32 friction)
{
    if (fp->physics.vel_air.x < 0.0F)
    {
        fp->physics.vel_air.x += friction;

        if (fp->physics.vel_air.x >= 0.0F)
        {
            fp->physics.vel_air.x = 0.0F;
        }
    }
    else
    {
        fp->physics.vel_air.x -= friction;

        if (fp->physics.vel_air.x <= 0.0F)
        {
            fp->physics.vel_air.x = 0.0F;
        }
    }
}

/** Remix grab_pull_setup_: arms the command grab and records the captured fighter. */
static void ftRemixMarinaSpecialNGrabPullSetup(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftParamSetCaptureImmuneMask(fp, MARINA_NSP_CATCH_IMMUNE_MASK);
    fp->catch_gobj = fp->search_gobj;
}

/** Remix ground_initial_ (NSPGround special entry). */
static void ftRemixMarinaSpecialNGroundSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPGround, nFTRemixKirbyStatusMarinaNSPG);

    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixMarinaSpecialNGroundPullSetStatus);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMarinaSpecialNClearTempVars(fp);
}

/** Remix air_initial_ (NSPAir special entry). */
static void ftRemixMarinaSpecialNAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPAir, nFTRemixKirbyStatusMarinaNSPA);

    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixMarinaSpecialNAirPullSetStatus);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMarinaSpecialNClearTempVars(fp);
    /* asm andi 0x07 on 0x18D: the intent is clearing the absorb and fast-fall
     * bits (see Falco's ProcInterrupt), the asm's stale v1 is not reproducible. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/** Remix ground_pull_initial_ (NSPGroundPull, the grab's catch proc). */
static void ftRemixMarinaSpecialNGroundPullSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPGroundPull, nFTRemixKirbyStatusMarinaNSPGPull);

    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMarinaSpecialNGrabPullSetup(fighter_gobj);
    ftRemixMarinaSpecialNClearTempVars(fp);
    MARINA_TURN_TICS(fp) = 0;
}

/** Remix air_pull_initial_ (NSPAirPull, the grab's catch proc). Halves the x velocity. */
static void ftRemixMarinaSpecialNAirPullSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPAirPull, nFTRemixKirbyStatusMarinaNSPAPull);

    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMarinaSpecialNGrabPullSetup(fighter_gobj);
    fp->physics.vel_air.x = fp->physics.vel_air.x * 0.5F;
    fp->physics.vel_air.y = 0.0F;
    ftRemixMarinaSpecialNClearTempVars(fp);
    MARINA_TURN_TICS(fp) = 0;
}

/** Remix ground_throw_initial_ (NSPGroundThrow, U and D). */
static void ftRemixMarinaSpecialNGroundThrowSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPGroundThrow, nFTRemixKirbyStatusMarinaNSPGThrow);

    action += ftRemixMarinaSpecialNSelectThrowVariant(fp, MARINA_NSP_GROUND_TURN_TICS);
    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/** Remix air_throw_initial_ (NSPAirThrow, U and D). Zeroes the velocity. */
static void ftRemixMarinaSpecialNAirThrowSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPAirThrow, nFTRemixKirbyStatusMarinaNSPAThrow);

    action += ftRemixMarinaSpecialNSelectThrowVariant(fp, MARINA_NSP_AIR_TURN_TICS);
    ftMainSetStatus(fighter_gobj, action, 0.0F, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
}

/** Remix NSPGroundPull.main_: moves to the ground throw when the pull animation ends. */
static void ftRemixMarinaSpecialNGroundPullProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixMarinaSpecialNGroundThrowSetStatus(fighter_gobj);
    }
}

/** Remix NSPAirPull.main_: moves to the air throw when the pull animation ends. */
static void ftRemixMarinaSpecialNAirPullProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixMarinaSpecialNAirThrowSetStatus(fighter_gobj);
    }
}

/**
 * Remix throw_turn_ (MarinaSpecial.asm 652-697): counts down the turn timer and
 * turns the model like the vanilla Donkey Kong cargo turn.
 */
static void ftRemixMarinaSpecialNThrowProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 turn_tics = MARINA_TURN_TICS(fp);
    f32 turn_target;

    if (turn_tics == 0)
    {
        return;
    }
    turn_tics--;
    MARINA_TURN_TICS(fp) = turn_tics;

    /* asm 671-690: f16/f4 come from the floats at 0x8018C230..0x8018C23C (lui 0x8019 plus the signed
     * lwc1 offset 0xC230..0xC23C). Read from baserom.us.z64 (battle overlay, ROM 0x106C70, RAM - 0x800855C0):
     * 0x40490FDB (180 deg), 0x3FC90FDB (90 deg), 0x40490FDB (180 deg), 0xBFC90FDB (-90 deg). The asm
     * subtracts step * turn_tics / 8 from the target in both directions. */
    turn_target = (fp->lr == +1) ? F_CST_DTOR32(90.0F) : F_CST_DTOR32(-90.0F);
    DObjGetStruct(fighter_gobj)->rotate.vec.f.y = turn_target - (((f32)turn_tics / MARINA_TURN_TIC_DIVISOR) * F_CST_DTOR32(180.0F));
}

/** Remix NSPGround.physics_: grounded grab movement and the end-move friction. */
static void ftRemixMarinaSpecialNGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 state = fp->motion_vars.flags.flag2;

    if (state == MARINA_NSP_END_MOVE)
    {
        ftRemixMarinaSpecialNGroundEndMovePhysics(fighter_gobj);
        return;
    }
    if ((state != MARINA_NSP_BEGIN_MOVE) && (state != MARINA_NSP_MOVE))
    {
        ftPhysicsApplyGroundVelFriction(fighter_gobj);
        return;
    }
    if (state == MARINA_NSP_BEGIN_MOVE)
    {
        fp->physics.vel_ground.x = MARINA_NSP_GROUND_X_SPEED;
        fp->motion_vars.flags.flag2 = MARINA_NSP_MOVE;
    }
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/** Remix NSPAir.physics_: aerial grab movement, control only after the end state. */
static void ftRemixMarinaSpecialNAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 state = fp->motion_vars.flags.flag2;

    if (state == MARINA_NSP_BEGIN_MOVE)
    {
        fp->physics.vel_air.x = MARINA_NSP_AIR_X_SPEED * (f32)fp->lr;
        fp->physics.vel_air.y = 0.0F;
        fp->motion_vars.flags.flag2 = MARINA_NSP_MOVE;
        return;
    }
    if (state == MARINA_NSP_MOVE)
    {
        return;
    }
    if (state == MARINA_NSP_END_MOVE)
    {
        ftRemixMarinaSpecialNAirEndMovePhysics(fighter_gobj);
        return;
    }
    if (state == MARINA_NSP_END)
    {
        ftPhysicsApplyAirVelDriftFastFall(fighter_gobj);
        return;
    }
    ftPhysicsApplyAirVelFriction(fighter_gobj);
}

/** Remix ground_end_move_physics_: vanilla ground friction with fixed traction. */
static void ftRemixMarinaSpecialNGroundEndMovePhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 friction = grRemixSurfaceFriction(fp->coll_data.floor_flags & MAP_VERTEX_MAT_MASK) * MARINA_NSP_GROUND_TRACTION;

    ftPhysicsSetGroundVelFriction(fp, friction);
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

/** Remix air_end_move_physics_: vanilla aerial gravity and friction with fixed friction. */
static void ftRemixMarinaSpecialNAirEndMovePhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    ftPhysicsApplyGravityClampTVel(fp, attr->gravity, attr->tvel_base);
    ftRemixMarinaSpecialNApplyAirXFriction(fp, MARINA_NSP_AIR_FRICTION);
}

/** Remix air_throw_move_physics_: the vanilla aerial drift with throw gravity. */
static void ftRemixMarinaSpecialNAirThrowMovePhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    ftPhysicsApplyGravityClampTVel(fp, MARINA_NSP_THROW_GRAVITY, attr->tvel_base);
    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftPhysicsClampAirVelXStickDefault(fp, attr);
        ftPhysicsApplyAirVelXFriction(fp, attr);
    }
}

/** Remix throw_air_physics_: no control until temp variable 1 is set. */
static void ftRemixMarinaSpecialNAirThrowProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        ftRemixMarinaSpecialNAirThrowMovePhysics(fighter_gobj);
    }
}

/** Remix NSPGround.collision_: transitions to NSPAir when the floor is lost. */
static void ftRemixMarinaSpecialNGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixMarinaSpecialNGroundToAir);
}

/** Remix NSPAir.collision_: lands into NSPGround. */
static void ftRemixMarinaSpecialNAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMarinaSpecialNAirToGround);
}

/** Remix grab_ground_collision_: the grounded grab leaves to NSPAir off the edge. */
static void ftRemixMarinaSpecialNGrabGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMarinaSpecialNGrabGroundToAir);
}

/** Remix grab_air_collision_: the aerial grab lands into the grounded grab. */
static void ftRemixMarinaSpecialNGrabAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMarinaSpecialNGrabAirToGround);
}

/**
 * Remix throw_air_collision_: a landing releases the grabbed fighter. The asm
 * reads v0 from mpCommonProcFighterWaitOrLanding, which is the landing result
 * of mpCommonProcFighterLanding, so that call is made directly here.
 */
static void ftRemixMarinaSpecialNAirThrowProcMap(GObj *fighter_gobj)
{
    if (mpCommonProcFighterLanding(fighter_gobj, mpCommonSetFighterWaitOrLanding) != FALSE)
    {
        ftCommonCatchCaptureSetStatusRelease(fighter_gobj);
    }
}

/** Remix ground_to_air_: the transition from NSPGround to NSPAir. */
static void ftRemixMarinaSpecialNGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPAir, nFTRemixKirbyStatusMarinaNSPA);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixMarinaSpecialNAirPullSetStatus);
}

/** Remix air_to_ground_: the transition from NSPAir to NSPGround. */
static void ftRemixMarinaSpecialNAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 action = ftRemixMarinaSpecialNSelectAction(fp, nFTRemixMarinaStatusNSPGround, nFTRemixKirbyStatusMarinaNSPG);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, action, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixMarinaSpecialNAirPullSetStatus);
}

/** Remix grab_ground_to_air_: grounded grab to the matching aerial grab action (id + 5). */
static void ftRemixMarinaSpecialNGrabGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id + 5, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/** Remix grab_air_to_ground_: aerial grab to the matching grounded grab action (id - 5). */
static void ftRemixMarinaSpecialNGrabAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id - 5, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/** Remix MarinaCargo.jump_main_: enters CargoAir (Donkey's throw fall) when the jump animation ends. */
static void ftRemixMarinaCargoJumpProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftDonkeyThrowFFallSetStatus(fighter_gobj);
    }
}

/** True when the animation frame is -0.0F, the loop marker that Remix's Cargo shake checks for. */
static sb32 ftRemixMarinaCargoShakeIsLoopFrame(f32 anim_frame)
{
    union
    {
        f32 f;
        u32 bits;
    } frame;

    frame.f = anim_frame;

    return frame.bits == 0x80000000;
}

/** Applies the stored damage to the captured fighter (Cargo shake, temp variable 1). */
static void ftRemixMarinaCargoShakeApplyDamage(FTStruct *fp)
{
    s32 damage = fp->motion_vars.flags.flag0;

    if (damage == 0)
    {
        return;
    }
    if (ftParamGetBestHitStatusAll(fp->catch_gobj) != 1)
    {
        return;
    }
    ftParamUpdateDamage(ftGetStruct(fp->catch_gobj), damage);
    fp->motion_vars.flags.flag0 = 0;
}

/** Spawns the gem once per shake, from the grab joint, and clips it into the walls. */
static void ftRemixMarinaCargoShakeSpawnGem(FTStruct *fp)
{
    Vec3f pos;
    Vec3f vel;
    GObj *item_gobj;

    if (MARINA_GEM_SPAWNED(fp) != FALSE)
    {
        return;
    }
    MARINA_GEM_SPAWNED(fp) = TRUE;
    gmCollisionGetFighterPartsWorldPosition(fp->joints[MARINA_NSP_GRAB_JOINT], &pos);
    pos.z = 0.0F;
    vel.x = 0.0F;
    vel.y = MARINA_NSP_GEM_FALL_SPEED;
    vel.z = 0.0F;
    item_gobj = itManagerMakeItemSetupCommon(NULL, MARINA_NSP_GEM_ITEM_ID, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
    if (item_gobj == NULL)
    {
        return;
    }
    mpCommonRunItemCollisionDefault(item_gobj, fp->coll_data.p_translate, &fp->coll_data);
}

/** Cargo shake B-press: arms the loop flag once B is pressed after the press window opens. */
static void ftRemixMarinaCargoShakeCheckBPress(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    if ((fp->input.pl.button_tap & B_BUTTON) == 0)
    {
        return;
    }
    fp->motion_vars.flags.flag2 = TRUE;
}

/**
 * Cargo shake loop: at the loop frame, keeps shaking while the loop flag is set,
 * otherwise enters Cargo. The B-press flag clears on every loop frame.
 */
static void ftRemixMarinaCargoShakeCheckLoop(GObj *fighter_gobj, FTStruct *fp)
{
    s32 loop_requested;

    if (!ftRemixMarinaCargoShakeIsLoopFrame(fighter_gobj->anim_frame))
    {
        return;
    }
    loop_requested = fp->motion_vars.flags.flag2;
    fp->motion_vars.flags.flag2 = FALSE;
    fp->motion_vars.flags.flag1 = FALSE;
    if (loop_requested != FALSE)
    {
        return;
    }
    ftDonkeyThrowFWaitSetStatus(fighter_gobj);
}

/** Remix MarinaCargo.shake_main_: damage, gem spawn, B-press and loop handling. */
static void ftRemixMarinaCargoShakeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarinaCargoShakeApplyDamage(fp);
    ftRemixMarinaCargoShakeSpawnGem(fp);
    ftRemixMarinaCargoShakeCheckBPress(fp);
    ftRemixMarinaCargoShakeCheckLoop(fighter_gobj, fp);
}

/** True for Marina and N. Marina, the two fighters the Remix Cargo patches treat as Marina. */
static sb32 ftRemixMarinaIsCargoFighter(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_MARINA) || (fp->fkind == REMIX_ID_NMARINA);
}

/** Remix MarinaCargo.shake_initial_: enters CargoShake, clears the Cargo temp variables and plays the voice. */
static void ftRemixMarinaCargoShakeInitial(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftMainSetStatus(fighter_gobj, nFTRemixMarinaStatusCargoShake, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    fp = ftGetStruct(fighter_gobj);
    ftRemixMarinaSpecialNClearTempVars(fp);
    ftRemixMarinaDSPPlayVoice(fp);
}

/**
 * Remix MarinaCargo.shake_check_ (vanilla 0x8014DFB4): a grounded Marina that taps B
 * enters CargoShake. Returns TRUE when the action changed.
 */
sb32 ftRemixMarinaCargoShakeCheckInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->fkind != REMIX_ID_MARINA)
    {
        return FALSE;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return FALSE;
    }
    if ((fp->input.pl.button_tap & B_BUTTON) == 0)
    {
        return FALSE;
    }
    ftRemixMarinaCargoShakeInitial(fighter_gobj);
    return TRUE;
}

/** Remix MarinaCargo.jump_patch_ (vanilla 0x8014DB28): CargoJump for (N)Marina, else the vanilla ThrowFFall status. */
s32 ftRemixMarinaCargoJumpStatusId(FTStruct *fp)
{
    if (ftRemixMarinaIsCargoFighter(fp) != FALSE)
    {
        return nFTRemixMarinaStatusCargoJump;
    }
    return nFTDonkeyStatusThrowFFall;
}

/**
 * Frame speed of the Cargo jump, air, jumpsquat and throw-fall statuses (Remix
 * jump_patch_ 0x8014DB28, air_patch_ 0x8014DACC, jumpsquat_patch_ 0x8014D964):
 * 1.0F for (N)Marina, otherwise the vanilla 0.0F.
 */
f32 ftRemixMarinaCargoAnimSpeed(FTStruct *fp)
{
    if (ftRemixMarinaIsCargoFighter(fp) != FALSE)
    {
        return 1.0F;
    }
    return 0.0F;
}

/** Remix MarinaCargo.throw_f_patch_ (vanilla 0x8014E5C0): ThrowF base damage 5 for (N)Marina, else 8. */
s32 ftRemixMarinaCargoThrowFDamage(FTStruct *fp)
{
    if (ftRemixMarinaIsCargoFighter(fp) != FALSE)
    {
        return 5;
    }
    return 8;
}

/** Remix MarinaCargo.throw_f_patch_: clears gem_spawned for (N)Marina so the Cargo shake can spawn a gem. */
void ftRemixMarinaCargoThrowFInitGemSpawned(FTStruct *fp)
{
    if (ftRemixMarinaIsCargoFighter(fp) != FALSE)
    {
        MARINA_GEM_SPAWNED(fp) = FALSE;
    }
}

/* Remix address -> port routine for every MarinaNSP and MarinaCargo routine
 * in the 0x3F worklist. */
const FTRemixProc dFTRemixMarinaNSPProcs[] =
{
    { REMIX_SYM_MarinaCargo__jump_main_, ftRemixMarinaCargoJumpProcUpdate },
    { REMIX_SYM_MarinaCargo__shake_main_, ftRemixMarinaCargoShakeProcUpdate },
    { REMIX_SYM_MarinaNSP__air_collision_, ftRemixMarinaSpecialNAirProcMap },
    { REMIX_SYM_MarinaNSP__air_initial_, ftRemixMarinaSpecialNAirSetStatus },
    { REMIX_SYM_MarinaNSP__air_physics_, ftRemixMarinaSpecialNAirProcPhysics },
    { REMIX_SYM_MarinaNSP__air_pull_main_, ftRemixMarinaSpecialNAirPullProcUpdate },
    { REMIX_SYM_MarinaNSP__grab_air_collision_, ftRemixMarinaSpecialNGrabAirProcMap },
    { REMIX_SYM_MarinaNSP__grab_ground_collision_, ftRemixMarinaSpecialNGrabGroundProcMap },
    { REMIX_SYM_MarinaNSP__ground_collision_, ftRemixMarinaSpecialNGroundProcMap },
    { REMIX_SYM_MarinaNSP__ground_initial_, ftRemixMarinaSpecialNGroundSetStatus },
    { REMIX_SYM_MarinaNSP__ground_physics_, ftRemixMarinaSpecialNGroundProcPhysics },
    { REMIX_SYM_MarinaNSP__ground_pull_main_, ftRemixMarinaSpecialNGroundPullProcUpdate },
    { REMIX_SYM_MarinaNSP__throw_air_collision_, ftRemixMarinaSpecialNAirThrowProcMap },
    { REMIX_SYM_MarinaNSP__throw_air_physics_, ftRemixMarinaSpecialNAirThrowProcPhysics },
    { REMIX_SYM_MarinaNSP__throw_turn_, ftRemixMarinaSpecialNThrowProcInterrupt }
};

const s32 dFTRemixMarinaNSPProcsNum = ARRAY_COUNT(dFTRemixMarinaNSPProcs);

#endif
