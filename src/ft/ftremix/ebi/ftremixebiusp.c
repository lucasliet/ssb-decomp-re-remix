/**
 * ftremixebiusp.c - C ports of Smash Remix Ebisumaru up special routines
 * (.toolchain/pkgs/smashremix-src/src/Ebi/EbiSpecial.asm, scope EbiUSP, and the
 * EbiUSPRefresh hook in the same file).
 *
 * The collision routine splices Mario's up special landing check (0x80156358).
 * See build/remixasm/notes/46_EBI_USP.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_symbols.h"
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>
#include <sys/utils.h>

enum
{
    nFTRemixEbiStatusUSP = 0xDF
};

#define EBI_USP_GROUND_SPEED 80.0F                    /* 0x42A00000 */
#define EBI_USP_AIR_INITIAL_Y_SCALE 0.375F            /* 0x3EC00000 */
#define EBI_USP_ACCELERATION 2.0F                     /* 0x40000000 */
#define EBI_USP_MIN_SPEED 3.0F                        /* 0x40400000 */
#define EBI_USP_STICK_SCALE 0.53125F                  /* 0x3F080000 */
#define EBI_USP_MAX_TARGET_SPEED 42.5F                /* 0x422A0000: 80 * 0.53125 */
#define EBI_USP_STICK_DEADZONE 11
#define EBI_USP_MAX_TIME 80
#define EBI_USP_LANDING_LAG 0.2507031261920929F       /* 0x3E805C29: lui 0x3E80 + Mario's ori 0x5C29 */

/* N64 FTStruct offsets of the Remix scratch words this USP uses (see ftremixfields.h). */
#define EBI_USP_N64_UP_SPECIAL_BOOL 0x0ADC
#define EBI_USP_N64_FRAME_TIMER 0x0B18
#define EBI_USP_N64_ON_HIT_WORD 0x0B20

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);


/* EbiUSP.goemon_on_hit_subroutine_: the asm only loads the player struct and returns, so nothing happens on hit. */
static void ftRemixEbiUSPOnHit(GObj *fighter_gobj)
{
    (void) fighter_gobj;
}

/* EbiUSPRefresh: clears the up special bool, so the USP is usable again after a hit. Called from ftRemixRunOnHit. */
void ftRemixEbiUSPRefresh(FTStruct *fp)
{
    *ftRemixFighterWord(fp, EBI_USP_N64_UP_SPECIAL_BOOL) = FALSE;
}

/* ftMainSetStatus(gobj, USP, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE) followed by ftMainPlayAnimEventsAll. */
static void ftRemixEbiUSPSetStatus(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTRemixEbiStatusUSP, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* EbiUSP.goemon_on_hit_subroutine_establishment_: installs the on-hit routine and clears its two words. */
static void ftRemixEbiUSPOnHitEstablishment(FTStruct *fp)
{
    fp->proc_damage = ftRemixEbiUSPOnHit;
    *ftRemixFighterWord(fp, EBI_USP_N64_ON_HIT_WORD) = 0;
    fp->motion_vars.flags.flag0 = 0;
}

/* Clears the fast fall and absorb bits of byte 0x018D. The asm stores `v1 & 7` there, where v1 was
 * clobbered by the calls before it, so the three low flags (is_magnify_show, is_limit_map_bounds,
 * is_invisible) are left as they are. Same choice as the Goemon USP port. */
static void ftRemixEbiUSPClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Clears the frame timer and temp variables 1-3 (temp variable 2 = 1), clears the air flags and refills the jumps. */
static void ftRemixEbiUSPInitStatusVars(FTStruct *fp)
{
    *ftRemixFighterWord(fp, EBI_USP_N64_FRAME_TIMER) = 0;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 1;
    fp->motion_vars.flags.flag2 = 0;
    ftRemixEbiUSPClearAirFlags(fp);
    fp->jumps_used = fp->attr->jumps_max;
}

/* EbiUSP.air_initial_: runs once per USP (up special bool at 0x0ADC), then sets up the status and scales y velocity. */
static void ftRemixEbiUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *up_special_bool = ftRemixFighterWord(fp, EBI_USP_N64_UP_SPECIAL_BOOL);
    f32 vel_y;

    if (*up_special_bool != FALSE)
    {
        return;
    }
    *up_special_bool = TRUE;
    ftRemixEbiUSPSetStatus(fighter_gobj);
    ftRemixEbiUSPOnHitEstablishment(fp);

    vel_y = fp->physics.vel_air.y * EBI_USP_AIR_INITIAL_Y_SCALE;
    ftRemixEbiUSPInitStatusVars(fp);
    fp->physics.vel_air.y = vel_y;
}

/* EbiUSP.ground_initial_: launches the grounded start upward, then runs the air initial. */
static void ftRemixEbiUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->physics.vel_air.y = EBI_USP_GROUND_SPEED;
    mpCommonSetFighterAir(fp);
    ftRemixEbiUSPAirInitial(fighter_gobj);
}

/* EbiUSP.main_: after MAX_TIME frames, transitions to the wait or fall status. */
static void ftRemixEbiUSPMainProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *frame_timer = ftRemixFighterWord(fp, EBI_USP_N64_FRAME_TIMER);
    s32 timer_before = *frame_timer;

    *frame_timer = timer_before + 1;
    if (timer_before < EBI_USP_MAX_TIME)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* Returns TRUE when both stick axes are inside the deadzone (|x| and |y| < 11). */
static sb32 ftRemixEbiUSPIsStickNeutral(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    return (ABS(stick_x) < EBI_USP_STICK_DEADZONE) && (ABS(stick_y) < EBI_USP_STICK_DEADZONE);
}

/* Stops Ebi when its speed is at most MIN_SPEED. Returns TRUE if it stopped. */
static sb32 ftRemixEbiUSPStopBelowMinSpeed(FTStruct *fp)
{
    f32 vel_x = fp->physics.vel_air.x;
    f32 vel_y = fp->physics.vel_air.y;
    f32 speed = sqrtf(vel_x * vel_x + vel_y * vel_y);

    if (speed > EBI_USP_MIN_SPEED)
    {
        return FALSE;
    }
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    return TRUE;
}

/* Target velocity for the stick, scaled down to MAX_TARGET_SPEED when its length exceeds it. */
static Vec2f ftRemixEbiUSPGetStickTarget(FTStruct *fp)
{
    Vec2f target;
    f32 target_length;

    target.x = (f32) fp->input.pl.stick_range.x * EBI_USP_STICK_SCALE;
    target.y = (f32) fp->input.pl.stick_range.y * EBI_USP_STICK_SCALE;
    target_length = sqrtf(target.x * target.x + target.y * target.y);
    if (target_length >= EBI_USP_MAX_TARGET_SPEED)
    {
        f32 scale = EBI_USP_MAX_TARGET_SPEED / target_length;

        target.x *= scale;
        target.y *= scale;
    }
    return target;
}

/* Adds ACCELERATION along the angle from the current air velocity to target. */
static void ftRemixEbiUSPAccelerateToward(FTStruct *fp, Vec2f target)
{
    f32 diff_x = target.x - fp->physics.vel_air.x;
    f32 diff_y = target.y - fp->physics.vel_air.y;
    f32 angle = syUtilsArcTan2(diff_y, diff_x);

    fp->physics.vel_air.x += EBI_USP_ACCELERATION * __cosf(angle);
    fp->physics.vel_air.y += EBI_USP_ACCELERATION * __sinf(angle);
}

/* EbiUSP.physics_: steers toward the stick's target velocity, or toward rest when the stick is neutral. */
static void ftRemixEbiUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec2f target = { 0.0F, 0.0F };

    if (ftRemixEbiUSPIsStickNeutral(fp))
    {
        if (ftRemixEbiUSPStopBelowMinSpeed(fp))
        {
            return;
        }
    }
    else
    {
        target = ftRemixEbiUSPGetStickTarget(fp);
    }
    ftRemixEbiUSPAccelerateToward(fp, target);
}

/* Landing check of ftMarioSpecialHiProcMap (0x80156358) with Ebi's landing lag. */
static void ftRemixEbiUSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsAir)
    {
        if ((fp->motion_vars.flags.flag1 == 0) || (fp->physics.vel_air.y >= 0.0F))
        {
            mpCommonCheckFighterProject(fighter_gobj);
        }
        else if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) != FALSE)
        {
            if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
            {
                ftCommonCliffCatchSetStatus(fighter_gobj);
            }
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, EBI_USP_LANDING_LAG);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* Remix address -> port routine for every EbiUSP routine in the worklist. */
const FTRemixProc dFTRemixEbiUSPProcs[] =
{
    { REMIX_SYM_EbiUSP__air_initial_, ftRemixEbiUSPAirInitial },
    { REMIX_SYM_EbiUSP__collision_, ftRemixEbiUSPProcMap },
    { REMIX_SYM_EbiUSP__ground_initial_, ftRemixEbiUSPGroundInitial },
    { REMIX_SYM_EbiUSP__main_, ftRemixEbiUSPMainProcUpdate },
    { REMIX_SYM_EbiUSP__physics_, ftRemixEbiUSPProcPhysics }
};

const s32 dFTRemixEbiUSPProcsNum = ARRAY_COUNT(dFTRemixEbiUSPProcs);

#endif
