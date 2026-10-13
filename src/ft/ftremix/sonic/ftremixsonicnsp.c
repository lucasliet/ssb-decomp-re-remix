/**
 * ftremixsonicnsp.c - C ports of Smash Remix Sonic Homing Attack routines
 * (.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm, scope SonicNSP).
 *
 * Remix keeps the homing state in player struct words: target object 0x0B18,
 * X_DIFF 0x0B1C, movement angle 0x0B20 and movement timer 0x0B28. The target is
 * a GObj pointer, which does not fit a 32-bit word on the LP64 port, so it lives
 * per player in sFTRemixSonicNSPTarget. The other words are read through
 * SONIC_NSP_OFF_* as Marth's port does, because SonicDSP shares those slots.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <it/item.h>
#include <mp/mpcommon.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <sys/obj.h>
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sys/utils.h>

#define SONIC_NSP_OFF_KIRBY_HAT 0x0980         /* s8 hat id; overlaps FTStruct.modelpart_status */
#define SONIC_NSP_OFF_X_DIFF 0x0B1C            /* f32 target X_DIFF * DIRECTION */
#define SONIC_NSP_OFF_MOVE_ANGLE 0x0B20        /* f32 movement angle */
#define SONIC_NSP_OFF_MOVE_TIMER 0x0B28        /* s32 movement timer */

#define SONIC_NSP_SSONIC_HAT_ID 0x2F
#define SONIC_NSP_MAX_X_RANGE 2208.0F          /* 0x450A0000 */
#define SONIC_NSP_MAX_X_RANGE_SS 5984.0F       /* 0x45BB0000 */
#define SONIC_NSP_MIN_Y_RANGE 1000.0F          /* 0x447A0000 */
#define SONIC_NSP_MIN_Y_RANGE_SS 4000.0F       /* 0x457A0000 */
#define SONIC_NSP_SPEED 110.0F                 /* 0x42DC0000 */
#define SONIC_NSP_SPEED_SS 130.0F              /* 0x43020000 */
#define SONIC_NSP_LOCKED_SPEED 130.0F          /* 0x43020000 */
#define SONIC_NSP_LOCKED_SPEED_SS 130.0F       /* 0x43020000 */
#define SONIC_NSP_RECOIL_X_SPEED -20.0F        /* 0xC1A00000 */
#define SONIC_NSP_RECOIL_Y_SPEED 80.0F         /* 0x42A00000 */
#define SONIC_NSP_BOUNCE_Y_SPEED 60.0F         /* 0x42700000 */
#define SONIC_NSP_BEGIN_Y_SPEED 12.0F          /* 0x41400000 */
#define SONIC_NSP_TURN_SPEED 0.13962600F       /* 0x3E0EFA1E, 8 degrees */
#define SONIC_NSP_TURN_SPEED_SS 0.0262451171875F /* 0x3CD70000 */
#define SONIC_NSP_DEFAULT_ANGLE -0.36651900F   /* 0xBEBBA861 */
#define SONIC_NSP_MAX_INITIAL_ANGLE 0.61086500F /* 0x3F1C61A6 */
#define SONIC_NSP_MIN_INITIAL_ANGLE -0.61086500F /* 0xBF1C61A6 */
#define SONIC_NSP_TWO_PI 6.2831898F            /* 0x40C90FE4 */
#define SONIC_NSP_MINUS_PI -3.1415901F         /* 0xC0490FD0 */
#define SONIC_NSP_DURATION 12
#define SONIC_NSP_LOCKED_DURATION 20
#define SONIC_NSP_BOUNCE_VEL_X_MUL 0.25F
#define SONIC_NSP_END_VEL_MUL 0.5F
#define SONIC_NSP_SSONIC_END_VEL_MUL 0.25F     /* 0x3E800000 */

enum
{
    nFTRemixSonicStatusNSPBegin = 0xFD,
    nFTRemixSonicStatusNSPMove = 0xFE,
    nFTRemixSonicStatusNSPLockedMove = 0xFF,
    nFTRemixSonicStatusNSPGroundEnd = 0x100,
    nFTRemixSonicStatusNSPAirEnd = 0x101,
    nFTRemixSonicStatusNSPGroundRecoil = 0x102,
    nFTRemixSonicStatusNSPAirRecoil = 0x103,
    nFTRemixSonicStatusNSPBounce = 0x104
};

enum
{
    nFTRemixKirbyStatusSonicNSPBegin = 0x15E,
    nFTRemixKirbyStatusSonicNSPMove = 0x15F,
    nFTRemixKirbyStatusSonicNSPLockedMove = 0x160,
    nFTRemixKirbyStatusSonicNSPGroundEnd = 0x161,
    nFTRemixKirbyStatusSonicNSPAirEnd = 0x162,
    nFTRemixKirbyStatusSonicNSPGroundRecoil = 0x163,
    nFTRemixKirbyStatusSonicNSPAirRecoil = 0x164,
    nFTRemixKirbyStatusSonicNSPBounce = 0x165,
    nFTRemixKirbyStatusSonicSSonicNSPBegin = 0x1C7,
    nFTRemixKirbyStatusSonicSSonicNSPMove = 0x1C8,
    nFTRemixKirbyStatusSonicSSonicNSPLockedMove = 0x1C9
};

/* Target object per player (SonicNSP target word 0x0B18); NULL when none. */
static GObj *sFTRemixSonicNSPTarget[MAXCONTROLLERS];

static sb32 ftRemixSonicNSPIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

static sb32 ftRemixSonicNSPHasSuperSonicHat(FTStruct *fp)
{
    return *ftRemixFighterKirbyHat(fp) == SONIC_NSP_SSONIC_HAT_ID;
}

/* SonicNSP.is_super_sonic */
static sb32 ftRemixSonicNSPIsSuperSonic(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_SSONIC)
    {
        return TRUE;
    }
    if (!ftRemixSonicNSPIsKirby(fp))
    {
        return FALSE;
    }
    return ftRemixSonicNSPHasSuperSonicHat(fp);
}

static s32 ftRemixSonicNSPStatusFor(FTStruct *fp, s32 sonic_status, s32 kirby_status)
{
    return ftRemixSonicNSPIsKirby(fp) ? kirby_status : sonic_status;
}

/* Kirby with the Super Sonic hat uses the SSonic copy of the move. */
static s32 ftRemixSonicNSPMoveStatusFor(FTStruct *fp, s32 sonic_status, s32 kirby_status, s32 kirby_ssonic_status)
{
    if (!ftRemixSonicNSPIsKirby(fp))
    {
        return sonic_status;
    }
    return ftRemixSonicNSPHasSuperSonicHat(fp) ? kirby_ssonic_status : kirby_status;
}

/* Changes status from its first frame, then plays the animation events (Remix's initial routines). */
static void ftRemixSonicNSPStartAction(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Changes status from the current animation frame, continuing the hitbox (Remix's transition routines). */
static void ftRemixSonicNSPContinueAction(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, 1.0F, 1);
}

static GObj *ftRemixSonicNSPTargetOf(FTStruct *fp)
{
    return sFTRemixSonicNSPTarget[fp->player];
}

static void ftRemixSonicNSPSetTarget(FTStruct *fp, GObj *target_gobj)
{
    sFTRemixSonicNSPTarget[fp->player] = target_gobj;
}

static f32 *ftRemixSonicNSPXDiff(FTStruct *fp)
{
    return ftRemixFighterFloat(fp, SONIC_NSP_OFF_X_DIFF);
}

static f32 *ftRemixSonicNSPMoveAngle(FTStruct *fp)
{
    return ftRemixFighterFloat(fp, SONIC_NSP_OFF_MOVE_ANGLE);
}

static s32 *ftRemixSonicNSPMoveTimer(FTStruct *fp)
{
    return ftRemixFighterWord(fp, SONIC_NSP_OFF_MOVE_TIMER);
}

static void ftRemixSonicNSPClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall (byte 0x018D, mask 0x07). */
static void ftRemixSonicNSPClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

static f32 ftRemixSonicNSPAbs(f32 value)
{
    return (value < 0.0F) ? -value : value;
}

/* Keeps an angle above -pi by adding a full turn, as Remix's `c.lt.s` + `bc1fl` pairs do. */
static f32 ftRemixSonicNSPWrapAngle(f32 angle)
{
    if (!(SONIC_NSP_MINUS_PI < angle))
    {
        return angle + SONIC_NSP_TWO_PI;
    }
    return angle;
}

/* Remix's Character.get_hitbox_collision_flags_ masked with 0x00F0: any interact bit on an active attack coll. */
static sb32 ftRemixSonicNSPAttackCollHasInteraction(FTAttackColl *attack_coll)
{
    s32 i;

    if (attack_coll->attack_state == 0)
    {
        return FALSE;
    }
    for (i = 0; i < ARRAY_COUNT(attack_coll->attack_records); i++)
    {
        GMAttackRecord *record = &attack_coll->attack_records[i];

        if (record->victim_flags.is_interact_hurt || record->victim_flags.is_interact_shield ||
            record->victim_flags.is_interact_reflect || record->victim_flags.is_interact_absorb)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static sb32 ftRemixSonicNSPHitboxCollided(FTStruct *fp)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(fp->attack_colls); i++)
    {
        if (ftRemixSonicNSPAttackCollHasInteraction(&fp->attack_colls[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Team battle without team attack: teammates are not valid targets. */
static sb32 ftRemixSonicNSPIsTeamBlocked(FTStruct *fp, FTStruct *other_fp)
{
    SCBattleState *battle = gSCManagerBattleState;

    if (!battle->is_team_battle || battle->is_team_attack)
    {
        return FALSE;
    }
    return other_fp->team == fp->team;
}

/* Targets in a KO action (status id < 7) are skipped. */
static sb32 ftRemixSonicNSPIsKOStatus(FTStruct *other_fp)
{
    return (u32)other_fp->status_id < 7;
}

static sb32 ftRemixSonicNSPIsInXRange(FTStruct *fp, f32 x_diff)
{
    f32 max_x_range = ftRemixSonicNSPIsSuperSonic(fp) ? SONIC_NSP_MAX_X_RANGE_SS : SONIC_NSP_MAX_X_RANGE;

    if (!(x_diff <= max_x_range))
    {
        return FALSE;
    }
    return 0.0F <= x_diff;
}

/* A new candidate must not be farther than the current target. */
static sb32 ftRemixSonicNSPIsCloserThanTarget(FTStruct *fp, f32 x_diff)
{
    if (ftRemixSonicNSPTargetOf(fp) == NULL)
    {
        return TRUE;
    }
    return x_diff <= *ftRemixSonicNSPXDiff(fp);
}

/* Cone-shaped Y range that widens with X_DIFF. */
static sb32 ftRemixSonicNSPIsInYRange(FTStruct *fp, f32 x_diff, f32 y_diff)
{
    f32 min_y_range = ftRemixSonicNSPIsSuperSonic(fp) ? SONIC_NSP_MIN_Y_RANGE_SS : SONIC_NSP_MIN_Y_RANGE;
    f32 y_range = min_y_range + (0.5F * x_diff);

    return ftRemixSonicNSPAbs(y_diff) <= y_range;
}

/* SonicNSP.check_target_ */
static sb32 ftRemixSonicNSPCheckTarget(FTStruct *fp, GObj *candidate_gobj, f32 *x_diff)
{
    Vec3f *player_pos = fp->coll_data.p_translate;
    Vec3f *target_pos = &DObjGetStruct(candidate_gobj)->translate.vec.f;
    f32 candidate_x_diff = (target_pos->x - player_pos->x) * (f32)fp->lr;
    f32 y_diff = target_pos->y - player_pos->y;

    if (!ftRemixSonicNSPIsInXRange(fp, candidate_x_diff))
    {
        return FALSE;
    }
    if (!ftRemixSonicNSPIsCloserThanTarget(fp, candidate_x_diff))
    {
        return FALSE;
    }
    if (!ftRemixSonicNSPIsInYRange(fp, candidate_x_diff, y_diff))
    {
        return FALSE;
    }
    *x_diff = candidate_x_diff;
    return TRUE;
}

static void ftRemixSonicNSPTryTarget(FTStruct *fp, GObj *candidate_gobj)
{
    f32 x_diff;

    if (!ftRemixSonicNSPCheckTarget(fp, candidate_gobj, &x_diff))
    {
        return;
    }
    ftRemixSonicNSPSetTarget(fp, candidate_gobj);
    *ftRemixSonicNSPXDiff(fp) = x_diff;
}

static void ftRemixSonicNSPSearchFighters(FTStruct *fp, GObj *fighter_gobj)
{
    GObj *other_gobj;

    for (other_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; other_gobj != NULL; other_gobj = other_gobj->link_next)
    {
        FTStruct *other_fp = ftGetStruct(other_gobj);

        if ((other_gobj == fighter_gobj) || ftRemixSonicNSPIsTeamBlocked(fp, other_fp) ||
            ftRemixSonicNSPIsKOStatus(other_fp))
        {
            continue;
        }
        ftRemixSonicNSPTryTarget(fp, other_gobj);
    }
}

/* Item hurtbox is active while hitstatus bit 0 is set (ITStruct.damage_coll, Remix word 0x0248). */
static sb32 ftRemixSonicNSPIsItemHurtboxActive(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    return (ip->damage_coll.hitstatus & 0x1) != 0;
}

static void ftRemixSonicNSPSearchItems(FTStruct *fp)
{
    GObj *item_gobj;

    for (item_gobj = gGCCommonLinks[nGCCommonLinkIDItem]; item_gobj != NULL; item_gobj = item_gobj->link_next)
    {
        if (ftRemixSonicNSPIsItemHurtboxActive(item_gobj))
        {
            ftRemixSonicNSPTryTarget(fp, item_gobj);
        }
    }
}

/* SonicNSP.check_for_targets_: fighters first, items only while no fighter target is held. */
static void ftRemixSonicNSPCheckForTargets(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPSearchFighters(fp, fighter_gobj);
    if (ftRemixSonicNSPTargetOf(fp) != NULL)
    {
        return;
    }
    ftRemixSonicNSPSearchItems(fp);
}

/* atan2 of the target offset (Y_DIFF, X_DIFF * DIRECTION). */
static f32 ftRemixSonicNSPAngleToTarget(FTStruct *fp)
{
    GObj *target_gobj = ftRemixSonicNSPTargetOf(fp);
    Vec3f *player_pos = fp->coll_data.p_translate;
    Vec3f *target_pos = &DObjGetStruct(target_gobj)->translate.vec.f;
    f32 x_diff = (target_pos->x - player_pos->x) * (f32)fp->lr;
    f32 y_diff = target_pos->y - player_pos->y;

    return syUtilsArcTan2(y_diff, x_diff);
}

static f32 ftRemixSonicNSPClampInitialAngle(f32 angle)
{
    if (!(SONIC_NSP_MIN_INITIAL_ANGLE < angle))
    {
        return SONIC_NSP_MIN_INITIAL_ANGLE;
    }
    if (!(angle < SONIC_NSP_MAX_INITIAL_ANGLE))
    {
        return SONIC_NSP_MAX_INITIAL_ANGLE;
    }
    return angle;
}

/* Top joint rotation.z is the movement angle mirrored by DIRECTION. */
static void ftRemixSonicNSPSetJointAngle(FTStruct *fp, f32 angle)
{
    DObjGetStruct(fp->fighter_gobj)->rotate.vec.f.z = (f32)fp->lr * angle;
}

/* SonicNSP.locked_move_initial_ */
static void ftRemixSonicNSPLockedMoveInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 angle;

    ftRemixSonicNSPStartAction(fighter_gobj, ftRemixSonicNSPMoveStatusFor(fp,
        nFTRemixSonicStatusNSPLockedMove, nFTRemixKirbyStatusSonicNSPLockedMove,
        nFTRemixKirbyStatusSonicSSonicNSPLockedMove));
    angle = ftRemixSonicNSPClampInitialAngle(ftRemixSonicNSPAngleToTarget(fp));
    *ftRemixSonicNSPMoveAngle(fp) = angle;
    ftRemixSonicNSPSetJointAngle(fp, angle);
    *ftRemixSonicNSPMoveTimer(fp) = SONIC_NSP_LOCKED_DURATION;
}

/* SonicNSP.move_initial_ */
static void ftRemixSonicNSPMoveInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPStartAction(fighter_gobj, ftRemixSonicNSPMoveStatusFor(fp,
        nFTRemixSonicStatusNSPMove, nFTRemixKirbyStatusSonicNSPMove,
        nFTRemixKirbyStatusSonicSSonicNSPMove));
    *ftRemixSonicNSPMoveAngle(fp) = SONIC_NSP_DEFAULT_ANGLE;
    ftRemixSonicNSPSetJointAngle(fp, SONIC_NSP_DEFAULT_ANGLE);
    *ftRemixSonicNSPMoveTimer(fp) = SONIC_NSP_DURATION;
}

/* SonicNSP.bounce_initial_ */
static void ftRemixSonicNSPBounceInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPStartAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPBounce, nFTRemixKirbyStatusSonicNSPBounce));
    fp->physics.vel_air.x *= SONIC_NSP_BOUNCE_VEL_X_MUL;
    fp->physics.vel_air.y = SONIC_NSP_BOUNCE_Y_SPEED;
}

/* SonicNSP.ground_end_initial_ */
static void ftRemixSonicNSPGroundEndInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPStartAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPGroundEnd, nFTRemixKirbyStatusSonicNSPGroundEnd));
    fp->physics.vel_ground.x *= SONIC_NSP_END_VEL_MUL;
}

/* SonicNSP.air_end_initial_ */
static void ftRemixSonicNSPAirEndInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPStartAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPAirEnd, nFTRemixKirbyStatusSonicNSPAirEnd));
    fp->physics.vel_air.x *= SONIC_NSP_END_VEL_MUL;
    fp->physics.vel_air.y *= SONIC_NSP_END_VEL_MUL;
}

/* SonicNSP.air_recoil_initial_ */
static void ftRemixSonicNSPAirRecoilInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPStartAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPAirRecoil, nFTRemixKirbyStatusSonicNSPAirRecoil));
    fp->physics.vel_air.x = SONIC_NSP_RECOIL_X_SPEED * (f32)fp->lr;
    fp->physics.vel_air.y = SONIC_NSP_RECOIL_Y_SPEED;
}

/* SonicNSP.ground_end_transition_ */
static void ftRemixSonicNSPGroundEndTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixSonicNSPContinueAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPGroundEnd, nFTRemixKirbyStatusSonicNSPGroundEnd));
}

/* SonicNSP.air_end_transition_ */
static void ftRemixSonicNSPAirEndTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixSonicNSPContinueAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPAirEnd, nFTRemixKirbyStatusSonicNSPAirEnd));
    ftPhysicsClampAirVelXMax(fp);
}

/* SonicNSP.ground_recoil_transition_ */
static void ftRemixSonicNSPGroundRecoilTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixSonicNSPContinueAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPGroundRecoil, nFTRemixKirbyStatusSonicNSPGroundRecoil));
}

/* SonicNSP.air_recoil_transition_ */
static void ftRemixSonicNSPAirRecoilTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixSonicNSPContinueAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPAirRecoil, nFTRemixKirbyStatusSonicNSPAirRecoil));
    ftPhysicsClampAirVelXMax(fp);
}

static f32 ftRemixSonicNSPTurnSpeed(FTStruct *fp)
{
    return ftRemixSonicNSPIsSuperSonic(fp) ? SONIC_NSP_TURN_SPEED_SS : SONIC_NSP_TURN_SPEED;
}

/* Turns the movement angle toward the target by at most one turn step. */
static void ftRemixSonicNSPTurnToTarget(FTStruct *fp)
{
    f32 current_angle = *ftRemixSonicNSPMoveAngle(fp);
    f32 target_angle = ftRemixSonicNSPAngleToTarget(fp);
    f32 diff = ftRemixSonicNSPWrapAngle(target_angle - current_angle);
    f32 turn_speed = ftRemixSonicNSPTurnSpeed(fp);
    f32 next_angle = target_angle;

    if (turn_speed < ftRemixSonicNSPAbs(diff))
    {
        next_angle = (diff > 0.0F) ? (current_angle + turn_speed) : (current_angle - turn_speed);
    }
    next_angle = ftRemixSonicNSPWrapAngle(next_angle);
    *ftRemixSonicNSPMoveAngle(fp) = next_angle;
    ftRemixSonicNSPSetJointAngle(fp, next_angle);
}

static f32 ftRemixSonicNSPMoveSpeed(FTStruct *fp, sb32 is_locked)
{
    sb32 is_super = ftRemixSonicNSPIsSuperSonic(fp);

    if (!is_locked)
    {
        return is_super ? SONIC_NSP_SPEED_SS : SONIC_NSP_SPEED;
    }
    return is_super ? SONIC_NSP_LOCKED_SPEED_SS : SONIC_NSP_LOCKED_SPEED;
}

/* SonicNSP.move_physics_ */
static void ftRemixSonicNSPMovePhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_locked = (ftRemixSonicNSPTargetOf(fp) != NULL);
    f32 speed = ftRemixSonicNSPMoveSpeed(fp, is_locked);
    f32 angle;

    if (is_locked)
    {
        ftRemixSonicNSPTurnToTarget(fp);
    }
    angle = *ftRemixSonicNSPMoveAngle(fp);
    fp->physics.vel_air.x = speed * __cosf(angle) * (f32)fp->lr;
    fp->physics.vel_air.y = speed * __sinf(angle);
}

/* SonicNSP.move_main_ (NSP_Move and NSP_Locked_Move) */
static void ftRemixSonicNSPMoveMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *timer = ftRemixSonicNSPMoveTimer(fp);

    if (ftRemixSonicNSPHitboxCollided(fp))
    {
        ftRemixSonicNSPAirRecoilInitial(fighter_gobj);
        return;
    }
    *timer -= 1;
    if (*timer != 0)
    {
        return;
    }
    if (fp->ga != nMPKineticsGround)
    {
        ftRemixSonicNSPAirEndInitial(fighter_gobj);
    }
    else
    {
        ftRemixSonicNSPGroundEndInitial(fighter_gobj);
    }
}

/* SonicNSP.move_collision_ */
static void ftRemixSonicNSPMoveCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSonicNSPBounceInitial);
}

/* SonicNSP.ground_end_collision_ */
static void ftRemixSonicNSPGroundEndCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSonicNSPAirEndTransition);
}

/* SonicNSP.air_end_collision_ */
static void ftRemixSonicNSPAirEndCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSonicNSPGroundEndTransition);
}

/* SonicNSP.ground_recoil_collision_ */
static void ftRemixSonicNSPGroundRecoilCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSonicNSPAirRecoilTransition);
}

/* SonicNSP.air_recoil_collision_ */
static void ftRemixSonicNSPAirRecoilCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixSonicNSPGroundRecoilTransition);
}

/* SonicNSP.ssonic_begin_initial_ (special entry for Super Sonic). Unlike begin_initial_, it keeps the
 * target and X_DIFF and plays no animation events. */
static void ftRemixSonicNSPSSonicBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftMainSetStatus(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPBegin, nFTRemixKirbyStatusSonicSSonicNSPBegin), 0.0F, 1.0F, 0);
    ftRemixSonicNSPClearTempVars(fp);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = SONIC_NSP_BEGIN_Y_SPEED;
    ftRemixSonicNSPClearAirFlags(fp);
}

/* SonicNSP.ssonic_move_initial_: Super Sonic's move start. Clears the target and velocity, then locks
 * onto a target or moves in a fixed direction. */
static void ftRemixSonicNSPSSonicMoveInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSonicNSPSetTarget(fp, NULL);
    *ftRemixSonicNSPXDiff(fp) = 0.0F;
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    ftRemixSonicNSPCheckForTargets(fighter_gobj);
    if (ftRemixSonicNSPTargetOf(fp) != NULL)
    {
        ftRemixSonicNSPLockedMoveInitial(fighter_gobj);
        return;
    }
    ftRemixSonicNSPMoveInitial(fighter_gobj);
}

/* SonicNSP.ssonic_begin_main_: moves to ssonic_move_initial_ when the animation ends. */
static void ftRemixSonicNSPSSonicBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixSonicNSPSSonicMoveInitial);
}

/* SonicNSP.ssonic_move_main_: once temp variable 2 is set and B is released, scales the velocity
 * by 0.25 and goes to idle. */
static void ftRemixSonicNSPSSonicMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    if (fp->input.pl.button_hold & B_BUTTON)
    {
        return;
    }
    fp->physics.vel_air.x *= SONIC_NSP_SSONIC_END_VEL_MUL;
    fp->physics.vel_air.y *= SONIC_NSP_SSONIC_END_VEL_MUL;
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* SonicNSP.ssonic_move_collision_: lands while temp variable 2 is set, otherwise projects off the wall. */
static void ftRemixSonicNSPSSonicMoveProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
        return;
    }
    mpCommonProcFighterProject(fighter_gobj);
}

/* SonicNSP.begin_main_ */
static void ftRemixSonicNSPBeginMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 temp_flag0 = fp->motion_vars.flags.flag0;

    if (temp_flag0 == 0)
    {
        return;
    }
    if ((temp_flag0 == 1) && (fp->input.pl.button_hold & B_BUTTON))
    {
        return;
    }
    ftRemixSonicNSPCheckForTargets(fighter_gobj);
    if (ftRemixSonicNSPTargetOf(fp) != NULL)
    {
        ftRemixSonicNSPLockedMoveInitial(fighter_gobj);
    }
    else
    {
        ftRemixSonicNSPMoveInitial(fighter_gobj);
    }
}

/* SonicNSP.begin_initial_ (special entry for neutral special) */
static void ftRemixSonicNSPBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    ftRemixSonicNSPStartAction(fighter_gobj,
        ftRemixSonicNSPStatusFor(fp, nFTRemixSonicStatusNSPBegin, nFTRemixKirbyStatusSonicNSPBegin));
    ftRemixSonicNSPClearTempVars(fp);
    ftRemixSonicNSPSetTarget(fp, NULL);
    *ftRemixSonicNSPXDiff(fp) = 0.0F;
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = SONIC_NSP_BEGIN_Y_SPEED;
    ftRemixSonicNSPClearAirFlags(fp);
}

/* Called by SonicNSP.destroyed_target_fix_ when a GObj is destroyed; clears it as anyone's target. */
void ftRemixSonicNSPForgetTarget(GObj *destroyed_gobj)
{
    s32 player;

    for (player = 0; player < MAXCONTROLLERS; player++)
    {
        if (sFTRemixSonicNSPTarget[player] == destroyed_gobj)
        {
            sFTRemixSonicNSPTarget[player] = NULL;
        }
    }
}

/* Remix address -> port routine for every SonicNSP routine in the Sonic worklist. */
const FTRemixProc dFTRemixSonicNSPProcs[] =
{
    { REMIX_SYM_SonicNSP__begin_initial_, ftRemixSonicNSPBeginInitial },
    { REMIX_SYM_SonicNSP__begin_main_, ftRemixSonicNSPBeginMain },
    { REMIX_SYM_SonicNSP__move_main_, ftRemixSonicNSPMoveMain },
    { REMIX_SYM_SonicNSP__move_physics_, ftRemixSonicNSPMovePhysics },
    { REMIX_SYM_SonicNSP__move_collision_, ftRemixSonicNSPMoveCollision },
    { REMIX_SYM_SonicNSP__ground_end_collision_, ftRemixSonicNSPGroundEndCollision },
    { REMIX_SYM_SonicNSP__air_end_collision_, ftRemixSonicNSPAirEndCollision },
    { REMIX_SYM_SonicNSP__ground_recoil_collision_, ftRemixSonicNSPGroundRecoilCollision },
    { REMIX_SYM_SonicNSP__air_recoil_collision_, ftRemixSonicNSPAirRecoilCollision },
    { REMIX_SYM_SonicNSP__ssonic_begin_initial_, ftRemixSonicNSPSSonicBeginInitial },
    { REMIX_SYM_SonicNSP__ssonic_begin_main_, ftRemixSonicNSPSSonicBeginProcUpdate },
    { REMIX_SYM_SonicNSP__ssonic_move_main_, ftRemixSonicNSPSSonicMoveProcUpdate },
    { REMIX_SYM_SonicNSP__ssonic_move_collision_, ftRemixSonicNSPSSonicMoveProcMap }
};

const s32 dFTRemixSonicNSPProcsNum = ARRAY_COUNT(dFTRemixSonicNSPProcs);

#endif
