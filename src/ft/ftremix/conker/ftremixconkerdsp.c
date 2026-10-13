/**
 * ftremixconkerdsp.c - C ports of Smash Remix Conker grenade routines
 * (.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm, scope ConkerDSP).
 *
 * The grenade item itself (grenade_* in the asm) is it/itremix/itremixconkergrenade.c.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixhooks.h"
#include <ft/ftmain.h>
#include <gm/gmcollision.h>
#include <it/itremix/itremixconkergrenade.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixConkerStatusGrenadeToss = 0xEC,
    nFTRemixConkerStatusGrenadeTossFail = 0xEF,
    nFTRemixConkerStatusGrenadeTossAir = 0xF1,
    nFTRemixConkerStatusGrenadeTossFailAir = 0xF5
};

#define CONKER_DSP_HOLD_POWER_START_FRAME 3
#define CONKER_DSP_HOLD_POWER_GAIN 5
#define CONKER_DSP_INITIAL_POWER 20
#define CONKER_DSP_JOINT_RIGHT_HAND 16 /* FTStruct.joints[16] at N64 0x0928 (part 0xC) */

/* Conker's grenade fields live in the parent's union slots: power and hold
 * counter in status_vars (0x0B20, 0x0B28), the active grenade object in
 * passive_vars (0x0ADC). They are read through these overlays. */
typedef struct FTRemixConkerDSPStatusVars
{
    s32 grenade_power;
    s32 hold_frames;
} FTRemixConkerDSPStatusVars;

static FTRemixConkerDSPStatusVars *ftRemixConkerDSPStatusVars(FTStruct *fp)
{
    return (FTRemixConkerDSPStatusVars *)&fp->status_vars;
}

/* Returns the active grenade object, or NULL when no grenade is in flight. */
static GObj *ftRemixConkerDSPActiveGrenade(FTStruct *fp)
{
    return *(GObj **)&fp->passive_vars;
}

void ftRemixConkerDSPSetActiveGrenade(FTStruct *fp, GObj *grenade_gobj)
{
    *(GObj **)&fp->passive_vars = grenade_gobj;
}

/* ConkerDSP.main: grenade_stage_setting_ at the right hand (part 0xC). */
static void ftRemixConkerDSPTossGrenade(FTStruct *fp, GObj *fighter_gobj)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[CONKER_DSP_JOINT_RIGHT_HAND], &pos);
    itRemixConkerGrenadeMakeItem(fighter_gobj, &pos, ftRemixConkerDSPStatusVars(fp)->grenade_power);
}

/* ConkerDSP.initial_ and air_initial_: shared body, the status id differs by
 * ready_status (no grenade in flight) and blocked_status (grenade in flight). */
static void ftRemixConkerDSPInitialSetStatus(GObj *fighter_gobj, s32 ready_status, s32 blocked_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixConkerDSPStatusVars *vars = ftRemixConkerDSPStatusVars(fp);
    s32 status = ready_status;

    fp->motion_vars.flags.flag0 = 0;
    vars->grenade_power = CONKER_DSP_INITIAL_POWER;
    vars->hold_frames = 0;

    if (ftRemixConkerDSPActiveGrenade(fp) != NULL)
    {
        status = blocked_status;
    }
    ftMainSetStatus(fighter_gobj, status, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    /* Remix sets is_special_interrupt on both paths; the asm's second
     * ftMainSetStatus after the branch is unreachable and is not ported. */
    fp->is_special_interrupt = TRUE;
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* ConkerDSP.initial_: special entry for the grounded grenade. */
static void ftRemixConkerDSPInitial(GObj *fighter_gobj)
{
    ftRemixConkerDSPInitialSetStatus(fighter_gobj, nFTRemixConkerStatusGrenadeToss, nFTRemixConkerStatusGrenadeTossFail);
}

/* ConkerDSP.air_initial_: special entry for the aerial grenade. */
static void ftRemixConkerDSPAirInitial(GObj *fighter_gobj)
{
    ftRemixConkerDSPInitialSetStatus(fighter_gobj, nFTRemixConkerStatusGrenadeTossAir, nFTRemixConkerStatusGrenadeTossFailAir);
}

/* ConkerDSP.main: holds the throw, charges power while B is held, and ends the
 * move when the animation finishes. */
static void ftRemixConkerDSPMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixConkerDSPStatusVars *vars = ftRemixConkerDSPStatusVars(fp);
    s32 held_frames = vars->hold_frames;

    vars->hold_frames = held_frames + 1;
    if ((held_frames >= CONKER_DSP_HOLD_POWER_START_FRAME) && (fp->input.pl.button_hold & B_BUTTON))
    {
        vars->grenade_power += CONKER_DSP_HOLD_POWER_GAIN;
    }
    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixConkerDSPTossGrenade(fp, fighter_gobj);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* ConkerDSP.ground_to_air_: grounded grenade leaves the ground for the aerial toss. */
static void ftRemixConkerDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixConkerStatusGrenadeTossAir, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftPhysicsClampAirVelXMax(fp);
}

/* ConkerDSP.air_to_ground_: aerial grenade lands into the grounded toss. */
static void ftRemixConkerDSPAirToGround(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixConkerStatusGrenadeToss, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* ConkerDSP.ground_to_air_fail: grounded fail toss leaves the ground. */
static void ftRemixConkerDSPGroundToAirFail(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixConkerStatusGrenadeTossFailAir, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftPhysicsClampAirVelXMax(fp);
}

/* ConkerDSP.air_to_ground_fail: aerial fail toss lands into the grounded fail toss. */
static void ftRemixConkerDSPAirToGroundFail(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixConkerStatusGrenadeTossFail, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* ConkerDSP.ground_collision: grounded toss walks off an edge into the aerial toss. */
static void ftRemixConkerDSPGroundCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixConkerDSPGroundToAir);
}

/* ConkerDSP.ground_collision_fail: grounded fail toss walks off an edge. */
static void ftRemixConkerDSPGroundCollisionFail(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixConkerDSPGroundToAirFail);
}

/* ConkerDSP.air_collision: aerial toss landing switches to the grounded toss. */
static void ftRemixConkerDSPAirCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixConkerDSPAirToGround);
}

/* ConkerDSP.air_collision_fail: aerial fail toss landing switches to the grounded fail toss. */
static void ftRemixConkerDSPAirCollisionFail(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixConkerDSPAirToGroundFail);
}

/* Remix address -> port routine for every ConkerDSP routine that the action
 * array or special tables reference. */
const FTRemixProc dFTRemixConkerDSPProcs[] =
{
    { REMIX_SYM_ConkerDSP__initial_, ftRemixConkerDSPInitial },
    { REMIX_SYM_ConkerDSP__air_initial_, ftRemixConkerDSPAirInitial },
    { REMIX_SYM_ConkerDSP__main, ftRemixConkerDSPMain },
    { REMIX_SYM_ConkerDSP__ground_collision, ftRemixConkerDSPGroundCollision },
    { REMIX_SYM_ConkerDSP__ground_collision_fail, ftRemixConkerDSPGroundCollisionFail },
    { REMIX_SYM_ConkerDSP__air_collision, ftRemixConkerDSPAirCollision },
    { REMIX_SYM_ConkerDSP__air_collision_fail, ftRemixConkerDSPAirCollisionFail }
};

const s32 dFTRemixConkerDSPProcsNum = ARRAY_COUNT(dFTRemixConkerDSPProcs);

#endif
