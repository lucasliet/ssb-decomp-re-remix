/**
 * ftremixdsamusdsp.c - C ports of Smash Remix Dark Samus down special routines
 * (.toolchain/pkgs/smashremix-src/src/DSamus/DsamusSpecial.asm, scope DSamusDSP).
 *
 * Aerial DSP is action 0xE8 and DSP landing is action 0xE9 (DSamus.asm,
 * add_new_action order after NSPChargeAir 0xE7; the matching ROM entries are
 * in the DSAMUS action array). The DSP uses the Remix temp variables
 * motion_vars.flags.flag1 (N64 0x0180, temp variable 2) and flag2 (0x0184,
 * temp variable 3).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/fighter.h>
#include <ft/ftdef.h>
#include <ft/ftmain.h>
#include <ft/ftchar/ftsamus/ftsamusfunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ef/efmanager.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixDSamusStatusDSPLanding = 0xE9
};

#define DSAMUS_DSP_JUMP_SPEED 32.0F
#define DSAMUS_DSP_GROUND_X_MULTIPLIER 0.625F
#define DSAMUS_DSP_AIR_X_SPEED 70.0F
#define DSAMUS_DSP_AIR_Y_SPEED (-40.0F)
#define DSAMUS_DSP_AIR_X_STOP_MULTIPLIER 0.5F
#define DSAMUS_DSP_JUMP_SMOKE_LR 1
#define DSAMUS_DSP_JUMP_SMOKE_SCALE 1.0F

/* Switches the fighter to the aerial DSP and slows its x velocity. Shared by DSamusDSP.ground_to_air_ and ground_to_air_jump_. */
static void ftRemixDSamusDSPSwitchToAir(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftSamusSpecialLwSwitchStatusAir(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
    fp->physics.vel_air.x *= DSAMUS_DSP_GROUND_X_MULTIPLIER;
}

/* DSamusDSP.ground_to_air_: grounded DSP walks off a ledge into the aerial DSP. */
static void ftRemixDSamusDSPGroundToAir(GObj *fighter_gobj)
{
    ftRemixDSamusDSPSwitchToAir(fighter_gobj);
}

/* DSamusDSP.ground_to_air_jump_: grounded DSP jump input starts the aerial DSP with JUMP_SPEED. */
static void ftRemixDSamusDSPGroundToAirJump(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftRemixDSamusDSPSwitchToAir(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->physics.vel_air.y = DSAMUS_DSP_JUMP_SPEED;
    efManagerDustHeavyDoubleMakeEffect(fp->coll_data.p_translate, DSAMUS_DSP_JUMP_SMOKE_LR, DSAMUS_DSP_JUMP_SMOKE_SCALE);
}

/* DSamusDSP.landing_initial_: starts the DSP landing from Falcon Kick's landing start (0x801600EC). */
static void ftRemixDSamusDSPLandingInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    fp->physics.vel_ground.x *= DSAMUS_DSP_GROUND_X_MULTIPLIER;
    ftMainSetStatus(fighter_gobj, nFTRemixDSamusStatusDSPLanding, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* DSamusDSP.air_physics_: while temp variable 2 is set, the aerial DSP moves with fixed velocities instead of the aerial physics. */
static void ftRemixDSamusDSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        ftSamusSpecialAirLwProcPhysics(fighter_gobj);
        return;
    }
    if (fp->motion_vars.flags.flag1 != 1)
    {
        fp->motion_vars.flags.flag1 = 0;
        fp->physics.vel_air.x *= DSAMUS_DSP_AIR_X_STOP_MULTIPLIER;
        return;
    }
    fp->physics.vel_air.x = DSAMUS_DSP_AIR_X_SPEED * (f32)fp->lr;
    fp->physics.vel_air.y = DSAMUS_DSP_AIR_Y_SPEED;
}

/* DSamusDSP.air_collision_: temp variable 3 set means the ground pound is landing; otherwise the normal aerial collision runs. */
static void ftRemixDSamusDSPAirProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 == 0)
    {
        mpCommonProcFighterCliffFloorCeil(fighter_gobj);
        return;
    }
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDSamusDSPLandingInitial);
}

/* DSamusDSP.ground_collision_: temp variable 3 with a jump input leaves for the jump; otherwise the common floor check runs. */
static void ftRemixDSamusDSPGroundProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 != 0 && ftCommonKneeBendGetInputTypeCommon(fp) != 0)
    {
        ftRemixDSamusDSPGroundToAirJump(fighter_gobj);
        return;
    }
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDSamusDSPGroundToAir);
}

/* Routines of DSamusDSP, referenced from the Remix action array and special tables. */
const FTRemixProc dFTRemixDSamusDSPProcs[] =
{
    { REMIX_SYM_DSamusDSP__air_physics_, ftRemixDSamusDSPAirProcPhysics },
    { REMIX_SYM_DSamusDSP__air_collision_, ftRemixDSamusDSPAirProcMap },
    { REMIX_SYM_DSamusDSP__ground_collision_, ftRemixDSamusDSPGroundProcMap }
};

const s32 dFTRemixDSamusDSPProcsNum = ARRAY_COUNT(dFTRemixDSamusDSPProcs);

#endif
