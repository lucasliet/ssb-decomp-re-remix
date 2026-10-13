/**
 * ftremixsheikusp.c - C ports of Smash Remix Sheik Up Special (Vanish)
 * routines (.toolchain/pkgs/smashremix-src/src/Sheik/SheikSpecial.asm,
 * scope SheikUSP).
 *
 * Actions 0xE4-0xE9 are USPG_BEGIN..USPA_END (Sheik.asm:114-119).
 * end_collision_ is Samus' up special collision (0x8015DD58) copied from ROM
 * with a Sheik landing lag; the port follows the copied bytes in the Remix
 * ROM, which call mpCommonProcFighterOnCliffEdge when grounded.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftphysics.h>
#include <sys/utils.h>

#define SHEIK_USP_MOVE_SPEED 280.0F
#define SHEIK_USP_DEFAULT_ANGLE 1.5707964F          /* 0x3FC90FDB, pi / 2 */
#define SHEIK_USP_INITIAL_Y_SPEED 75.0F             /* 0x42960000 */
#define SHEIK_USP_LANDING_LAG 0.3695068359375F      /* 0x3EBD3000 */
#define SHEIK_USP_MOVE_TICS 12
#define SHEIK_USP_MOVE_TICS_APPLY_MAX 6
#define SHEIK_USP_MIN_STICK_MAGNITUDE 11
#define SHEIK_USP_END_DRIFT_STICK_MIN 8
#define SHEIK_USP_FALL_DRIFT 0.9375F                /* 0x3F700000 */
#define SHEIK_USP_FALL_LANDING_LAG 0.5F             /* 0x3F000000 */
#define SHEIK_USP_BEGIN_GROUND_VEL_MUL 0.5F
#define SHEIK_USP_BEGIN_AIR_VEL_MUL 0.5F
#define SHEIK_USP_GROUND_END_VEL_MUL 0.25F          /* 0x3E800000 */
#define SHEIK_USP_AIR_END_VEL_MUL 0.109375F         /* 0x3DE00000 */

enum
{
    nFTRemixSheikStatusUSPGroundBegin = 0xE4,
    nFTRemixSheikStatusUSPGroundMove = 0xE5,
    nFTRemixSheikStatusUSPGroundEnd = 0xE6,
    nFTRemixSheikStatusUSPAirBegin = 0xE7,
    nFTRemixSheikStatusUSPAirMove = 0xE8,
    nFTRemixSheikStatusUSPAirEnd = 0xE9
};

extern sb32 ftSamusSpecialHiProcPass(GObj *fighter_gobj);

/* Remix keeps the vanish timer and angle in status vars 0x0B18 and 0x0B20,
 * which have no typed field in FTStatusVars, so they live per port here. */
static s32 sFTRemixSheikUSPMoveTics[MAXCONTROLLERS];
static f32 sFTRemixSheikUSPMoveAngle[MAXCONTROLLERS];

/* Stick vector length rounded to nearest, matching Remix's cvt.w.s. */
static s32 ftRemixSheikUSPStickMagnitude(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    return (s32)(sqrtf((f32)((stick_x * stick_x) + (stick_y * stick_y))) + 0.5F);
}

/* SheikUSP.end_invisibility_: invisible while temp variable 3 is set. */
static void ftRemixSheikUSPEndInvisibility(FTStruct *fp)
{
    fp->is_invisible = (fp->motion_vars.flags.flag2 != 0);
}

/* Clears the vanish's temp variables 1-3 (motion_vars.flags 0-2). */
static void ftRemixSheikUSPResetTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Makes Sheik invisible and intangible for the vanish. */
static void ftRemixSheikUSPHideIntangible(FTStruct *fp)
{
    fp->is_invisible = TRUE;
    /* REMIX-TODO(CharEnvColor): skipped the moveset_table[port] = 0xFFFFFF00 write; is_invisible is what the port honours. */
    fp->hitstatus = nGMHitStatusIntangible;
}

/* SheikUSP.ground_begin_transition_ */
static void ftRemixSheikUSPGroundBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPGroundBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* SheikUSP.air_begin_transition_ */
static void ftRemixSheikUSPAirBeginTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPAirBegin, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
}

/* SheikUSP.ground_move_transition_ */
static void ftRemixSheikUSPGroundMoveTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPGroundMove, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftRemixSheikUSPHideIntangible(fp);
}

/* SheikUSP.air_move_transition_ */
static void ftRemixSheikUSPAirMoveTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPAirMove, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftPhysicsClampAirVelXMax(fp);
    sFTRemixSheikUSPMoveAngle[fp->player] = 0.0F;
    ftRemixSheikUSPHideIntangible(fp);
}

/* SheikUSP.ground_begin_initial_ */
static void ftRemixSheikUSPGroundBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPGroundBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSheikUSPResetTempVars(fp);
    fp->physics.vel_ground.x *= SHEIK_USP_BEGIN_GROUND_VEL_MUL;
}

/* SheikUSP.air_begin_initial_ */
static void ftRemixSheikUSPAirBeginSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPAirBegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixSheikUSPResetTempVars(fp);
    fp->physics.vel_air.x *= SHEIK_USP_BEGIN_AIR_VEL_MUL;
    fp->physics.vel_air.y = SHEIK_USP_INITIAL_Y_SPEED;
    /* Remix masks 0x018D with 0x07: clears is_absorb..is_fastfall. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Action for the vanish's first movement frame. Sets the aerial state where Remix does. */
static s32 ftRemixSheikUSPMoveActionId(FTStruct *fp, s32 stick_magnitude)
{
    s32 stick_x = fp->input.pl.stick_range.x;
    s32 stick_y = fp->input.pl.stick_range.y;

    if (stick_magnitude < SHEIK_USP_MIN_STICK_MAGNITUDE)
    {
        mpCommonSetFighterAir(fp);
        return nFTRemixSheikStatusUSPAirMove;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return nFTRemixSheikStatusUSPAirMove;
    }
    if ((stick_y != 0) || (stick_x == 0))
    {
        mpCommonSetFighterAir(fp);
        return nFTRemixSheikStatusUSPAirMove;
    }
    return nFTRemixSheikStatusUSPGroundMove;
}

/* Writes the vanish's movement angle from the stick, or the default angle when the stick is within the deadzone. */
static void ftRemixSheikUSPSetMoveAngle(FTStruct *fp, s32 stick_magnitude)
{
    if (stick_magnitude < SHEIK_USP_MIN_STICK_MAGNITUDE)
    {
        sFTRemixSheikUSPMoveAngle[fp->player] = SHEIK_USP_DEFAULT_ANGLE;
        return;
    }
    sFTRemixSheikUSPMoveAngle[fp->player] = syUtilsArcTan2(fp->input.pl.stick_range.y, fp->input.pl.stick_range.x * fp->lr);
}

/* SheikUSP.apply_movement_: velocity from the movement angle, only during the last ticks of the timer. */
static void ftRemixSheikUSPApplyMovement(FTStruct *fp)
{
    f32 angle;
    f32 vel_x;
    f32 vel_y;

    if (sFTRemixSheikUSPMoveTics[fp->player] >= SHEIK_USP_MOVE_TICS_APPLY_MAX)
    {
        return;
    }
    angle = sFTRemixSheikUSPMoveAngle[fp->player];
    vel_x = SHEIK_USP_MOVE_SPEED * __cosf(angle);
    vel_y = SHEIK_USP_MOVE_SPEED * __sinf(angle);
    if (fp->ga == nMPKineticsGround)
    {
        fp->physics.vel_ground.x = vel_x;
        fp->physics.vel_air.x = vel_x * fp->lr;
        return;
    }
    fp->physics.vel_air.x = vel_x * fp->lr;
    fp->physics.vel_air.y = vel_y;
}

/* SheikUSP.move_initial_: begins the vanish movement and picks Ground_Move or Air_Move. */
static void ftRemixSheikUSPMoveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    sb32 is_grounded_at_start = (fp->ga == nMPKineticsGround);
    s32 stick_magnitude;

    if (ABS(fp->input.pl.stick_range.x) >= SHEIK_USP_MIN_STICK_MAGNITUDE)
    {
        ftParamSetStickLR(fp);
    }
    stick_magnitude = ftRemixSheikUSPStickMagnitude(fp);
    ftMainSetStatus(fighter_gobj, ftRemixSheikUSPMoveActionId(fp, stick_magnitude), 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    fp->physics.vel_ground.x = 0.0F;
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->jumps_used = fp->attr->jumps_max;
    ftRemixSheikUSPSetMoveAngle(fp, stick_magnitude);
    sFTRemixSheikUSPMoveTics[fp->player] = SHEIK_USP_MOVE_TICS;
    ftRemixSheikUSPApplyMovement(fp);
    ftRemixSheikUSPHideIntangible(fp);
    if (is_grounded_at_start)
    {
        fp->coll_data.ignore_line_id = fp->coll_data.floor_line_id;
    }
}

/* SheikUSP.ground_end_initial_ */
static void ftRemixSheikUSPGroundEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPGroundEnd, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_ground.x *= SHEIK_USP_GROUND_END_VEL_MUL;
}

/* SheikUSP.air_end_initial_ */
static void ftRemixSheikUSPAirEndSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSheikStatusUSPAirEnd, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.x *= SHEIK_USP_AIR_END_VEL_MUL;
    fp->physics.vel_air.y *= SHEIK_USP_AIR_END_VEL_MUL;
    fp->motion_vars.flags.flag1 = 0;
}

/* SheikUSP.begin_main_ */
static void ftRemixSheikUSPBeginProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixSheikUSPMoveSetStatus);
}

/* SheikUSP.move_main_: counts the vanish down and moves on to the end action. */
static void ftRemixSheikUSPMoveProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    sFTRemixSheikUSPMoveTics[fp->player]--;
    if (sFTRemixSheikUSPMoveTics[fp->player] != 0)
    {
        return;
    }
    if (fp->ga != nMPKineticsGround)
    {
        ftRemixSheikUSPAirEndSetStatus(fighter_gobj);
    }
    else ftRemixSheikUSPGroundEndSetStatus(fighter_gobj);
}

/* SheikUSP.move_physics_ */
static void ftRemixSheikUSPMoveProcPhysics(GObj *fighter_gobj)
{
    ftRemixSheikUSPApplyMovement(ftGetStruct(fighter_gobj));
}

/* SheikUSP.ground_end_main_ */
static void ftRemixSheikUSPGroundEndProcUpdate(GObj *fighter_gobj)
{
    ftRemixSheikUSPEndInvisibility(ftGetStruct(fighter_gobj));
    ftAnimEndSetWait(fighter_gobj);
}

/* SheikUSP.air_end_main_: special fall once the animation has ended. */
static void ftRemixSheikUSPAirEndProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixSheikUSPEndInvisibility(fp);
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, SHEIK_USP_FALL_DRIFT, FALSE, TRUE, FALSE, SHEIK_USP_FALL_LANDING_LAG, TRUE);
}

/* SheikUSP.end_physics_: stick drift once temp variable 2 is set, then gravity, then friction. */
static void ftRemixSheikUSPEndProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    if (fp->motion_vars.flags.flag1 != 0)
    {
        ftPhysicsClampAirVelXStickRange(fp, SHEIK_USP_END_DRIFT_STICK_MIN, attr->air_accel, attr->air_speed_max_x);
    }
    ftPhysicsApplyGravityDefault(fp, attr);
    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftPhysicsApplyAirVelXFriction(fp, attr);
    }
}

/* SheikUSP.ground_begin_collision_ */
static void ftRemixSheikUSPGroundBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixSheikUSPAirBeginTransition);
}

/* SheikUSP.air_begin_collision_ */
static void ftRemixSheikUSPAirBeginProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixSheikUSPGroundBeginTransition);
}

/* SheikUSP.ground_move_collision_ */
static void ftRemixSheikUSPGroundMoveProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixSheikUSPAirMoveTransition);
}

/* SheikUSP.air_move_collision_ */
static void ftRemixSheikUSPAirMoveProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterCliff(fighter_gobj, ftRemixSheikUSPGroundMoveTransition);
}

/* SheikUSP.end_collision_: Samus' up special collision with Sheik's landing lag. */
static void ftRemixSheikUSPEndProcMap(GObj *fighter_gobj)
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
    else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, SHEIK_USP_LANDING_LAG);
}

/* Remix address -> port routine for every SheikUSP routine in the Remix action array or special tables. */
const FTRemixProc dFTRemixSheikUSPProcs[] =
{
    { REMIX_SYM_SheikUSP__air_begin_collision_, ftRemixSheikUSPAirBeginProcMap },
    { REMIX_SYM_SheikUSP__air_begin_initial_, ftRemixSheikUSPAirBeginSetStatus },
    { REMIX_SYM_SheikUSP__air_end_main_, ftRemixSheikUSPAirEndProcUpdate },
    { REMIX_SYM_SheikUSP__air_move_collision_, ftRemixSheikUSPAirMoveProcMap },
    { REMIX_SYM_SheikUSP__begin_main_, ftRemixSheikUSPBeginProcUpdate },
    { REMIX_SYM_SheikUSP__end_collision_, ftRemixSheikUSPEndProcMap },
    { REMIX_SYM_SheikUSP__end_physics_, ftRemixSheikUSPEndProcPhysics },
    { REMIX_SYM_SheikUSP__ground_begin_collision_, ftRemixSheikUSPGroundBeginProcMap },
    { REMIX_SYM_SheikUSP__ground_begin_initial_, ftRemixSheikUSPGroundBeginSetStatus },
    { REMIX_SYM_SheikUSP__ground_end_main_, ftRemixSheikUSPGroundEndProcUpdate },
    { REMIX_SYM_SheikUSP__ground_move_collision_, ftRemixSheikUSPGroundMoveProcMap },
    { REMIX_SYM_SheikUSP__move_main_, ftRemixSheikUSPMoveProcUpdate },
    { REMIX_SYM_SheikUSP__move_physics_, ftRemixSheikUSPMoveProcPhysics }
};

const s32 dFTRemixSheikUSPProcsNum = ARRAY_COUNT(dFTRemixSheikUSPProcs);

#endif
