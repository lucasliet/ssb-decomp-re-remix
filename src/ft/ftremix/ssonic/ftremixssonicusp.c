/**
 * ftremixssonicusp.c - C ports of Smash Remix Super Sonic Up Special (Spring
 * Teleport) routines (.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm,
 * scope SSonicUSP, lines 2901-3592).
 *
 * Actions 0x100-0x105 are USP_Ground_Begin..USP_Air_End (Remix action array at
 * 0x804C2020, entries 36-41). end_collision_ is Samus' up special collision
 * (0x8015DD58) copied from ROM with a landing lag of 1.5.
 */

#ifdef PORT

#include "ft/ftremix/ftremixenvcolor.h"
#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <mp/mpcommon.h>
#include <sys/utils.h>

#define SSONIC_USP_SPEED 160.0F                 /* 0x43200000 */
#define SSONIC_USP_DEFAULT_ANGLE 1.5707964F     /* 0x3FC90FDB, pi / 2 */
#define SSONIC_USP_LANDING_LAG 1.5F             /* 0x3FC00000 */
#define SSONIC_USP_MOVE_TICS 12
#define SSONIC_USP_MIN_STICK_MAGNITUDE 11
#define SSONIC_USP_CANCEL_MIN_STICK_Y 40
#define SSONIC_USP_MAX_AIR_BEGINS 2
#define SSONIC_USP_GROUND_BEGIN_VEL_MUL 0.5F
#define SSONIC_USP_AIR_BEGIN_VEL_MUL 0.5F
#define SSONIC_USP_GROUND_END_VEL_MUL 0.5F
#define SSONIC_USP_AIR_END_VEL_MUL 0.25F

/* N64 FTStruct offsets of the Remix scratch words (see ftremixfields.h). */
#define SSONIC_USP_N64_UP_SPECIAL_COUNT 0x0ADC
#define SSONIC_USP_N64_MOVE_TIC 0x0B18
#define SSONIC_USP_N64_MOVE_ANGLE 0x0B20

enum
{
    nFTRemixSSonicStatusUSPGroundBegin = 0x100,
    nFTRemixSSonicStatusUSPGroundMove = 0x101,
    nFTRemixSSonicStatusUSPGroundEnd = 0x102,
    nFTRemixSSonicStatusUSPAirBegin = 0x103,
    nFTRemixSSonicStatusUSPAirMove = 0x104,
    nFTRemixSSonicStatusUSPAirEnd = 0x105
};

extern sb32 ftSamusSpecialHiProcPass(GObj *fighter_gobj);
extern void ftPhysicsClampAirVelXMax(FTStruct *fp);

/* Stick vector length rounded to nearest, matching Remix's cvt.w.s. */
static s32 ftRemixSSonicUSPStickMagnitude(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    return (s32)(sqrtf((f32)((stick_x * stick_x) + (stick_y * stick_y))) + 0.5F);
}

/* SSonicUSP.end_invisibility_: invisible while temp variable 3 is set. */
static void ftRemixSSonicUSPEndInvisibility(FTStruct *fp)
{
    fp->is_invisible = (fp->motion_vars.flags.flag2 != 0);
    ftRemixEnvColorSetMoveset(fp->player, (fp->motion_vars.flags.flag2 != 0) ? 0xFFFFFF00 : 0);
}

/* Clears temp variables 1-3 (motion_vars.flags 0-2). */
static void ftRemixSSonicUSPResetTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Makes Super Sonic invisible and intangible for the teleport. */
static void ftRemixSSonicUSPHideIntangible(FTStruct *fp)
{
    fp->is_invisible = TRUE;
    ftRemixEnvColorSetMoveset(fp->player, 0xFFFFFF00);
    /* N64 0x05BB is the low byte of the big-endian hitstatus word at 0x05B8, so this matches `sb 0x0003, 0x05BB`. */
    fp->hitstatus = nGMHitStatusIntangible;
}

/* SSonicUSP.ground_begin_transition_ */
static void ftRemixSSonicUSPGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPGroundBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* SSonicUSP.air_begin_transition_ */
static void ftRemixSSonicUSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPAirBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
}

/* SSonicUSP.ground_move_transition_ */
static void ftRemixSSonicUSPGroundMoveTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPGroundMove, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftRemixSSonicUSPHideIntangible(fp);
}

/* SSonicUSP.air_move_transition_ */
static void ftRemixSSonicUSPAirMoveTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPAirMove, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
    *ftRemixFighterFloat(fp, SSONIC_USP_N64_MOVE_ANGLE) = 0.0F;
    ftRemixSSonicUSPHideIntangible(fp);
}

/* SSonicUSP.ground_begin_initial_ */
static void ftRemixSSonicUSPGroundBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPGroundBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSSonicUSPResetTempVars(fp);
    fp->physics.vel_ground.x *= SSONIC_USP_GROUND_BEGIN_VEL_MUL;
}

/* SSonicUSP.air_begin_initial_: counts the up special, then starts the aerial begin action. */
static void ftRemixSSonicUSPAirBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *up_special_count = ftRemixFighterWord(fp, SSONIC_USP_N64_UP_SPECIAL_COUNT);

    if (*up_special_count >= SSONIC_USP_MAX_AIR_BEGINS)
    {
        return;
    }
    (*up_special_count)++;

    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPAirBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSSonicUSPResetTempVars(fp);
    fp->physics.vel_air.x *= SSONIC_USP_AIR_BEGIN_VEL_MUL;
    fp->physics.vel_air.y *= SSONIC_USP_AIR_BEGIN_VEL_MUL;
    /* Remix masks 0x018D with 0x07: clears is_absorb..is_fastfall. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Action for the teleport's first movement frame. Sets the aerial state where Remix does. */
static s32 ftRemixSSonicUSPMoveActionId(FTStruct *fp, s32 stick_magnitude)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    if (stick_magnitude < SSONIC_USP_MIN_STICK_MAGNITUDE)
    {
        mpCommonSetFighterAir(fp);
        return nFTRemixSSonicStatusUSPAirMove;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return nFTRemixSSonicStatusUSPAirMove;
    }
    if ((stick_y != 0) || (stick_x == 0))
    {
        mpCommonSetFighterAir(fp);
        return nFTRemixSSonicStatusUSPAirMove;
    }
    return nFTRemixSSonicStatusUSPGroundMove;
}

/* Writes the movement angle from the stick, or the default angle when the stick is within the deadzone. */
static void ftRemixSSonicUSPSetMoveAngle(FTStruct *fp, s32 stick_magnitude)
{
    f32 *move_angle = ftRemixFighterFloat(fp, SSONIC_USP_N64_MOVE_ANGLE);

    *move_angle = SSONIC_USP_DEFAULT_ANGLE;
    if (stick_magnitude < SSONIC_USP_MIN_STICK_MAGNITUDE)
    {
        return;
    }
    *move_angle = syUtilsArcTan2(fp->input.pl.stick_range.y, fp->input.pl.stick_range.x * fp->lr);
}

/* SSonicUSP.apply_movement_: velocity from the movement angle; grounded Remix writes ground x unscaled by direction. */
static void ftRemixSSonicUSPApplyMovement(FTStruct *fp)
{
    f32 angle = *ftRemixFighterFloat(fp, SSONIC_USP_N64_MOVE_ANGLE);
    f32 vel_x = SSONIC_USP_SPEED * __cosf(angle);
    f32 vel_y = SSONIC_USP_SPEED * __sinf(angle);

    if (fp->ga == nMPKineticsGround)
    {
        fp->physics.vel_ground.x = vel_x;
        fp->physics.vel_air.x = vel_x * fp->lr;
        return;
    }
    fp->physics.vel_air.x = vel_x * fp->lr;
    fp->physics.vel_air.y = vel_y;
}

/* SSonicUSP.move_initial_: starts the teleport movement and picks Ground_Move or Air_Move. */
static void ftRemixSSonicUSPMoveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_grounded_at_start = (fp->ga == nMPKineticsGround);
    s32 stick_magnitude;

    /* REMIX-TODO(PokemonAnnouncer): skipped teleport_announcement_ (announcer line or FGM fallback). */
    if (ABS(fp->input.pl.stick_range.x) >= SSONIC_USP_MIN_STICK_MAGNITUDE)
    {
        ftParamSetStickLR(fp);
    }
    stick_magnitude = ftRemixSSonicUSPStickMagnitude(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSSonicUSPMoveActionId(fp, stick_magnitude), 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSSonicUSPSetMoveAngle(fp, stick_magnitude);
    *ftRemixFighterWord(fp, SSONIC_USP_N64_MOVE_TIC) = SSONIC_USP_MOVE_TICS;
    ftRemixSSonicUSPApplyMovement(fp);
    ftRemixSSonicUSPHideIntangible(fp);
    if (is_grounded_at_start)
    {
        fp->coll_data.ignore_line_id = fp->coll_data.floor_line_id;
    }
}

/* SSonicUSP.ground_end_initial_ */
static void ftRemixSSonicUSPGroundEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPGroundEnd, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_ground.x *= SSONIC_USP_GROUND_END_VEL_MUL;
}

/* SSonicUSP.air_end_initial_ */
static void ftRemixSSonicUSPAirEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSSonicStatusUSPAirEnd, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.x *= SSONIC_USP_AIR_END_VEL_MUL;
    fp->physics.vel_air.y *= SSONIC_USP_AIR_END_VEL_MUL;
}

/* SSonicUSP.begin_main_ */
static void ftRemixSSonicUSPBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixSSonicUSPMoveSetStatus);
}

/* SSonicUSP.move_main_: counts the teleport down and moves on to the end action. */
static void ftRemixSSonicUSPMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *move_tic = ftRemixFighterWord(fp, SSONIC_USP_N64_MOVE_TIC);

    (*move_tic)--;
    if (*move_tic != 0)
    {
        return;
    }
    if (fp->ga != nMPKineticsGround)
    {
        ftRemixSSonicUSPAirEndSetStatus(fighter_gobj);
    }
    else ftRemixSSonicUSPGroundEndSetStatus(fighter_gobj);
}

/* SSonicUSP.move_physics_ */
static void ftRemixSSonicUSPMoveProcPhysics(GObj *fighter_gobj)
{
    ftRemixSSonicUSPApplyMovement(ftGetStruct(fighter_gobj));
}

/* SSonicUSP.ground_begin_collision_ */
static void ftRemixSSonicUSPGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSSonicUSPAirBeginTransition);
}

/* SSonicUSP.air_begin_collision_ */
static void ftRemixSSonicUSPAirBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixSSonicUSPGroundBeginTransition);
}

/* SSonicUSP.ground_move_collision_ */
static void ftRemixSSonicUSPGroundMoveProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixSSonicUSPAirMoveTransition);
}

/* SSonicUSP.air_move_collision_ */
static void ftRemixSSonicUSPAirMoveProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixSSonicUSPGroundMoveTransition);
}

/* SSonicUSP.air_end_main_: waits for the animation, then sets wait or fall. */
static void ftRemixSSonicUSPAirEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSSonicUSPEndInvisibility(fp);
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    mpCommonSetFighterWaitOrFall(fighter_gobj);
}

/* SSonicUSP.ground_end_main_ */
static void ftRemixSSonicUSPGroundEndProcUpdate(GObj *fighter_gobj)
{
    ftRemixSSonicUSPEndInvisibility(ftGetStruct(fighter_gobj));
    ftAnimEndSetWait(fighter_gobj);
}

/* SSonicUSP.ground_end_interrupt_: allows interrupts once temp variable 2 is set. */
static void ftRemixSSonicUSPGroundEndProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    ftCommonSquatRvProcInterrupt(fighter_gobj);
}

/* SSonicUSP.air_end_interrupt_: allows interrupts once temp variable 2 is set. */
static void ftRemixSSonicUSPAirEndProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    ftCommonJumpProcInterrupt(fighter_gobj);
}

/* SSonicUSP.end_collision_: Samus' up special collision with a landing lag of 1.5. */
static void ftRemixSSonicUSPEndProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsAir)
    {
        mpCommonProcFighterOnCliffEdge(fighter_gobj);
        return;
    }
    if (fp->physics.vel_air.y >= 0.0F)
    {
        mpCommonCheckFighterProject(fighter_gobj);
        return;
    }
    if (mpCommonCheckFighterPassCliff(fighter_gobj, ftSamusSpecialHiProcPass) == FALSE)
    {
        return;
    }
    if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
    {
        ftCommonCliffCatchSetStatus(fighter_gobj);
    }
    else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, SSONIC_USP_LANDING_LAG);
}

/* Remix's Character.get_hitbox_collision_flags_ masked with 0x00F0: any interact bit on an active attack coll. */
static sb32 ftRemixSSonicUSPAttackCollHasInteraction(FTAttackColl *attack_coll)
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

/* True when any of the fighter's attack colls has interacted with something this frame. */
static sb32 ftRemixSSonicUSPHitboxCollided(FTStruct *fp)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(fp->attack_colls); i++)
    {
        if (ftRemixSSonicUSPAttackCollHasInteraction(&fp->attack_colls[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Remix teleport_cancel_patch_ conditions: Super Sonic, stick_y >= 40, B tapped, and a hitbox interaction. */
static sb32 ftRemixSSonicUSPIsTeleportCancel(FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_SSONIC)
    {
        return FALSE;
    }
    if (fp->input.pl.stick_range.y < SSONIC_USP_CANCEL_MIN_STICK_Y)
    {
        return FALSE;
    }
    if (!(fp->input.pl.button_tap & B_BUTTON))
    {
        return FALSE;
    }
    return ftRemixSSonicUSPHitboxCollided(fp);
}

/*
 * Vanilla patch entry, called from ftMainProcUpdateInterrupt in place of its
 * proc_interrupt call (RAM 0x800E1998, see the notes file). Remix
 * teleport_cancel_patch_ (SonicSpecial.asm:3595): cancels the interrupt into
 * the up special, otherwise runs the original interrupt.
 */
void ftRemixSSonicUSPInterruptPatch(GObj *fighter_gobj, void (*proc_interrupt)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixSSonicUSPIsTeleportCancel(fp))
    {
        ftRemixSSonicUSPAirBeginSetStatus(fighter_gobj);
        return;
    }
    proc_interrupt(fighter_gobj);
}

/* Remix address -> port routine for every SSonicUSP routine in the worklist. */
const FTRemixProc dFTRemixSSonicUSPProcs[] =
{
    { REMIX_SYM_SSonicUSP__air_begin_collision_, ftRemixSSonicUSPAirBeginProcMap },
    { REMIX_SYM_SSonicUSP__air_begin_initial_, ftRemixSSonicUSPAirBeginSetStatus },
    { REMIX_SYM_SSonicUSP__air_end_interrupt_, ftRemixSSonicUSPAirEndProcInterrupt },
    { REMIX_SYM_SSonicUSP__air_end_main_, ftRemixSSonicUSPAirEndProcUpdate },
    { REMIX_SYM_SSonicUSP__air_move_collision_, ftRemixSSonicUSPAirMoveProcMap },
    { REMIX_SYM_SSonicUSP__begin_main_, ftRemixSSonicUSPBeginProcUpdate },
    { REMIX_SYM_SSonicUSP__end_collision_, ftRemixSSonicUSPEndProcMap },
    { REMIX_SYM_SSonicUSP__ground_begin_collision_, ftRemixSSonicUSPGroundBeginProcMap },
    { REMIX_SYM_SSonicUSP__ground_begin_initial_, ftRemixSSonicUSPGroundBeginSetStatus },
    { REMIX_SYM_SSonicUSP__ground_end_interrupt_, ftRemixSSonicUSPGroundEndProcInterrupt },
    { REMIX_SYM_SSonicUSP__ground_end_main_, ftRemixSSonicUSPGroundEndProcUpdate },
    { REMIX_SYM_SSonicUSP__ground_move_collision_, ftRemixSSonicUSPGroundMoveProcMap },
    { REMIX_SYM_SSonicUSP__move_main_, ftRemixSSonicUSPMoveProcUpdate },
    { REMIX_SYM_SSonicUSP__move_physics_, ftRemixSSonicUSPMoveProcPhysics }
};

const s32 dFTRemixSSonicUSPProcsNum = ARRAY_COUNT(dFTRemixSSonicUSPProcs);

#endif
