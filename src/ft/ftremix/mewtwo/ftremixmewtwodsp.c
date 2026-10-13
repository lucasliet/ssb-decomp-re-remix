/**
 * ftremixmewtwodsp.c - C ports of Smash Remix Mewtwo down special routines
 * (.toolchain/pkgs/smashremix-src/src/Mewtwo/MewtwoSpecial.asm, scope MewtwoDSP).
 *
 * Grounded and aerial DSP use actions 0xE5 and 0xE6 (Mewtwo.asm,
 * Character.edit_action_parameters for MTWO).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixMewtwoStatusDSPGround = 0xE5,
    nFTRemixMewtwoStatusDSPAir = 0xE6
};

/* MewtwoDSP.ground_transition_: switches an aerial DSP to the grounded DSP. */
static void ftRemixMewtwoDSPGroundTransition(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusDSPGround, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT);
}

/* MewtwoDSP.air_transition_: switches a grounded DSP to the aerial DSP. */
static void ftRemixMewtwoDSPAirTransition(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusDSPAir, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT);
    /* Vanilla 0x800D8EB8 is ftPhysicsClampAirVelXMax; the Remix asm comment calls it "momentum capture?". */
    ftPhysicsClampAirVelXMax(fp);
}

/* MewtwoDSP.ground_initial_: special entry for the grounded DSP. */
static void ftRemixMewtwoDSPGroundInitial(GObj *fighter_gobj)
{
    /* REMIX-TODO(PokemonAnnouncer): skips PokemonAnnouncer.disable_announcement_ (PokemonAnnouncer.asm:799). */
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusDSPGround, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* MewtwoDSP.air_initial_: special entry for the aerial DSP. */
static void ftRemixMewtwoDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp;

    /* REMIX-TODO(PokemonAnnouncer): skips PokemonAnnouncer.disable_announcement_ (PokemonAnnouncer.asm:799). */
    ftMainSetStatus(fighter_gobj, nFTRemixMewtwoStatusDSPAir, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp = ftGetStruct(fighter_gobj);
    fp->physics.vel_air.y = 0.0F;
}

/* MewtwoDSP.ground_collision_: no floor or slide-off transitions to the aerial DSP. */
static void ftRemixMewtwoDSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMewtwoDSPAirTransition);
}

/* MewtwoDSP.air_collision_: landing transitions to the grounded DSP. */
static void ftRemixMewtwoDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMewtwoDSPGroundTransition);
}

/* Remix address -> port routine for every MewtwoDSP routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixMewtwoDSPProcs[] =
{
    { REMIX_SYM_MewtwoDSP__ground_initial_, ftRemixMewtwoDSPGroundInitial },
    { REMIX_SYM_MewtwoDSP__air_initial_, ftRemixMewtwoDSPAirInitial },
    { REMIX_SYM_MewtwoDSP__ground_collision_, ftRemixMewtwoDSPGroundProcMap },
    { REMIX_SYM_MewtwoDSP__air_collision_, ftRemixMewtwoDSPAirProcMap }
};

const s32 dFTRemixMewtwoDSPProcsNum = ARRAY_COUNT(dFTRemixMewtwoDSPProcs);

#endif
