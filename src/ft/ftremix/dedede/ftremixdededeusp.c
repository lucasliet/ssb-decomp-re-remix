/**
 * ftremixdededeusp.c - C ports of Smash Remix Dedede up special routines
 * (.toolchain/pkgs/smashremix-src/src/Dedede/DededeSpecial.asm, scope DededeUSP).
 *
 * Super Dedede Jump: the star projectile is created by the vanilla Yoshi star
 * routine (wpYoshiStarMakeStars, 0x8016C954), so no Remix weapon descriptor is built.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftanimend.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <gm/gmcollision.h>
#include <mp/mpcommon.h>
#include <sys/obj.h>
#include <sys/utils.h>
#include <wp/wpyoshi/wpyoshistar.h>

extern alSoundEffect *func_800269C0_275C0(u16 sfx_id);

enum
{
    nFTRemixDededeStatusUSPBegin = 0xE4,
    nFTRemixDededeStatusUSPMove = 0xE5,
    nFTRemixDededeStatusUSPLand = 0xE6,
    nFTRemixDededeStatusUSPCancel = 0xE9,
    nFTRemixDededeStatusUSPCeilingBonk = 0x102
};

#define DEDEDE_USP_BEGIN_SPEED 0.5F                 /* 0x3F000000 */
#define DEDEDE_USP_MOVE_SPEED_X 0.6875F             /* 0x3F300000 */
#define DEDEDE_USP_MOVE_SPEED_Y 144.0F              /* 0x43100000 */
#define DEDEDE_USP_GRAVITY 2.0F                     /* 0x40000000 */
#define DEDEDE_USP_GRAVITY_PEAK 0.19921875F         /* 0x3E4C0000, the Remix comment says 0.2 */
#define DEDEDE_USP_GRAVITY_FALLING 2.5F             /* 0x40200000 */
#define DEDEDE_USP_MAX_FALLING 125.0F               /* 0x42FA0000 */
#define DEDEDE_USP_LANDING_LAG 1.0F                 /* 0x3F800000, the landing fsm passed to the fall special */
#define DEDEDE_USP_CANCEL_STICK_Y (-39)
#define DEDEDE_USP_FGM_CEILING_BONK 0x0134
#define DEDEDE_USP_SURFACE_GROUND 0x0800            /* Surface.GROUND */
#define DEDEDE_USP_SURFACE_CEILING 0x0400           /* Surface.CEILING */
#define DEDEDE_USP_CLIFF_MASK 0x3000                /* cliff bits checked by the Remix source */
#define DEDEDE_USP_CEILING_MASK_CURR 0x4000         /* mask_curr bit checked by ceiling_bonk_collision_ */

/* Changes the fighter status with no animation events and default frame and speed. */
static void ftRemixDededeUSPChangeStatus(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* DededeUSP.begin_set_aerial */
static void ftRemixDededeUSPBeginSetAerial(GObj *fighter_gobj)
{
    ftGetStruct(fighter_gobj)->ga = TRUE;
}

/* Lands on ground or catches a cliff. Returns TRUE when the fighter landed on ground. */
static sb32 ftRemixDededeUSPSetGroundOrCliff(GObj *fighter_gobj, FTStruct *fp)
{
    if (fp->coll_data.mask_stat & DEDEDE_USP_SURFACE_GROUND)
    {
        mpCommonSetFighterGround(fp);
        return TRUE;
    }
    if (fp->coll_data.mask_stat & DEDEDE_USP_CLIFF_MASK)
    {
        ftCommonCliffCatchSetStatus(fighter_gobj);
    }
    return FALSE;
}

/* DededeUSP.cancel_initial_ */
static void ftRemixDededeUSPCancelInitial(GObj *fighter_gobj)
{
    ftRemixDededeUSPChangeStatus(fighter_gobj, nFTRemixDededeStatusUSPCancel);
}

/* DededeUSP.move_initial_ */
static void ftRemixDededeUSPMoveInitial(GObj *fighter_gobj)
{
    ftMainSetStatus(fighter_gobj, nFTRemixDededeStatusUSPMove, 0.0F, 1.0F, FTSTATUS_PRESERVE_HIT);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Ceiling bonk entry: stops vertical motion and plays the bonk status. */
static void ftRemixDededeUSPCeilingSubroutine(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixDededeUSPChangeStatus(fighter_gobj, nFTRemixDededeStatusUSPCeilingBonk);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->physics.vel_air.y = 0.0F;
    fp->physics.vel_air.z = 0.0F;
}

/* Horizontal drift of the first phase; the stick x is kept at 0x0B20 as the Remix source does. */
static void ftRemixDededeUSPApplyLateralMovement(FTStruct *fp)
{
    f32 stick_x = (f32)fp->input.pl.stick_range.x;

    fp->status_vars.common.dead.pos.y = stick_x;
    fp->physics.vel_air.x = DEDEDE_USP_BEGIN_SPEED * stick_x;
    fp->physics.vel_air.y = 0.0F;
}

/* Gravity multiplier of the vertical phase, selected by moveset variable 2 (flag1, 0x180). */
static f32 ftRemixDededeUSPGravityMultiplier(u32 phase)
{
    if (phase == 0)
    {
        return DEDEDE_USP_GRAVITY;
    }
    if (phase == 1)
    {
        return DEDEDE_USP_GRAVITY_PEAK;
    }
    return DEDEDE_USP_GRAVITY_FALLING;
}

/* Applies gravity and the air x clamp for the vertical phase. */
static void ftRemixDededeUSPApplyVerticalMovement(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 gravity = fp->attr->gravity * ftRemixDededeUSPGravityMultiplier(fp->motion_vars.flags.flag1);

    ftPhysicsApplyGravityClampTVel(fp, gravity, DEDEDE_USP_MAX_FALLING);
    ftPhysicsCheckClampAirVelXDecMax(fp, fp->attr);
}

/* First-frame launch of the movement stage: refills the jumps used by mid-air jumps and sets the launch velocity. */
static void ftRemixDededeUSPBeginLaunch(FTStruct *fp)
{
    f32 stick_x = (f32)fp->input.pl.stick_range.x;

    fp->jumps_used = fp->attr->jumps_max;
    fp->status_vars.common.dead.pos.y = stick_x;
    fp->physics.vel_air.x = stick_x * DEDEDE_USP_MOVE_SPEED_X;
    fp->physics.vel_air.y = DEDEDE_USP_MOVE_SPEED_Y;
}

/* Spawns the star projectiles at the top joint, through the vanilla Yoshi star routine. */
static void ftRemixDededeUSPSpawnStars(GObj *fighter_gobj, FTStruct *fp)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };

    gmCollisionGetFighterPartsWorldPosition(fp->joints[0], &pos);
    wpYoshiStarMakeStars(fighter_gobj, &pos);
}

/* DededeUSP.begin_initial_ (special entry) */
static void ftRemixDededeUSPBeginInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixDededeUSPChangeStatus(fighter_gobj, nFTRemixDededeStatusUSPBegin);
    ftMainPlayAnimEventsAll(fighter_gobj);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->physics.vel_ground.x = 0.0F;
    fp->physics.vel_ground.y = 0.0F;
}

/* DededeUSP.begin_main_ */
static void ftRemixDededeUSPBeginMain(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixDededeUSPMoveInitial);
}

/* DededeUSP.begin_physics_ */
static void ftRemixDededeUSPBeginPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 phase = fp->motion_vars.flags.flag2;

    if (phase == 0)
    {
        ftRemixDededeUSPApplyLateralMovement(fp);
        return;
    }
    if (phase != 2)
    {
        ftRemixDededeUSPBeginLaunch(fp);
    }
    ftRemixDededeUSPApplyVerticalMovement(fighter_gobj);
}

/* DededeUSP.begin_collision_ */
static void ftRemixDededeUSPBeginCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == 0)
    {
        mpCommonProcFighterOnFloor(fighter_gobj, ftRemixDededeUSPBeginSetAerial);
        return;
    }
    if (!mpCommonCheckFighterCeilHeavyCliff(fighter_gobj))
    {
        return;
    }
    ftRemixDededeUSPSetGroundOrCliff(fighter_gobj, fp);
}

/* DededeUSP.move_cancel_ */
static void ftRemixDededeUSPMoveCancel(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->physics.vel_air.y > 0.0F)
    {
        return;
    }
    /* The Remix source reads the high byte at 0x1BE with a 0x40 mask, which is B_BUTTON in button_tap. */
    if ((fp->input.pl.button_tap & B_BUTTON) || fp->input.pl.stick_range.y < DEDEDE_USP_CANCEL_STICK_Y)
    {
        ftRemixDededeUSPChangeStatus(fighter_gobj, nFTRemixDededeStatusUSPCancel);
    }
}

/* DededeUSP.move_physics_ */
static void ftRemixDededeUSPMovePhysics(GObj *fighter_gobj)
{
    ftRemixDededeUSPApplyVerticalMovement(fighter_gobj);
}

/* DededeUSP.move_collision_ */
static void ftRemixDededeUSPMoveCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!mpCommonCheckFighterCeilHeavyCliff(fighter_gobj))
    {
        return;
    }
    if (ftRemixDededeUSPSetGroundOrCliff(fighter_gobj, fp))
    {
        ftRemixDededeUSPChangeStatus(fighter_gobj, nFTRemixDededeStatusUSPLand);
        fp->physics.vel_ground.x = 0.0F;
        return;
    }
    if (fp->coll_data.mask_stat & DEDEDE_USP_CLIFF_MASK)
    {
        return;
    }
    if (fp->coll_data.mask_stat & DEDEDE_USP_SURFACE_CEILING)
    {
        ftRemixDededeUSPChangeStatus(fighter_gobj, nFTRemixDededeStatusUSPCeilingBonk);
        func_800269C0_275C0(DEDEDE_USP_FGM_CEILING_BONK);
    }
}

/* DededeUSP.cancel_main_ */
static void ftRemixDededeUSPCancelMain(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, DEDEDE_USP_LANDING_LAG, FALSE);
}

/* DededeUSP.cancel_collision_ */
static void ftRemixDededeUSPCancelCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!mpCommonCheckFighterCeilHeavyCliff(fighter_gobj))
    {
        return;
    }
    if (ftRemixDededeUSPSetGroundOrCliff(fighter_gobj, fp))
    {
        fp->status_vars.common.dead.wait = 0;
        ftRemixDededeUSPChangeStatus(fighter_gobj, nFTCommonStatusLandingFallSpecial);
    }
}

/* DededeUSP.landing_main_ */
static void ftRemixDededeUSPLandingMain(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixDededeUSPSpawnStars(fighter_gobj, fp);
    }
    ftAnimEndCheckSetStatus(fighter_gobj, ftCommonWaitSetStatus);
}

/* DededeUSP.ceiling_bonk_main_ */
static void ftRemixDededeUSPCeilingBonkMain(GObj *fighter_gobj)
{
    ftAnimEndCheckSetStatus(fighter_gobj, ftRemixDededeUSPCancelInitial);
}

/* DededeUSP.ceiling_bonk_collision_ */
static void ftRemixDededeUSPCeilingBonkCollision(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!mpCommonCheckFighterCeilHeavyCliff(fighter_gobj))
    {
        return;
    }
    if (fp->coll_data.mask_stat & DEDEDE_USP_CLIFF_MASK)
    {
        ftCommonCliffCatchSetStatus(fighter_gobj);
        return;
    }
    if (fp->coll_data.mask_stat & DEDEDE_USP_SURFACE_GROUND)
    {
        mpCommonSetFighterWaitOrLanding(fighter_gobj);
        return;
    }
    if (fp->coll_data.mask_curr & DEDEDE_USP_CEILING_MASK_CURR)
    {
        ftRemixDededeUSPCeilingSubroutine(fighter_gobj);
    }
}

const FTRemixProc dFTRemixDededeUSPProcs[] =
{
    { REMIX_SYM_DededeUSP__begin_initial_, ftRemixDededeUSPBeginInitial },
    { REMIX_SYM_DededeUSP__begin_main_, ftRemixDededeUSPBeginMain },
    { REMIX_SYM_DededeUSP__begin_physics_, ftRemixDededeUSPBeginPhysics },
    { REMIX_SYM_DededeUSP__begin_collision_, ftRemixDededeUSPBeginCollision },
    { REMIX_SYM_DededeUSP__move_cancel_, ftRemixDededeUSPMoveCancel },
    { REMIX_SYM_DededeUSP__move_physics_, ftRemixDededeUSPMovePhysics },
    { REMIX_SYM_DededeUSP__move_collision_, ftRemixDededeUSPMoveCollision },
    { REMIX_SYM_DededeUSP__landing_main_, ftRemixDededeUSPLandingMain },
    { REMIX_SYM_DededeUSP__cancel_main_, ftRemixDededeUSPCancelMain },
    { REMIX_SYM_DededeUSP__cancel_collision_, ftRemixDededeUSPCancelCollision },
    { REMIX_SYM_DededeUSP__ceiling_bonk_main_, ftRemixDededeUSPCeilingBonkMain },
    { REMIX_SYM_DededeUSP__ceiling_bonk_collision_, ftRemixDededeUSPCeilingBonkCollision }
};

const s32 dFTRemixDededeUSPProcsNum = ARRAY_COUNT(dFTRemixDededeUSPProcs);

#endif
