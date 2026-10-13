/**
 * ftremixmewtwousp.c - C ports of Smash Remix Mewtwo Up Special (Teleport)
 * routines (.toolchain/pkgs/smashremix-src/src/Mewtwo/MewtwoSpecial.asm,
 * scope MewtwoUSP).
 *
 * Actions 0xEA-0xEF are USP_Ground_Begin..USP_Air_End (Mewtwo.asm:314-319).
 * end_collision_ is Samus' up special collision (0x8015DD58) copied from ROM
 * with a Mewtwo landing lag; the port follows the copied bytes in the Remix
 * ROM, which call mpCommonProcFighterOnCliffEdge when grounded.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <sys/utils.h>

#define MTWO_USP_MOVE_SPEED 128.0F
#define MTWO_USP_DEFAULT_ANGLE 1.5707964F      /* 0x3FC90FDB, pi / 2 */
#define MTWO_USP_LANDING_LAG 1.2307693F        /* 0x3F9D89D9 */
#define MTWO_USP_MOVE_TICS 12
#define MTWO_USP_MIN_STICK_MAGNITUDE 11
#define MTWO_USP_FALL_DRIFT 0.9375F
#define MTWO_USP_GROUND_END_VEL_MUL 0.5F
#define MTWO_USP_AIR_END_VEL_MUL 0.25F

enum
{
    nFTRemixMewtwoStatusUSPGroundBegin = 0xEA,
    nFTRemixMewtwoStatusUSPGroundMove = 0xEB,
    nFTRemixMewtwoStatusUSPGroundEnd = 0xEC,
    nFTRemixMewtwoStatusUSPAirBegin = 0xED,
    nFTRemixMewtwoStatusUSPAirMove = 0xEE,
    nFTRemixMewtwoStatusUSPAirEnd = 0xEF
};

extern sb32 ftSamusSpecialHiProcPass(GObj *fighter_gobj);
extern void ftPhysicsClampAirVelXMax(FTStruct *fp);

/* Remix keeps the teleport timer and angle in status vars 0x0B18 and 0x0B20,
 * which have no typed field in FTStatusVars, so they live per port here. */
static s32 sFTRemixMewtwoUSPMoveTics[MAXCONTROLLERS];
static f32 sFTRemixMewtwoUSPMoveAngle[MAXCONTROLLERS];

/* Stick vector length rounded to nearest, matching Remix's cvt.w.s. */
static s32 ftRemixMewtwoUSPStickMagnitude(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    return (s32)(sqrtf((f32)((stick_x * stick_x) + (stick_y * stick_y))) + 0.5F);
}

/* MewtwoUSP.end_invisibility_: invisible while temp variable 3 is set. */
static void ftRemixMewtwoUSPEndInvisibility(FTStruct *fp)
{
    fp->is_invisible = (fp->motion_vars.flags.flag2 != 0);
}

/* Clears the teleport's temp variables 1-3 (motion_vars.flags 0-2). */
static void ftRemixMewtwoUSPResetTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Makes Mewtwo invisible and intangible for the teleport. */
static void ftRemixMewtwoUSPHideIntangible(FTStruct *fp)
{
    fp->is_invisible = TRUE;
    /* REMIX-TODO(CharEnvColor): skipped the moveset_table[port] = 0xFFFFFF00 write; is_invisible is what the port honours. */
    fp->hitstatus = nGMHitStatusIntangible;
}

/* MewtwoUSP.ground_begin_transition_ */
static void ftRemixMewtwoUSPGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPGroundBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* MewtwoUSP.air_begin_transition_ */
static void ftRemixMewtwoUSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPAirBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
}

/* MewtwoUSP.ground_move_transition_ */
static void ftRemixMewtwoUSPGroundMoveTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPGroundMove, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftRemixMewtwoUSPHideIntangible(fp);
}

/* MewtwoUSP.air_move_transition_ */
static void ftRemixMewtwoUSPAirMoveTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPAirMove, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
    sFTRemixMewtwoUSPMoveAngle[fp->player] = 0.0F;
    ftRemixMewtwoUSPHideIntangible(fp);
}

/* MewtwoUSP.ground_begin_initial_ */
static void ftRemixMewtwoUSPGroundBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPGroundBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMewtwoUSPResetTempVars(fp);
    fp->physics.vel_ground.x *= 0.5F;
}

/* MewtwoUSP.air_begin_initial_ */
static void ftRemixMewtwoUSPAirBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPAirBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixMewtwoUSPResetTempVars(fp);
    fp->physics.vel_air.x *= 0.5F;
    fp->physics.vel_air.y *= 0.5F;
    /* Remix masks 0x018D with 0x07: clears is_absorb..is_fastfall. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Action for the teleport's first movement frame. Sets the aerial state where Remix does. */
static s32 ftRemixMewtwoUSPMoveActionId(FTStruct *fp, s32 stick_magnitude)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    if (stick_magnitude < MTWO_USP_MIN_STICK_MAGNITUDE)
    {
        mpCommonSetFighterAir(fp);
        return nFTRemixMewtwoStatusUSPAirMove;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return nFTRemixMewtwoStatusUSPAirMove;
    }
    if ((stick_y != 0) || (stick_x == 0))
    {
        mpCommonSetFighterAir(fp);
        return nFTRemixMewtwoStatusUSPAirMove;
    }
    return nFTRemixMewtwoStatusUSPGroundMove;
}

/* Writes the teleport's movement angle from the stick, or the default angle when the stick is within the deadzone. */
static void ftRemixMewtwoUSPSetMoveAngle(FTStruct *fp, s32 stick_magnitude)
{
    if (stick_magnitude < MTWO_USP_MIN_STICK_MAGNITUDE)
    {
        sFTRemixMewtwoUSPMoveAngle[fp->player] = MTWO_USP_DEFAULT_ANGLE;
        return;
    }
    sFTRemixMewtwoUSPMoveAngle[fp->player] = syUtilsArcTan2(fp->input.pl.stick_range.y, fp->input.pl.stick_range.x * fp->lr);
}

/* Velocity from the movement angle; grounded Remix writes ground x unscaled by direction. */
static void ftRemixMewtwoUSPApplyMovement(FTStruct *fp)
{
    f32 angle = sFTRemixMewtwoUSPMoveAngle[fp->player];
    f32 vel_x = MTWO_USP_MOVE_SPEED * __cosf(angle);
    f32 vel_y = MTWO_USP_MOVE_SPEED * __sinf(angle);

    if (fp->ga == nMPKineticsGround)
    {
        fp->physics.vel_ground.x = vel_x;
        fp->physics.vel_air.x = vel_x * fp->lr;
        return;
    }
    fp->physics.vel_air.x = vel_x * fp->lr;
    fp->physics.vel_air.y = vel_y;
}

/* MewtwoUSP.move_initial_: begins the teleport movement and picks Ground_Move or Air_Move. */
static void ftRemixMewtwoUSPMoveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_grounded_at_start = (fp->ga == nMPKineticsGround);
    s32 stick_magnitude;

    /* REMIX-TODO(PokemonAnnouncer): skipped teleport_announcement_ (announcer line or FGM fallback). */
    if (ABS(fp->input.pl.stick_range.x) >= MTWO_USP_MIN_STICK_MAGNITUDE)
    {
        ftParamSetStickLR(fp);
    }
    stick_magnitude = ftRemixMewtwoUSPStickMagnitude(fp);
    ftMainSetStatus(fighter_gobj, ftRemixMewtwoUSPMoveActionId(fp, stick_magnitude), 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->jumps_used = fp->attr->jumps_max;
    ftRemixMewtwoUSPSetMoveAngle(fp, stick_magnitude);
    sFTRemixMewtwoUSPMoveTics[fp->player] = MTWO_USP_MOVE_TICS;
    ftRemixMewtwoUSPApplyMovement(fp);
    ftRemixMewtwoUSPHideIntangible(fp);
    if (is_grounded_at_start)
    {
        fp->coll_data.ignore_line_id = fp->coll_data.floor_line_id;
    }
}

/* MewtwoUSP.ground_end_initial_ */
static void ftRemixMewtwoUSPGroundEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPGroundEnd, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_ground.x *= MTWO_USP_GROUND_END_VEL_MUL;
}

/* MewtwoUSP.air_end_initial_ */
static void ftRemixMewtwoUSPAirEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusUSPAirEnd, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.x *= MTWO_USP_AIR_END_VEL_MUL;
    fp->physics.vel_air.y *= MTWO_USP_AIR_END_VEL_MUL;
}

/* MewtwoUSP.begin_main_ */
static void ftRemixMewtwoUSPBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixMewtwoUSPMoveSetStatus);
}

/* MewtwoUSP.move_main_: counts the teleport down and moves on to the end action. */
static void ftRemixMewtwoUSPMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    sFTRemixMewtwoUSPMoveTics[fp->player]--;
    if (sFTRemixMewtwoUSPMoveTics[fp->player] != 0)
    {
        return;
    }
    if (fp->ga != nMPKineticsGround)
    {
        ftRemixMewtwoUSPAirEndSetStatus(fighter_gobj);
    }
    else ftRemixMewtwoUSPGroundEndSetStatus(fighter_gobj);
}

/* MewtwoUSP.move_physics_ */
static void ftRemixMewtwoUSPMoveProcPhysics(GObj *fighter_gobj)
{
    ftRemixMewtwoUSPApplyMovement(ftGetStruct(fighter_gobj));
}

/* MewtwoUSP.ground_end_main_ */
static void ftRemixMewtwoUSPGroundEndProcUpdate(GObj *fighter_gobj)
{
    ftRemixMewtwoUSPEndInvisibility(ftGetStruct(fighter_gobj));
    ftAnimEndSetWait(fighter_gobj);
}

/* MewtwoUSP.air_end_main_: special fall once the animation has ended. */
static void ftRemixMewtwoUSPAirEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMewtwoUSPEndInvisibility(fp);
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, MTWO_USP_FALL_DRIFT, FALSE, TRUE, FALSE, MTWO_USP_LANDING_LAG, FALSE);
}

/* MewtwoUSP.ground_begin_collision_ */
static void ftRemixMewtwoUSPGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMewtwoUSPAirBeginTransition);
}

/* MewtwoUSP.air_begin_collision_ */
static void ftRemixMewtwoUSPAirBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixMewtwoUSPGroundBeginTransition);
}

/* MewtwoUSP.ground_move_collision_ */
static void ftRemixMewtwoUSPGroundMoveProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixMewtwoUSPAirMoveTransition);
}

/* MewtwoUSP.air_move_collision_ */
static void ftRemixMewtwoUSPAirMoveProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixMewtwoUSPGroundMoveTransition);
}

/* MewtwoUSP.end_collision_: Samus' up special collision with Mewtwo's landing lag. */
static void ftRemixMewtwoUSPEndProcMap(GObj *fighter_gobj)
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
    else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, MTWO_USP_LANDING_LAG);
}

/* Remix address -> port routine for every MewtwoUSP routine in the Remix action array or special tables. */
const FTRemixProc dFTRemixMewtwoUSPProcs[] =
{
    { REMIX_SYM_MewtwoUSP__air_begin_collision_, ftRemixMewtwoUSPAirBeginProcMap },
    { REMIX_SYM_MewtwoUSP__air_begin_initial_, ftRemixMewtwoUSPAirBeginSetStatus },
    { REMIX_SYM_MewtwoUSP__air_end_main_, ftRemixMewtwoUSPAirEndProcUpdate },
    { REMIX_SYM_MewtwoUSP__air_move_collision_, ftRemixMewtwoUSPAirMoveProcMap },
    { REMIX_SYM_MewtwoUSP__begin_main_, ftRemixMewtwoUSPBeginProcUpdate },
    { REMIX_SYM_MewtwoUSP__end_collision_, ftRemixMewtwoUSPEndProcMap },
    { REMIX_SYM_MewtwoUSP__ground_begin_collision_, ftRemixMewtwoUSPGroundBeginProcMap },
    { REMIX_SYM_MewtwoUSP__ground_begin_initial_, ftRemixMewtwoUSPGroundBeginSetStatus },
    { REMIX_SYM_MewtwoUSP__ground_end_main_, ftRemixMewtwoUSPGroundEndProcUpdate },
    { REMIX_SYM_MewtwoUSP__ground_move_collision_, ftRemixMewtwoUSPGroundMoveProcMap },
    { REMIX_SYM_MewtwoUSP__move_main_, ftRemixMewtwoUSPMoveProcUpdate },
    { REMIX_SYM_MewtwoUSP__move_physics_, ftRemixMewtwoUSPMoveProcPhysics }
};

const s32 dFTRemixMewtwoUSPProcsNum = ARRAY_COUNT(dFTRemixMewtwoUSPProcs);

#endif
