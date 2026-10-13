/**
 * ftremixdragonkingnsp.c - C ports of Smash Remix Dragon King's neutral special
 * routines (.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm,
 * scope DragonKingNSP).
 *
 * Temp variables: flag0 (0x17C) is the dragon ball spawn request, flag1 (0x180)
 * is the aerial control lock. Scratch word 0x0B20 holds the dragon ball object.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>

enum
{
    nFTRemixDragonKingStatusNSPGround = 0xE4,
    nFTRemixDragonKingStatusNSPAir = 0xE5
};

/* Kirby and J Kirby copy statuses, from the KIRBY add_new_action order in Kirby.asm (0xDC + 83 + index). */
enum
{
    nFTRemixDragonKingKirbyStatusNSPGround = 0x1AB,
    nFTRemixDragonKingKirbyStatusNSPAir = 0x1AC
};

#define DRAGONKING_NSP_OFF_BALL_OBJECT 0x0B20
#define DRAGONKING_NSP_KIRBY_TRANSITION_FLAGS \
    (FTSTATUS_PRESERVE_RUMBLE | FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_COLANIM)

/* DragonKingNSP.dragon_ball_ (called by main_). The lightning ball is an effect spawned
 * on Remix GFX struct 0x8013E1D4, which is not ported, so the spawn request is consumed
 * without creating the ball. */
static void ftRemixDragonKingNSPDragonBall(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag0 != 1)
    {
        return;
    }
    /* REMIX-TODO(GFX): efManagerMakeEffectNoForce(GFX struct 0x8013E1D4) is not spawned. The ball object,
     * its scale, its hand attachment and the on-hit hook it installs are skipped. */
    fp->motion_vars.flags.flag0 = 0;
}

/* DragonKingNSP.destroy_ball_on_hit_ (installed as proc_damage by the transitions). Only a
 * spawned ball is destroyed; dragon_ball_ never spawns one here, so the word stays 0. */
static void ftRemixDragonKingNSPDestroyBallOnHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (*ftRemixFighterWord(fp, DRAGONKING_NSP_OFF_BALL_OBJECT) == 0)
    {
        return;
    }
    /* REMIX-TODO(GFX): destroy_ball_object_ (patched vanilla effect destroy) is not ported. */
}

/* Shared body of DragonKingNSP.ground_initial_ and air_initial_. */
static void ftRemixDragonKingNSPSetDragonBallStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftCaptainSpecialNInitStatusVars(fighter_gobj);
    *ftRemixFighterWord(fp, DRAGONKING_NSP_OFF_BALL_OBJECT) = 0;
}

/* DragonKingNSP.ground_initial_ */
static void ftRemixDragonKingNSPGroundInitial(GObj *fighter_gobj)
{
    ftRemixDragonKingNSPSetDragonBallStatus(fighter_gobj, nFTRemixDragonKingStatusNSPGround);
}

/* DragonKingNSP.air_initial_ */
static void ftRemixDragonKingNSPAirInitial(GObj *fighter_gobj)
{
    ftRemixDragonKingNSPSetDragonBallStatus(fighter_gobj, nFTRemixDragonKingStatusNSPAir);
}

/* DragonKingNSP.ground_to_air_ (transition from ground collision). */
static void ftRemixDragonKingNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftCaptainSpecialNSwitchStatusAir(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.air_to_ground_ (transition from air collision). */
static void ftRemixDragonKingNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp;

    ftCaptainSpecialAirNSwitchStatusGround(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.ground_collision_ */
static void ftRemixDragonKingNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixDragonKingNSPGroundToAir);
}

/* DragonKingNSP.air_collision_ */
static void ftRemixDragonKingNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDragonKingNSPAirToGround);
}

/* REMIX-TODO(lag): kirby_ground_to_air_ and kirby_air_to_ground_ also store 0x800F9C8C and 0x800F9CC4 in
 * proc_lagstart and proc_lagend. Neither address names a decomp function (0x800F9C8C lies just after
 * mpCollisionCheckProjectRWall at 0x800F9C30), so the lag procs are not set here. See the notes file. */
/* DragonKingNSP.kirby_ground_to_air_: moves a grounded Kirby special to its aerial status. */
static void ftRemixDragonKingNSPKirbyGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingKirbyStatusNSPAir, fighter_gobj->anim_frame, 1.0F,
        DRAGONKING_NSP_KIRBY_TRANSITION_FLAGS);
    ftPhysicsClampAirVelXMax(fp);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.kirby_air_to_ground_: moves an aerial Kirby special to its grounded status. */
static void ftRemixDragonKingNSPKirbyAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixDragonKingKirbyStatusNSPGround, fighter_gobj->anim_frame, 1.0F,
        DRAGONKING_NSP_KIRBY_TRANSITION_FLAGS);
    fp->proc_damage = ftRemixDragonKingNSPDestroyBallOnHit;
}

/* DragonKingNSP.kirby_ground_collision_ */
static void ftRemixDragonKingNSPKirbyGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDragonKingNSPKirbyGroundToAir);
}

/* DragonKingNSP.kirby_air_collision_ */
static void ftRemixDragonKingNSPKirbyAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDragonKingNSPKirbyAirToGround);
}

/* DragonKingNSP.main_ (the proc update). Once the animation ends, the fighter waits on the
 * ground or falls in the air, chosen by the kinetics (ga). */
static void ftRemixDragonKingNSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    void (*proc_status)(GObj *);

    ftRemixDragonKingNSPDragonBall(fp);

    proc_status = (fp->ga == nMPKineticsGround) ? ftCommonWaitSetStatus : ftCommonFallSetStatus;
    ftAnimEndCheckSetStatus(fighter_gobj, proc_status);
}

/* DragonKingNSP.physics_aerial_ (temp variable 2 locks aerial control). */
static void ftRemixDragonKingNSPAerialProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
        return;
    }
    ftPhysicsApplyAirVelFriction(fighter_gobj);
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
}

const FTRemixProc dFTRemixDragonKingNSPProcs[] =
{
    { REMIX_SYM_DragonKingNSP__air_collision_, ftRemixDragonKingNSPAirProcMap },
    { REMIX_SYM_DragonKingNSP__air_initial_, ftRemixDragonKingNSPAirInitial },
    { REMIX_SYM_DragonKingNSP__ground_collision_, ftRemixDragonKingNSPGroundProcMap },
    { REMIX_SYM_DragonKingNSP__ground_initial_, ftRemixDragonKingNSPGroundInitial },
    { REMIX_SYM_DragonKingNSP__main_, ftRemixDragonKingNSPProcUpdate },
    { REMIX_SYM_DragonKingNSP__physics_aerial_, ftRemixDragonKingNSPAerialProcPhysics },
    { REMIX_SYM_DragonKingNSP__kirby_ground_collision_, ftRemixDragonKingNSPKirbyGroundProcMap },
    { REMIX_SYM_DragonKingNSP__kirby_air_collision_, ftRemixDragonKingNSPKirbyAirProcMap },
};

const s32 dFTRemixDragonKingNSPProcsNum = ARRAY_COUNT(dFTRemixDragonKingNSPProcs);

#endif
