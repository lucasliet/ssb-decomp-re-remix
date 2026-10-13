/**
 * ftremixdragonkingdsp.c - C ports of Smash Remix Dragon King down special routines
 * (.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm, scope DragonKingDSP).
 *
 * The aerial down special keeps its phase in temp variable 3 (motion_vars.flags.flag2):
 * BEGIN while the rise starts, MOVE while it is driven, 0 in the aerial loop. Temp
 * variable 2 (flag1) selects the friction physics, which removes player control, when set.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixDragonKingStatusDSPG = 0xE6,
    nFTRemixDragonKingStatusDSPALoop = 0xE7,
    nFTRemixDragonKingStatusDSPALanding = 0xE8,
    nFTRemixDragonKingStatusDSPABegin = 0xE9
};

enum
{
    nFTRemixDragonKingDSPTemp3Begin = 1,
    nFTRemixDragonKingDSPTemp3Move = 2
};

#define DRAGONKING_DSP_AERIAL_INITIAL_Y_SPEED 8.0F   /* 0x41000000 */
#define DRAGONKING_DSP_X_SPEED 60.0F                 /* 0x42700000, multiplied by the facing direction */
#define DRAGONKING_DSP_Y_SPEED -140.0F               /* 0xC30C0000 */
#define DRAGONKING_DSP_LANDING_X_SCALE 0.5F          /* 0x3F000000; the asm comment says 0.625 but the immediate is 0x3F00 */

/* DragonKingDSP.loop_initial_: starts the aerial loop with every temp variable cleared. */
static void ftRemixDragonKingDSPLoopInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingStatusDSPALoop, 0.0F, 1.0F, FTSTATUS_PRESERVE_HIT);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* DragonKingDSP.landing_initial_: Falcon Kick's landing (0x801600EC) with the DSP landing action. */
static void ftRemixDragonKingDSPLandingInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    fp->physics.vel_ground.x *= DRAGONKING_DSP_LANDING_X_SCALE;
    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingStatusDSPALanding, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* DragonKingDSP.aerial_main_ */
static void ftRemixDragonKingDSPAerialProcUpdate(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixDragonKingDSPLoopInitial);
}

/* DragonKingDSP.ground_initial_ */
static void ftRemixDragonKingDSPGroundInitial(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingStatusDSPG, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* DragonKingDSP.air_initial_ */
static void ftRemixDragonKingDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingStatusDSPABegin, 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = nFTRemixDragonKingDSPTemp3Begin;

    /* lbu 0x018D; andi 0x07: clears is_absorb..is_fastfall, keeps is_magnify_show..is_invisible. */
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* DragonKingDSP.physics_: friction while temp variable 2 is set, drift otherwise. The dive or rise speed follows. */
static void ftRemixDragonKingDSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 != 0)
    {
        ftPhysicsApplyAirVelFriction(fighter_gobj);
    }
    else
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
    }

    if (fp->status_id == nFTRemixDragonKingStatusDSPALoop || fp->motion_vars.flags.flag2 == nFTRemixDragonKingDSPTemp3Move)
    {
        fp->physics.vel_air.x = (f32)fp->lr * DRAGONKING_DSP_X_SPEED;
        fp->physics.vel_air.y = DRAGONKING_DSP_Y_SPEED;
        return;
    }
    /* The asm stores the stale f2 as x velocity here; this port keeps the x velocity the physics call left. */
    fp->physics.vel_air.y = DRAGONKING_DSP_AERIAL_INITIAL_Y_SPEED;
}

/* DragonKingDSP.collision_: grounded collision, or the DSP landing when the dive ends, else the common landing. */
static void ftRemixDragonKingDSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }

    if (fp->motion_vars.flags.flag2 == nFTRemixDragonKingDSPTemp3Move || fp->status_id == nFTRemixDragonKingStatusDSPALoop)
    {
        mpCommonProcFighterCliff(fighter_gobj, ftRemixDragonKingDSPLandingInitial);
        return;
    }
    mpCommonProcFighterCliffWaitOrLanding(fighter_gobj);
}

const FTRemixProc dFTRemixDragonKingDSPProcs[] =
{
    { REMIX_SYM_DragonKingDSP__aerial_main_, ftRemixDragonKingDSPAerialProcUpdate },
    { REMIX_SYM_DragonKingDSP__air_initial_, ftRemixDragonKingDSPAirInitial },
    { REMIX_SYM_DragonKingDSP__collision_, ftRemixDragonKingDSPProcMap },
    { REMIX_SYM_DragonKingDSP__ground_initial_, ftRemixDragonKingDSPGroundInitial },
    { REMIX_SYM_DragonKingDSP__physics_, ftRemixDragonKingDSPProcPhysics }
};

const s32 dFTRemixDragonKingDSPProcsNum = ARRAY_COUNT(dFTRemixDragonKingDSPProcs);

#endif
