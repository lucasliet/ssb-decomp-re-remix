/**
 * ftremixdragonkingusp.c - C ports of Smash Remix Dragon King up special routines
 * (.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm, scope DragonKingUSP).
 *
 * The up special is a command grab. Temp variables 1/2/3 are motion_vars.flags.flag0/1/2
 * (moveset commands 5C0000XX / 580000XX). Air pull copies vanilla air physics (0x800D90E0)
 * with a custom gravity, a custom friction and no fast fall. The drain of the captured
 * player is ftRemixDragonKingUSPDrain. Deviations and open questions are in
 * build/remixasm/notes/47_DRAGONKING_USP.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftchar/ftcaptain/ftcaptainfunctions.h>
#include <ft/ftchar/ftyoshi/ftyoshifunctions.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftparam.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixDragonKingStatusUSPGround = 0xEF,
    nFTRemixDragonKingStatusUSPGroundThrow = 0xF0,
    nFTRemixDragonKingStatusUSPAir = 0xF1,
    nFTRemixDragonKingStatusUSPAirPull = 0xF2,
    nFTRemixDragonKingStatusUSPLandingThrow = 0xF3
};

#define DRAGONKING_USP_CAPTURE_IMMUNE_MASK 0x3F
#define DRAGONKING_USP_AIR_PULL_X_SCALE 0.5F               /* 0x3F000000 */
#define DRAGONKING_USP_AIR_PULL_Y_SCALE 0.125F             /* 0x3E000000 */
#define DRAGONKING_USP_AIR_PULL_Y_SPEED 54.0F              /* 0x42580000 */
#define DRAGONKING_USP_AIR_PULL_FRICTION 1.75F             /* 0x3FE00000 */
#define DRAGONKING_USP_AIR_PULL_GRAVITY 0.5F               /* 0x3F000000 */
#define DRAGONKING_USP_AIR_PULL_GRAVITY_TEMP2 7.0F         /* 0x40E00000 */
#define DRAGONKING_USP_AIR_PULL_FALL_SPEED 110.0F          /* 0x42DC0000 */

/* Temp variable 1 (motion_vars.flags.flag0) value that drains the captured player. */
#define DRAGONKING_USP_DRAIN_TEMP1 1

/* Scratch word for the turn frame timer, N64 0x0B18. */
#define DRAGONKING_USP_TURN_TIMER_OFFSET 0x0B18

/* Clears temp variables 1, 2 and 3. */
static void ftRemixDragonKingUSPClearTempVariables(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* ftMainSetStatus with speed 1.0F; the caller plays anim events when the vanilla code does. */
static void ftRemixDragonKingUSPSetStatus(GObj *fighter_gobj, s32 status_id, f32 frame_begin, u32 flags)
{
    ftMainSetStatus(fighter_gobj, status_id, frame_begin, 1.0F, flags);
}

/* Capture immune mask and copies search_gobj to catch_gobj (0x830 -> 0x840). */
static void ftRemixDragonKingUSPThrowSetup(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftParamSetCaptureImmuneMask(fp, DRAGONKING_USP_CAPTURE_IMMUNE_MASK);
    fp->catch_gobj = fp->search_gobj;
}

/* DragonKingUSP.ground_throw_initial_: grounded throw after the command grab. */
static void ftRemixDragonKingUSPGroundThrowInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixDragonKingUSPSetStatus(fighter_gobj, nFTRemixDragonKingStatusUSPGroundThrow, 0.0F,
                                  FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixDragonKingUSPThrowSetup(fighter_gobj);
    ftRemixDragonKingUSPClearTempVariables(fp);
    *ftRemixFighterWord(fp, DRAGONKING_USP_TURN_TIMER_OFFSET) = 0;
}

/* DragonKingUSP.air_pull_initial_: aerial throw entry. Scales the velocity and adds the pull rise. */
static void ftRemixDragonKingUSPAirPullInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixDragonKingUSPSetStatus(fighter_gobj, nFTRemixDragonKingStatusUSPAirPull, 0.0F, FTSTATUS_PRESERVE_COLANIM);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixDragonKingUSPThrowSetup(fighter_gobj);
    ftRemixDragonKingUSPClearTempVariables(fp);
    fp->physics.vel_air.x *= DRAGONKING_USP_AIR_PULL_X_SCALE;
    fp->physics.vel_air.y =
        (fp->physics.vel_air.y * DRAGONKING_USP_AIR_PULL_Y_SCALE) + DRAGONKING_USP_AIR_PULL_Y_SPEED;
}

/* DragonKingUSP.landing_throw_initial_: landing throw entry, set grounded. */
static void ftRemixDragonKingUSPLandingThrowInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixDragonKingUSPSetStatus(fighter_gobj, nFTRemixDragonKingStatusUSPLandingThrow, 0.0F,
                                  FTSTATUS_PRESERVE_COLANIM);
    ftRemixDragonKingUSPClearTempVariables(fp);
}

/* Clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall (asm: and with 0x07 on the 0x18D byte).
 * The asm masks the struct pointer in v1 rather than the byte; see the notes for this deviation. */
static void ftRemixDragonKingUSPClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* DragonKingUSP.air_initial_: aerial up special entry, the command grab starts from air. */
static void ftRemixDragonKingUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->proc_status = ftCaptainSpecialHiProcStatus;
    ftRemixDragonKingUSPSetStatus(fighter_gobj, nFTRemixDragonKingStatusUSPAir, 0.0F, FTSTATUS_PRESERVE_NONE);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixDragonKingUSPAirPullInitial);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixDragonKingUSPClearTempVariables(fp);
    ftRemixDragonKingUSPClearAirFlags(fp);
}

/* DragonKingUSP.ground_initial_: grounded up special entry. */
static void ftRemixDragonKingUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixDragonKingUSPSetStatus(fighter_gobj, nFTRemixDragonKingStatusUSPGround, 0.0F, FTSTATUS_PRESERVE_NONE);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixDragonKingUSPGroundThrowInitial);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixDragonKingUSPClearTempVariables(fp);
}

/* DragonKingUSP.ground_to_air_: ground to air transition, keeps the current animation frame. */
static void ftRemixDragonKingUSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    fp->proc_status = ftCaptainSpecialHiProcStatus;
    ftRemixDragonKingUSPSetStatus(fighter_gobj, nFTRemixDragonKingStatusUSPAir, fighter_gobj->anim_frame,
                                  FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
    ftYoshiSpecialNSetCatchParams(fp, ftRemixDragonKingUSPAirPullInitial);
}

/* Air friction of 1.75F for the air pull. ftPhysicsApplyAirVelXFriction reads only air_friction. */
static void ftRemixDragonKingUSPAirPullFriction(FTStruct *fp)
{
    FTAttributes friction_attr = { 0 };

    friction_attr.air_friction = DRAGONKING_USP_AIR_PULL_FRICTION;
    ftPhysicsApplyAirVelXFriction(fp, &friction_attr);
}

/* DragonKingUSP.air_pull_physics_: vanilla air drift (0x800D90E0) with the pull gravity and friction.
 * Fast fall is skipped. Temp variable 2 selects the stronger gravity. */
static void ftRemixDragonKingUSPAirPullPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;
    f32 gravity = DRAGONKING_USP_AIR_PULL_GRAVITY;

    if (fp->motion_vars.flags.flag1 != 0)
    {
        gravity = DRAGONKING_USP_AIR_PULL_GRAVITY_TEMP2;
    }
    ftPhysicsApplyGravityClampTVel(fp, gravity, DRAGONKING_USP_AIR_PULL_FALL_SPEED);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftPhysicsClampAirVelXStickDefault(fp, attr);
        ftRemixDragonKingUSPAirPullFriction(fp);
    }
}

/* Drain while temp variable 1 is 1: heal the attacker by 1 and damage the captured player by 1.
 * The captured player is skipped while it has a super star. */
static void ftRemixDragonKingUSPDrain(FTStruct *fp)
{
    FTStruct *captured_fp;

    if (fp->motion_vars.flags.flag0 != DRAGONKING_USP_DRAIN_TEMP1)
    {
        return;
    }
    fp->damage_heal += 1;

    captured_fp = ftGetStruct(fp->catch_gobj);
    if (captured_fp->star_invincible_tics != 0)
    {
        return;
    }
    /* REMIX-TODO(Item.CloakingDevice): Remix skips the damage when Item.CloakingDevice.cloaked_players[captured port]
     * is set. That per-player table is not ported, so every captured player is treated as not cloaked. */
    ftParamUpdateDamage(captured_fp, 1);
}

/* DragonKingUSP.throw_main_: vanilla grab update (0x8014A0C0), then the drain. */
static void ftRemixDragonKingUSPThrowMain(GObj *fighter_gobj)
{
    ftCommonThrowProcUpdate(fighter_gobj);
    ftRemixDragonKingUSPDrain(ftGetStruct(fighter_gobj));
}

/* DragonKingUSP.air_pull_main_: air pull update, the drain only. */
static void ftRemixDragonKingUSPAirPullMain(GObj *fighter_gobj)
{
    ftRemixDragonKingUSPDrain(ftGetStruct(fighter_gobj));
}

/* DragonKingUSP.ground_collision_: grounded collision, transition to air when there is no floor. */
static void ftRemixDragonKingUSPGroundCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDragonKingUSPGroundToAir);
}

/* DragonKingUSP.air_pull_collision_: aerial collision, transition to the landing throw on landing. */
static void ftRemixDragonKingUSPAirPullCollision(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixDragonKingUSPLandingThrowInitial);
}

/* Called by the vanilla damage digits hook (drain_flash_fix_, notes 47_DRAGONKING_USP.md). `player_gobj` is the
 * gobj read from the hook's a1 + 0x78. Returns TRUE while a Dragon King capture keeps the % colour. */
sb32 ftRemixDragonKingUSPIsCaptureFlashOverride(GObj *player_gobj)
{
    FTStruct *player_fp;
    FTStruct *capturer_fp;
    s32 capturer_status;

    if (player_gobj == NULL)
    {
        return FALSE;
    }
    player_fp = ftGetStruct(player_gobj);
    if ((player_fp == NULL) || (player_fp->capture_gobj == NULL))
    {
        return FALSE;
    }
    capturer_fp = ftGetStruct(player_fp->capture_gobj);
    if (capturer_fp->fkind != REMIX_ID_DRAGONKING)
    {
        return FALSE;
    }
    capturer_status = capturer_fp->status_id;
    if ((capturer_status != nFTRemixDragonKingStatusUSPAirPull) &&
        (capturer_status != nFTRemixDragonKingStatusUSPGroundThrow))
    {
        return FALSE;
    }
    return capturer_fp->motion_vars.flags.flag0 != 0;
}

/* Remix address -> port routine for every DragonKingUSP routine in the Dragon King action array. */
const FTRemixProc dFTRemixDragonKingUSPProcs[] =
{
    { REMIX_SYM_DragonKingUSP__air_initial_, ftRemixDragonKingUSPAirInitial },
    { REMIX_SYM_DragonKingUSP__air_pull_collision_, ftRemixDragonKingUSPAirPullCollision },
    { REMIX_SYM_DragonKingUSP__air_pull_main_, ftRemixDragonKingUSPAirPullMain },
    { REMIX_SYM_DragonKingUSP__air_pull_physics_, ftRemixDragonKingUSPAirPullPhysics },
    { REMIX_SYM_DragonKingUSP__ground_collision_, ftRemixDragonKingUSPGroundCollision },
    { REMIX_SYM_DragonKingUSP__ground_initial_, ftRemixDragonKingUSPGroundInitial },
    { REMIX_SYM_DragonKingUSP__throw_main_, ftRemixDragonKingUSPThrowMain }
};

const s32 dFTRemixDragonKingUSPProcsNum = ARRAY_COUNT(dFTRemixDragonKingUSPProcs);

#endif
