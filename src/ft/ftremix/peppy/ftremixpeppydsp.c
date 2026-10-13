/**
 * ftremixpeppydsp.c - C ports of Smash Remix Peppy down special routines
 * (.toolchain/pkgs/smashremix-src/src/Peppy/PeppySpecial.asm, scope PeppyDSP).
 *
 * The grenade's temp variable 1 (motion_vars.flags.flag0) is the detonate
 * request. Its power (N64 0x0B20) and frames held (0x0B28) are status words, and
 * the active grenade object is passive word 0x0ADC. The grenade item itself is
 * Item.Flashbang (decomp/src/it/itremix/itremixflashbang.c).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <it/itremix/itremixflashbang.h>

enum
{
    nFTRemixPeppyStatusDSPGround = 0xEC,
    nFTRemixPeppyStatusDSPIgniteGround = 0xEF,
    nFTRemixPeppyStatusDSPAir = 0xF1,
    nFTRemixPeppyStatusDSPIgniteAir = 0xF5
};

#define PEPPY_DSP_N64_POWER 0x0B20
#define PEPPY_DSP_N64_HELD_FRAMES 0x0B28
#define PEPPY_DSP_INITIAL_POWER 20
#define PEPPY_DSP_HOLD_POWER_START_FRAME 3
#define PEPPY_DSP_HOLD_POWER_GAIN 1
#define PEPPY_DSP_JOINT_RIGHT_HAND 16 /* FTStruct.joints[16] at N64 0x0928 (part 0xC) */
#define PEPPY_DSP_FLASHBANG_FLAGS 0x80000002 /* 5th argument of the Flashbang spawn in PeppyDSP.main */

#define PEPPY_DSP_DETONATE_TIMER 3

/* Power of the throw, set by initial_ and raised while B is held (N64 0x0B20). */
static s32 *ftRemixPeppyDSPPower(FTStruct *fp)
{
    return ftRemixFighterWord(fp, PEPPY_DSP_N64_POWER);
}

/* Frames since the special began (N64 0x0B28). */
static s32 *ftRemixPeppyDSPHeldFrames(FTStruct *fp)
{
    return ftRemixFighterWord(fp, PEPPY_DSP_N64_HELD_FRAMES);
}

/* Returns the grenade object in flight, or NULL. The Flashbang item stores it at spawn and clears it at detonation (Flashbang.asm:158, :508). */
static GObj *ftRemixPeppyDSPActiveGrenade(FTStruct *fp)
{
    return *(GObj **)&fp->passive_vars;
}

/* PeppyDSP.initial_ and air_initial_: shared body, the status id differs by
 * ready_status (no grenade in flight) and ignite_status (grenade in flight). */
static void ftRemixPeppyDSPInitialSetStatus(GObj *fighter_gobj, s32 ready_status, s32 ignite_status)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status = ready_status;

    fp->motion_vars.flags.flag0 = 0;
    *ftRemixPeppyDSPPower(fp) = PEPPY_DSP_INITIAL_POWER;
    *ftRemixPeppyDSPHeldFrames(fp) = 0;

    if (ftRemixPeppyDSPActiveGrenade(fp) != NULL)
    {
        status = ignite_status;
    }
    ftMainSetStatus(fighter_gobj, status, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    /* The asm's second ftMainSetStatus after the branch is unreachable and is not ported. */
    fp->is_special_interrupt = TRUE;
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* PeppyDSP.initial_: special entry for the grounded grenade. */
static void ftRemixPeppyDSPInitial(GObj *fighter_gobj)
{
    ftRemixPeppyDSPInitialSetStatus(fighter_gobj, nFTRemixPeppyStatusDSPGround, nFTRemixPeppyStatusDSPIgniteGround);
}

/* PeppyDSP.air_initial_: special entry for the aerial grenade. */
static void ftRemixPeppyDSPAirInitial(GObj *fighter_gobj)
{
    ftRemixPeppyDSPInitialSetStatus(fighter_gobj, nFTRemixPeppyStatusDSPAir, nFTRemixPeppyStatusDSPIgniteAir);
}

/* PeppyDSP.main: the grenade is released from the right hand (part 0xC) with zero velocity. */
static void ftRemixPeppyDSPSpawnFlashbang(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[PEPPY_DSP_JOINT_RIGHT_HAND], &pos);
    pos.z = 0.0F;
    itRemixFlashbangMakeItem(fighter_gobj, &pos, &vel, PEPPY_DSP_FLASHBANG_FLAGS);
}

/* PeppyDSP.main: charges power while B is held, tosses the grenade on the
 * detonate request, and ends the move when the animation finishes. */
static void ftRemixPeppyDSPMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 *held_frames = ftRemixPeppyDSPHeldFrames(fp);
    s32 frames_before = *held_frames;

    *held_frames = frames_before + 1;
    if ((frames_before >= PEPPY_DSP_HOLD_POWER_START_FRAME) && (fp->input.pl.button_hold & B_BUTTON))
    {
        *ftRemixPeppyDSPPower(fp) += PEPPY_DSP_HOLD_POWER_GAIN;
    }
    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixPeppyDSPSpawnFlashbang(fighter_gobj);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* PeppyDSP.detonate_check: a detonate request shortens the grenade's fuse. */
static void ftRemixPeppyDSPDetonateCheck(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *grenade_gobj;
    ITStruct *ip;

    if (fp->motion_vars.flags.flag0 == 0)
    {
        return;
    }
    fp->motion_vars.flags.flag0 = 0;
    grenade_gobj = ftRemixPeppyDSPActiveGrenade(fp);
    if (grenade_gobj == NULL)
    {
        return;
    }
    ip = itGetStruct(grenade_gobj);
    if (ip->lifetime >= PEPPY_DSP_DETONATE_TIMER)
    {
        ip->lifetime = PEPPY_DSP_DETONATE_TIMER;
    }
}

/* PeppyDSP.detonate_main_ground: detonate check, then ftAnimEndSetWait (0x800D94C4). */
static void ftRemixPeppyDSPDetonateMainGround(GObj *fighter_gobj)
{
    ftRemixPeppyDSPDetonateCheck(fighter_gobj);
    ftAnimEndSetWait(fighter_gobj);
}

/* PeppyDSP.detonate_main_air: detonate check, then ftAnimEndSetFall (0x800D94E8). */
static void ftRemixPeppyDSPDetonateMainAir(GObj *fighter_gobj)
{
    ftRemixPeppyDSPDetonateCheck(fighter_gobj);
    ftAnimEndSetFall(fighter_gobj);
}

/* PeppyDSP.ground_to_air: grounded grenade leaves the ground for the aerial toss. */
static void ftRemixPeppyDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixPeppyStatusDSPAir, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftPhysicsClampAirVelXMax(fp);
}

/* PeppyDSP.air_to_ground: aerial grenade lands into the grounded toss. */
static void ftRemixPeppyDSPAirToGround(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixPeppyStatusDSPGround, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* PeppyDSP.ground_to_air_fail: grounded ignite leaves the ground for the aerial ignite. */
static void ftRemixPeppyDSPGroundToAirIgnite(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, nFTRemixPeppyStatusDSPIgniteAir, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
    ftPhysicsClampAirVelXMax(fp);
}

/* PeppyDSP.air_to_ground_fail: aerial ignite lands into the grounded ignite. */
static void ftRemixPeppyDSPAirToGroundIgnite(GObj *fighter_gobj)
{
    mpCommonSetFighterGround(ftGetStruct(fighter_gobj));
    ftMainSetStatus(fighter_gobj, nFTRemixPeppyStatusDSPIgniteGround, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_COLANIM);
}

/* PeppyDSP.ground_collision: grounded toss walks off an edge into the aerial toss. */
static void ftRemixPeppyDSPGroundCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixPeppyDSPGroundToAir);
}

/* PeppyDSP.ground_collision_fail: grounded ignite walks off an edge. */
static void ftRemixPeppyDSPGroundCollisionIgnite(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixPeppyDSPGroundToAirIgnite);
}

/* PeppyDSP.air_collision: aerial toss landing switches to the grounded toss. */
static void ftRemixPeppyDSPAirCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPeppyDSPAirToGround);
}

/* PeppyDSP.air_collision_fail: aerial ignite landing switches to the grounded ignite. */
static void ftRemixPeppyDSPAirCollisionIgnite(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixPeppyDSPAirToGroundIgnite);
}

/* Remix address -> port routine for every PeppyDSP routine that the action
 * array or special tables reference. */
const FTRemixProc dFTRemixPeppyDSPProcs[] =
{
    { REMIX_SYM_PeppyDSP__initial_, ftRemixPeppyDSPInitial },
    { REMIX_SYM_PeppyDSP__air_initial_, ftRemixPeppyDSPAirInitial },
    { REMIX_SYM_PeppyDSP__main, ftRemixPeppyDSPMain },
    { REMIX_SYM_PeppyDSP__detonate_main_ground, ftRemixPeppyDSPDetonateMainGround },
    { REMIX_SYM_PeppyDSP__detonate_main_air, ftRemixPeppyDSPDetonateMainAir },
    { REMIX_SYM_PeppyDSP__ground_collision, ftRemixPeppyDSPGroundCollision },
    { REMIX_SYM_PeppyDSP__ground_collision_fail, ftRemixPeppyDSPGroundCollisionIgnite },
    { REMIX_SYM_PeppyDSP__air_collision, ftRemixPeppyDSPAirCollision },
    { REMIX_SYM_PeppyDSP__air_collision_fail, ftRemixPeppyDSPAirCollisionIgnite }
};

const s32 dFTRemixPeppyDSPProcsNum = ARRAY_COUNT(dFTRemixPeppyDSPProcs);

#endif
