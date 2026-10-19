/*
 * ftremixairdodge.c - Air Dodging setting of Smash Remix (.toolchain/pkgs/smashremix-src/src/AirDodge.asm).
 * The jump check of ftCommonJumpAerialCheckInterruptCommon calls ftRemixAirDodgeCheckInterrupt before the hammer check.
 * The grounded and initial scripts reset the air dash flag of the port through ftRemixAirDodgeResetAirDash.
 * Setting values (Toggles.asm entry_air_dodge): 0 Off, 1 Melee, 2 Ultimate, 3 Air Dash.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmdef.h>
#include <gm/gmremixtoggle.h>
#include <mp/mpcommon.h>
#include <sys/utils.h>
#include "ft/ftremix/ftremixhooks.h"

enum
{
    nFTRemixAirDodgeTypeOff,
    nFTRemixAirDodgeTypeMelee,
    nFTRemixAirDodgeTypeUltimate,
    nFTRemixAirDodgeTypeAirDash,
};

/* AirDodge.asm: 80.0F, 1.5707964F (0x3FC90FDB), 0.5F, 1.1875F and 0.875F */
#define FTREMIX_AIRDODGE_MAX_SPEED 80.0F
#define FTREMIX_AIRDODGE_DEFAULT_ANGLE 1.5707964F
#define FTREMIX_AIRDODGE_LANDING_FSM 0.5F
#define FTREMIX_AIRDODGE_SPEED_MUL 1.1875F
#define FTREMIX_AIRDODGE_DRAG 0.875F
#define FTREMIX_AIRDODGE_STICK_DEADZONE 10

static sb32 sFTRemixAirDodgeAirDashUsed[GMCOMMON_PLAYERS_MAX];
static sb32 sFTRemixAirDodgeSpecialFlag;

static void ftRemixAirDodgeMovement(GObj *fighter_gobj);
static void ftRemixAirDodgeMain(GObj *fighter_gobj);
static void ftRemixAirDodgeAirDashMain(GObj *fighter_gobj);
static void ftRemixAirDodgeCollision(GObj *fighter_gobj);

/* Moveset scripts: each dw/dh pair of AirDodge.asm is one word, WAIT is 0x0400XXXX, AFTER is 0x0800XXXX, END is 0. */
static const u32 sFTRemixAirDodgeMoveset[] =
{
    0x98002C00, 0x00000000, 0x00000064, 0x00640064,
    0x04000004,
    0x38000060,
    0x74000003,
    0x58000001,
    0x04000002,
    0x98002C00, 0x00000000, 0x00000064, 0x00640064,
    0x08000014,
    0x74000001,
    0x04000003,
    0xD0003EE0,
    0x00000000,
};

static const u32 sFTRemixAirDodgeNeutralMoveset[] =
{
    0x98002C00, 0x00000000, 0x00000064, 0x00640064,
    0x04000004,
    0x38000060,
    0x74000003,
    0x58000001,
    0x04000002,
    0x98002C00, 0x00000000, 0x00000064, 0x00640064,
    0x0800001C,
    0x74000001,
    0x04000003,
    0xD0003EE0,
    0x00000000,
};

static const u32 sFTRemixAirDashMoveset[] =
{
    0x98002C00, 0x00000000, 0x00000064, 0x00640064,
    0x04000004,
    0x38000060,
    0x74000003,
    0x58000001,
    0x04000002,
    0x98002C00, 0x00000000, 0x00000064, 0x00640064,
    0x08000014,
    0x74000001,
    0x00000000,
};

/* AirDodge.asm reset_air_dash_grounded_ (0x800DE420) and reset_air_dash_initial_ (0x800D7DB4) */
void ftRemixAirDodgeResetAirDash(FTStruct *fp)
{
    sFTRemixAirDodgeAirDashUsed[fp->player] = FALSE;
}

/* Clears the absorb and fast-fall bits of byte 0x18D, as the 0x07 mask of AirDodge.asm does */
static void ftRemixAirDodgeClearBitfieldsFromMask(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Sets the moveset of both "played before" scripts, motion_scripts[i][0] */
static void ftRemixAirDodgeSetMoveset(FTStruct *fp, const u32 *moveset)
{
    fp->motion_scripts[0][0].p_script = (u32 *) moveset;
    fp->motion_scripts[1][0].p_script = (u32 *) moveset;
}

/* AirDodge.asm apply_air_movement_: the neutral stick is the square of -10..9 on both axes */
static sb32 ftRemixAirDodgeStickIsNeutral(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    return (stick_x >= -FTREMIX_AIRDODGE_STICK_DEADZONE) && (stick_x < FTREMIX_AIRDODGE_STICK_DEADZONE) &&
        (stick_y >= -FTREMIX_AIRDODGE_STICK_DEADZONE) && (stick_y < FTREMIX_AIRDODGE_STICK_DEADZONE);
}

/* Angle and speed of the stick; the x axis is multiplied by the direction the fighter faces, as in apply_air_movement_ */
static void ftRemixAirDodgeStickVector(FTStruct *fp, f32 *o_angle, f32 *o_speed)
{
    f32 stick_y = (f32) fp->input.pl.stick_range.y;
    f32 stick_x = (f32) (fp->input.pl.stick_range.x * fp->lr);
    f32 magnitude = sqrtf(stick_y * stick_y + stick_x * stick_x);

    *o_speed = (magnitude >= FTREMIX_AIRDODGE_MAX_SPEED) ? FTREMIX_AIRDODGE_MAX_SPEED : magnitude;
    *o_angle = syUtilsArcTan2(stick_y, stick_x);
}

/* AirDodge.asm _movement: the speed is scaled by 1.1875F, then split along the angle, the x axis by the direction */
static void ftRemixAirDodgeSetVelocity(FTStruct *fp, f32 speed, f32 angle)
{
    f32 scaled_speed = speed * FTREMIX_AIRDODGE_SPEED_MUL;

    fp->physics.vel_air.x = scaled_speed * __cosf(angle) * (f32) fp->lr;
    fp->physics.vel_air.y = scaled_speed * __sinf(angle);
}

/* AirDodge.asm apply_air_movement_: a neutral stick keeps the air velocity, except Melee, which uses the default angle */
static void ftRemixAirDodgeApplyAirMovement(FTStruct *fp, s32 type)
{
    f32 angle = FTREMIX_AIRDODGE_DEFAULT_ANGLE;
    f32 speed = 0.0F;

    sFTRemixAirDodgeSpecialFlag = FALSE;

    if (ftRemixAirDodgeStickIsNeutral(fp) == FALSE)
    {
        ftRemixAirDodgeStickVector(fp, &angle, &speed);
        fp->proc_physics = ftRemixAirDodgeMovement;
    }
    else if (type != nFTRemixAirDodgeTypeMelee)
    {
        sFTRemixAirDodgeSpecialFlag = TRUE;
        fp->proc_physics = ftPhysicsApplyAirVelDriftFastFall;
        return;
    }
    ftRemixAirDodgeSetVelocity(fp, speed, angle);
}

/*
 * AirDodge.asm air_dodge_initial_ and air_dash_initial_ start with the same prologue: temp variable 2 is cleared,
 * the absorb and fast-fall bits are cleared, then the fighter changes to DamageLow3.
 * The turnaround (0x800E8044, ftParamSetStickLR) is not ported: the asm passes the GObj in a0, not the player struct,
 * so on N64 it reads GObj+0x1C2 and writes GObj+0x44 and leaves the fighter facing as it was.
 */
static void ftRemixAirDodgeEnterStatus(GObj *fighter_gobj, FTStruct *fp)
{
    fp->motion_vars.flags.flag1 = 0;
    ftRemixAirDodgeClearBitfieldsFromMask(fp);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusDamageLw3, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* AirDodge.asm air_dodge_initial_ (Melee and Ultimate) */
static void ftRemixAirDodgeStartAirDodge(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 type = gmRemixToggle(air_dodge);

    ftRemixAirDodgeEnterStatus(fighter_gobj, fp);
    if (type != nFTRemixAirDodgeTypeUltimate)
    {
        fp->jumps_used = fp->attr->jumps_max;
    }
    ftRemixAirDodgeApplyAirMovement(fp, type);

    if (type == nFTRemixAirDodgeTypeUltimate)
    {
        fp->proc_update = ftRemixAirDodgeAirDashMain;
        ftRemixAirDodgeSetMoveset(fp, sFTRemixAirDodgeSpecialFlag ? sFTRemixAirDodgeNeutralMoveset : sFTRemixAirDodgeMoveset);
    }
    else
    {
        fp->proc_update = ftRemixAirDodgeMain;
        ftRemixAirDodgeSetMoveset(fp, sFTRemixAirDodgeMoveset);
        fp->proc_physics = ftRemixAirDodgeMovement;
    }
    fp->proc_map = ftRemixAirDodgeCollision;
    fp->proc_interrupt = NULL;
    sFTRemixAirDodgeAirDashUsed[fp->player] = TRUE;
}

/* AirDodge.asm air_dash_initial_ (Air Dash): the moveset and routines of a dash that keeps the fall interrupt */
static void ftRemixAirDodgeStartAirDash(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixAirDodgeEnterStatus(fighter_gobj, fp);
    ftRemixAirDodgeApplyAirMovement(fp, nFTRemixAirDodgeTypeAirDash);

    ftRemixAirDodgeSetMoveset(fp, sFTRemixAirDashMoveset);
    fp->proc_update = ftRemixAirDodgeAirDashMain;
    fp->proc_physics = ftRemixAirDodgeMovement;
    fp->proc_map = ftRemixAirDodgeCollision;
    fp->proc_interrupt = ftCommonFallProcInterrupt;
    sFTRemixAirDodgeAirDashUsed[fp->player] = TRUE;
}

/* AirDodge.asm initial_: a port that already used its air dash cannot dodge again until it lands */
static void ftRemixAirDodgeStart(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 type = gmRemixToggle(air_dodge);

    if (sFTRemixAirDodgeAirDashUsed[fp->player] != FALSE)
    {
        return;
    }
    if (type == nFTRemixAirDodgeTypeAirDash)
    {
        ftRemixAirDodgeStartAirDash(fighter_gobj);
        return;
    }
    ftRemixAirDodgeStartAirDodge(fighter_gobj);
}

/*
 * AirDodge.asm airdodge_check (0x801401AC): TRUE when the dodge started (or was blocked by the air dash flag), which
 * makes the jump check return without the jump or the hammer check.
 */
sb32 ftRemixAirDodgeCheckInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 dodge_buttons = fp->input.button_mask_z | R_TRIG;

    if ((fp->input.pl.button_tap & dodge_buttons) == 0)
    {
        return FALSE;
    }
    if (gmRemixToggle(air_dodge) == nFTRemixAirDodgeTypeOff)
    {
        return FALSE;
    }
    /* AirDodge.asm skips the dodge in single button mode (safety check for SBM CPUs) */
    if (gmRemixToggle(single_button_mode) != 0)
    {
        return FALSE;
    }
    if (fp->fkind == nFTKindBoss)
    {
        return FALSE;
    }
    if (fp->status_id == nFTCommonStatusFallSpecial)
    {
        return FALSE;
    }
    if ((fp->input.pl.button_tap & R_TRIG) != 0)
    {
        return FALSE;
    }
    ftRemixAirDodgeStart(fighter_gobj);
    return TRUE;
}

/* AirDodge.asm movement_: the velocity is damped by 0.875F while temp variable 2 is set */
static void ftRemixAirDodgeMovement(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    fp->physics.vel_air.x *= FTREMIX_AIRDODGE_DRAG;
    fp->physics.vel_air.y *= FTREMIX_AIRDODGE_DRAG;
}

/* AirDodge.asm main_: a special fall once the animation has ended */
static void ftRemixAirDodgeMain(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, FTREMIX_AIRDODGE_LANDING_FSM, TRUE);
}

/* AirDodge.asm air_dash_main_: back to the idle or fall status once the animation has ended */
static void ftRemixAirDodgeAirDashMain(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* AirDodge.asm begin_landing_: the landing animation, with interrupts allowed */
static void ftRemixAirDodgeBeginLanding(GObj *fighter_gobj)
{
    ftCommonLandingFallSpecialSetStatus(fighter_gobj, TRUE, FTREMIX_AIRDODGE_LANDING_FSM);
}

/* AirDodge.asm collision_: landing through the common air collision, without ledge grab */
static void ftRemixAirDodgeCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixAirDodgeBeginLanding);
}

#endif
