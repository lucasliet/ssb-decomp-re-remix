/**
 * ftremixlankyusp.c - C ports of Smash Remix Lanky up special routines
 * (.toolchain/pkgs/smashremix-src/src/Lanky/LankySpecial.asm, scope LankyUSP).
 *
 * The move runs USPGBegin/USPABegin -> USPMove, with USPTurn when the stick
 * pushes against the facing direction, then USPEnd. Remix temp variables 1/2/3
 * are motion_vars.flags.flag0/1/2. USPMove's timer and the damage hitstun timer
 * share the status_vars word at N64 0x0B18.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <math.h>
#include <ft/ftchar/ftmario/ftmario.h>

#define LANKY_USP_N64_TIMER_OFFSET 0x0B18      /* USPMove timer (status_vars word) */
#define LANKY_USP_N64_HITSTUN_OFFSET 0x0B18    /* USPDamage hitstun timer, same word */
#define LANKY_USP_MAX_TIME 120
#define LANKY_USP_FLAGS_CONTINUE 0x0803        /* continue: 3C FGM, gfx routines, hitboxes */
#define LANKY_USP_FLAGS_CANCEL 0x0003          /* continue: gfx routines, hitboxes */
#define LANKY_USP_Y_SPEED 16.0F                /* 0x41800000 */
#define LANKY_USP_END_Y_SPEED 50.0F            /* 0x42480000 */
#define LANKY_USP_AIR_ACCELERATION 0.010009765625F /* 0x3C240000, replaces the attribute value */
#define LANKY_USP_AIR_SPEED 24.0F              /* 0x41C00000 */
#define LANKY_USP_AIR_CONTROL_STICK_MIN 8
#define LANKY_USP_LANDING_FSM 0.375F           /* 0x3EC00000 */
#define LANKY_USP_COLLISION_LANDING_LAG 0.3757031261920929F /* 0x3EC05C29, LANDING_FSM with the vanilla low half */
#define LANKY_USP_TURN_STICK_MIN 11
#define LANKY_USP_DAMAGE_STICK_MIN 10.0F       /* 0x41200000 */
#define LANKY_USP_TURN_SPEED 0.10471999645233154F /* 0x3DD67770, 6 degrees in radians */
#define LANKY_USP_TWO_PI 6.28318977355957F     /* 0x40C90FE4 */
#define LANKY_USP_PI 3.141590118408203F        /* 0x40490FD0 */

enum
{
    nFTRemixLankyStatusUSPGBegin = 0xE5,
    nFTRemixLankyStatusUSPABegin = 0xE6,
    nFTRemixLankyStatusUSPMove = 0xE7,
    nFTRemixLankyStatusUSPTurn = 0xE8,
    nFTRemixLankyStatusUSPEnd = 0xE9
};

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);

/* Animation end is when the animation frame counts down to zero (c.le.s anim_frame, 0). */
static sb32 ftRemixLankyUSPAnimEnded(GObj *fighter_gobj)
{
    return fighter_gobj->anim_frame <= 0.0F;
}

/* LankyUSP.end_initial_: USPEnd with flags from the caller, a fixed fall speed and temp variables 2 and 3 set. */
static void ftRemixLankyUSPEndSetStatus(GObj *fighter_gobj, u32 flags)
{
    FTStruct *fp;

    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPEnd, 0.0F, 1.0F, flags);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->physics.vel_air.y = LANKY_USP_END_Y_SPEED;
    fp->motion_vars.flags.flag1 = 1;
    fp->motion_vars.flags.flag2 = 1;
}

/* LankyUSP.move_initial_: USPMove, which also takes away mid-air jumps and sets aerial kinetics. */
static void ftRemixLankyUSPMoveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPMove, 0.0F, 1.0F, LANKY_USP_FLAGS_CONTINUE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->jumps_used = fp->attr->jumps_max;
    fp->ga = nMPKineticsAir;
}

/* LankyUSP.turn_intial_: USPTurn. */
static void ftRemixLankyUSPTurnSetStatus(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPTurn, 0.0F, 1.0F, LANKY_USP_FLAGS_CONTINUE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Counts USPMove's timer down and ends the move on timeout or B. Returns TRUE when USPEnd began. */
static sb32 ftRemixLankyUSPEndMoveOnTimerOrCancel(GObj *fighter_gobj, FTStruct *fp)
{
    s32 *timer = ftRemixFighterWord(fp, LANKY_USP_N64_TIMER_OFFSET);

    *timer -= 1;
    if (*timer == 0)
    {
        ftRemixLankyUSPEndSetStatus(fighter_gobj, LANKY_USP_FLAGS_CONTINUE);
        return TRUE;
    }
    if (fp->input.pl.button_tap & B_BUTTON)
    {
        ftRemixLankyUSPEndSetStatus(fighter_gobj, LANKY_USP_FLAGS_CANCEL);
        return TRUE;
    }
    return FALSE;
}

/* The stick is at least 11 on x and points opposite to the facing direction (DIRECTION). */
static sb32 ftRemixLankyUSPIsTurnRequested(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_abs = (stick_x < 0) ? -stick_x : stick_x;

    if (stick_abs < LANKY_USP_TURN_STICK_MIN)
    {
        return FALSE;
    }
    return (stick_x < 0) != (fp->lr < 0);
}

/* LankyUSP.ground_initial_: USPGBegin entry, which clears temp variables 1-3. */
static void ftRemixLankyUSPGroundSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPGBegin, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/*
 * LankyUSP.air_initial_: USPABegin entry. The Remix code writes 0 to the
 * fast-fall byte from an unloaded register (asm 455-458); this port clears the
 * four flags it names and keeps the rest.
 */
static void ftRemixLankyUSPAirSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPABegin, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->physics.vel_air.y = 0.0F;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* LankyUSP.begin_main_: once the begin animation ends, sets a USPMove timer and starts USPMove. */
static void ftRemixLankyUSPBeginProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!ftRemixLankyUSPAnimEnded(fighter_gobj))
    {
        return;
    }
    *ftRemixFighterWord(fp, LANKY_USP_N64_TIMER_OFFSET) = LANKY_USP_MAX_TIME;
    ftRemixLankyUSPMoveSetStatus(fighter_gobj);
}

/* LankyUSP.begin_ground_to_air_: ground to USPABegin, keeping the animation frame. */
static void ftRemixLankyUSPBeginGroundToAir(GObj *fighter_gobj)
{
    mpCommonSetFighterAir(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPABegin, fighter_gobj->anim_frame, 1.0F,
                    LANKY_USP_FLAGS_CONTINUE);
    ftPhysicsClampAirVelXMax(ftGetStruct(fighter_gobj));
}

/* LankyUSP.begin_air_to_ground_ transition: air to USPGBegin, keeping the animation frame. */
static void ftRemixLankyUSPBeginAirToGround(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixLankyStatusUSPGBegin, fighter_gobj->anim_frame, 1.0F,
                    LANKY_USP_FLAGS_CONTINUE);
}

/* LankyUSP.begin_ground_collision_: grounded USPGBegin floor check, transitioning on no floor. */
static void ftRemixLankyUSPBeginGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixLankyUSPBeginGroundToAir);
}

/* LankyUSP.begin_air_collision_: aerial USPABegin landing check. */
static void ftRemixLankyUSPBeginAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixLankyUSPBeginAirToGround);
}

/* LankyUSP.main_: USPMove, which ends on timer or B and turns on a stick push against facing. */
static void ftRemixLankyUSPMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixLankyUSPEndMoveOnTimerOrCancel(fighter_gobj, fp))
    {
        return;
    }
    if (ftRemixLankyUSPIsTurnRequested(fp))
    {
        ftRemixLankyUSPTurnSetStatus(fighter_gobj);
    }
}

/* LankyUSP.turn_main_: USPTurn, which flips DIRECTION when temp variable 1 is set. */
static void ftRemixLankyUSPTurnProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->lr = -fp->lr;
        fp->motion_vars.flags.flag0 = 0;
    }
    if (ftRemixLankyUSPEndMoveOnTimerOrCancel(fighter_gobj, fp))
    {
        return;
    }
    if (ftRemixLankyUSPAnimEnded(fighter_gobj))
    {
        ftRemixLankyUSPMoveSetStatus(fighter_gobj);
    }
}

/* LankyUSP.end_main_: USPEnd lands into a special fall with LANDING_FSM once the animation ends. */
static void ftRemixLankyUSPEndProcUpdate(GObj *fighter_gobj)
{
    if (!ftRemixLankyUSPAnimEnded(fighter_gobj))
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, LANKY_USP_LANDING_FSM, FALSE);
}

/* LankyUSP.air_control_: horizontal air drift with the stick range minimum 8 and Remix's acceleration. */
static void ftRemixLankyUSPAirControl(FTStruct *fp)
{
    ftPhysicsClampAirVelXStickRange(fp, LANKY_USP_AIR_CONTROL_STICK_MIN, LANKY_USP_AIR_ACCELERATION,
                                    LANKY_USP_AIR_SPEED);
}

/* LankyUSP.physics_: air control and friction, then a constant rising speed. */
static void ftRemixLankyUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixLankyUSPAirControl(fp);
    ftPhysicsApplyAirVelXFriction(fp, fp->attr);
    fp->physics.vel_air.y = LANKY_USP_Y_SPEED;
}

/* LankyUSP.collision_: Mario's up special collision with Lanky's landing lag (Remix 0x80156358 copy). */
static void ftRemixLankyUSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsAir)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    if ((fp->motion_vars.flags.flag1 == 0) || (fp->physics.vel_air.y >= 0.0F))
    {
        mpCommonCheckFighterProject(fighter_gobj);
        return;
    }
    if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) == FALSE)
    {
        return;
    }
    if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
    {
        ftCommonCliffCatchSetStatus(fighter_gobj);
        return;
    }
    ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, LANKY_USP_COLLISION_LANDING_LAG);
}

/*
 * Reads the stick as an angle when it is at least 10 from centre (asm 796-806).
 * Returns FALSE and leaves *angle untouched otherwise.
 */
static sb32 ftRemixLankyUSPGetStickAngle(FTStruct *fp, f32 *angle)
{
    f32 stick_x = (f32)fp->input.pl.stick_range.x;
    f32 stick_y = (f32)fp->input.pl.stick_range.y;
    f32 magnitude = sqrtf((stick_y * stick_y) + (stick_x * stick_x));

    if (!(LANKY_USP_DAMAGE_STICK_MIN <= magnitude))
    {
        return FALSE;
    }
    *angle = atan2f(stick_y, stick_x);
    return TRUE;
}

/*
 * Rotates the knockback velocity toward the stick angle by TURN_SPEED per frame
 * (asm 808-864). Nothing changes when the remaining difference is within TURN_SPEED.
 */
static void ftRemixLankyUSPTurnKnockback(FTStruct *fp, f32 stick_angle)
{
    f32 vel_x = fp->physics.vel_damage_air.x;
    f32 vel_y = fp->physics.vel_damage_air.y;
    f32 speed = sqrtf((vel_y * vel_y) + (vel_x * vel_x));
    f32 knockback_angle = atan2f(vel_y, vel_x);
    f32 difference = (stick_angle + LANKY_USP_TWO_PI) - (knockback_angle + LANKY_USP_TWO_PI);

    if (!(0.0F < difference))
    {
        difference += LANKY_USP_TWO_PI;
    }
    if (!(LANKY_USP_TURN_SPEED < difference))
    {
        return;
    }
    if (difference < LANKY_USP_PI)
    {
        knockback_angle += LANKY_USP_TURN_SPEED;
    }
    else
    {
        knockback_angle += -LANKY_USP_TURN_SPEED;
    }
    fp->physics.vel_damage_air.x = speed * __cosf(knockback_angle);
    fp->physics.vel_damage_air.y = speed * __sinf(knockback_angle);
}

/* LankyUSP.damage_physics_: USPDamage knockback steering and fast fall after hitstun. */
static void ftRemixLankyUSPDamageProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 stick_angle;

    if (*ftRemixFighterWord(fp, LANKY_USP_N64_HITSTUN_OFFSET) == 0)
    {
        ftPhysicsApplyAirVelDriftFastFall(fighter_gobj);
    }
    if (ftRemixLankyUSPGetStickAngle(fp, &stick_angle))
    {
        ftRemixLankyUSPTurnKnockback(fp, stick_angle);
    }
    ftCommonDamageFlyRollUpdateModelPitch(fighter_gobj);
}

/* Remix address -> port routine for every LankyUSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixLankyUSPProcs[] =
{
    { REMIX_SYM_LankyUSP__air_initial_, ftRemixLankyUSPAirSetStatus },
    { REMIX_SYM_LankyUSP__begin_air_collision_, ftRemixLankyUSPBeginAirProcMap },
    { REMIX_SYM_LankyUSP__begin_ground_collision_, ftRemixLankyUSPBeginGroundProcMap },
    { REMIX_SYM_LankyUSP__begin_main_, ftRemixLankyUSPBeginProcUpdate },
    { REMIX_SYM_LankyUSP__collision_, ftRemixLankyUSPProcMap },
    { REMIX_SYM_LankyUSP__damage_physics_, ftRemixLankyUSPDamageProcPhysics },
    { REMIX_SYM_LankyUSP__end_main_, ftRemixLankyUSPEndProcUpdate },
    { REMIX_SYM_LankyUSP__ground_initial_, ftRemixLankyUSPGroundSetStatus },
    { REMIX_SYM_LankyUSP__main_, ftRemixLankyUSPMoveProcUpdate },
    { REMIX_SYM_LankyUSP__physics_, ftRemixLankyUSPProcPhysics },
    { REMIX_SYM_LankyUSP__turn_main_, ftRemixLankyUSPTurnProcUpdate }
};

const s32 dFTRemixLankyUSPProcsNum = ARRAY_COUNT(dFTRemixLankyUSPProcs);

#endif
