/**
 * ftremixpeach.c - C ports of Smash Remix Peach routines
 * (.toolchain/pkgs/smashremix-src/src/Peach/PeachSpecial.asm).
 *
 * Float's timer lives in the 0x0ADC passive_vars union (no decomp field for
 * Peach, so PEACH_FLOAT_TIMER names the u32 slot). Vanilla hooks that call the
 * non-static functions below are listed in docs/remix/notes/49_PEACH.md.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftchar/ftfox/ftfox.h>
#include <it/item.h>
#include <it/itmain.h>
#include <it/itcommon/itsword.h>
#include <it/itcommon/itbombhei.h>
#include <it/itremix/itremixturnip.h>
#include <it/itremix/itremixmrsaturn.h>
#include <sys/utils.h>

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);
extern void ftCaptainSpecialHiProcInterrupt(GObj *fighter_gobj);
extern void *func_800269C0_275C0(u16);

/* Called from vanilla hooks (see docs/remix/notes/49_PEACH.md). */
void ftRemixPeachFloatInitial(GObj *fighter_gobj);
sb32 ftRemixPeachJumpAerialCheckOrFloat(GObj *fighter_gobj);
sb32 ftRemixPeachFallOverride(GObj *fighter_gobj);
sb32 ftRemixPeachLightThrowCheckItemTypeThrow(FTStruct *fp);
void ftRemixPeachFloatAutoCancelAerial(FTStruct *fp);
void ftRemixPeachEndFloatOnHit(FTStruct *fp);
s32 ftRemixPeachFallSpecialStatus(FTStruct *fp, s32 status_id);
sb32 ftRemixPeachFloatHoldsHeight(FTStruct *fp);
void ftRemixPeachLandingSetLedgeFlag(FTStruct *fp);
void ftRemixPeachGroundedScriptRestoreFloat(FTStruct *fp);
void ftRemixPeachCliffRestoreFloat(FTStruct *fp);

#define PEACH_FLOAT_TIMER(fp) ((fp)->passive_vars.purin.unk_0x0)
#define PEACH_FLOAT_TIMER_START 150
#define PEACH_FLOAT_TIMER_END 1
#define PEACH_FLOAT_INIT_Y_DECAY 0.25F              /* 0x3E800000 (asm comment says 0.375; bits are 0.25) */
#define PEACH_FLOAT_INIT_Y_VELOCITY 24.0F           /* 0x41C00000 */
#define PEACH_FLOAT_DOUBLE_JUMP_FRAME 20.0F         /* 0x41A00000 */
#define PEACH_FLOAT_STICK_Y_START (-39)
#define PEACH_C_BUTTONS (U_CBUTTONS | D_CBUTTONS | L_CBUTTONS | R_CBUTTONS)

#define PEACH_NSP_Y_SPEED_INITIAL 30.0F             /* 0x41F00000 */
#define PEACH_NSP_X_SPEED 48.0F                     /* 0x42400000 */
#define PEACH_NSP_X_SMASH_SPEED 60.0F               /* 0x42700000 */
#define PEACH_NSP_RECOIL_X_SPEED (-34.0F)           /* 0xC2080000 */
#define PEACH_NSP_RECOIL_Y_SPEED 40.0F              /* 0x42200000 */
#define PEACH_NSP_AIR_FRICTION 1.0F                 /* 0x3F800000 */
#define PEACH_NSP_SHIELD_CLANG_FRAME 35.0F          /* 0x420C0000 */
#define PEACH_NSP_WALL_COLLISION_L 0x0001
#define PEACH_NSP_WALL_COLLISION_R 0x0020

#define PEACH_USP_AIR_Y_SPEED 82.0F                 /* 0x42A40000 */
#define PEACH_USP_GROUND_Y_SPEED 84.0F              /* 0x42A80000 */
#define PEACH_USP_X_SPEED 18.0F                     /* 0x41900000 */
#define PEACH_USP_FLOAT_GRAVITY 0.5F                /* 0x3F000000 */
#define PEACH_USP_FLOAT_FALL_SPEED 13.0F            /* 0x41500000 */
#define PEACH_USP_AIR_ACCELERATION 0.010009765625F  /* 0x3C240000 */
#define PEACH_USP_AIR_SPEED 24.0F                   /* 0x41C00000 */
#define PEACH_USP_LANDING_FSM 0.375F                /* 0x3EC00000 */
#define PEACH_USP_DECELERATE 0.875F                 /* 0x3F600000 */
#define PEACH_USP_STICK_Y_OPEN 40
#define PEACH_USP_STICK_Y_CLOSE (-39)

#define PEACH_DSP_TURNIP_ITEM_ID 0x4B               /* Item.Turnip.id: 0x2D + 30, last add_item in Item.asm */
#define PEACH_DSP_BINGO_BASE_DAMAGE 30              /* Item.Turnip.STITCH_BASE_DAMAGE */
#define PEACH_DSP_BINGO_SFX 0x5D6
#define PEACH_DSP_ITEM_ROLL_RANGE 128
#define PEACH_DSP_ITEM_ROLL_EXTRA 127
#define PEACH_DSP_EXTRA_ROLL_RANGE 6

enum
{
    nFTRemixPeachStatusFloat = 0xDE,
    nFTRemixPeachStatusNSPG = 0xDF,
    nFTRemixPeachStatusNSPA = 0xE0,
    nFTRemixPeachStatusNSPRecoil = 0xE1,
    nFTRemixPeachStatusUSPG = 0xE3,
    nFTRemixPeachStatusUSPA = 0xE4,
    nFTRemixPeachStatusUSPOpen = 0xE5,
    nFTRemixPeachStatusUSPFloat = 0xE6,
    nFTRemixPeachStatusUSPClose = 0xE7,
    nFTRemixPeachStatusUSPFall = 0xE8,
    nFTRemixPeachStatusDSPPull = 0xE9
};

/* Kirby.asm add_new_action(KIRBY, PEACH_NSP_*): id = 0x67C / 0x14 + index + 0xDC,
 * with the three PEACH_NSP entries at new-action indices 131, 132, 133. */
enum
{
    nFTRemixPeachKirbyStatusNSPGround = 0x1B2,
    nFTRemixPeachKirbyStatusNSPAir = 0x1B3,
    nFTRemixPeachKirbyStatusNSPRecoil = 0x1B4
};

enum
{
    nFTRemixPeachNSPMoveNone = 0,
    nFTRemixPeachNSPMoveBegin = 1,
    nFTRemixPeachNSPMove = 2,
    nFTRemixPeachNSPMoveEnd = 3,
    nFTRemixPeachNSPMoveDone = 4
};

enum
{
    nFTRemixPeachUSPMoveBegin = 1,
    nFTRemixPeachUSPMoveBeginMove = 2,
    nFTRemixPeachUSPMove = 3,
    nFTRemixPeachUSPMoveEnd = 4
};

static sb32 ftRemixPeachIsPeachOrPoly(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_PEACH) || (fp->fkind == REMIX_ID_NPEACH);
}

static sb32 ftRemixPeachIsPeach(FTStruct *fp)
{
    return fp->fkind == REMIX_ID_PEACH;
}

static sb32 ftRemixPeachIsFloatInUse(FTStruct *fp)
{
    return (PEACH_FLOAT_TIMER(fp) != 0) && (PEACH_FLOAT_TIMER(fp) != PEACH_FLOAT_TIMER_END);
}

static s32 ftRemixPeachNSPStatus(FTStruct *fp, s32 peach_status, s32 kirby_status)
{
    return (fp->fkind == REMIX_ID_PEACH) ? peach_status : kirby_status;
}

static void ftRemixPeachClearFastFallFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Friction with a caller-chosen value; ftPhysicsApplyAirVelXFriction reads it from FTAttributes. */
static void ftRemixPeachApplyAirVelXFriction(FTStruct *fp, f32 friction)
{
    if (fp->physics.vel_air.x < 0.0F)
    {
        fp->physics.vel_air.x += friction;

        if (fp->physics.vel_air.x >= 0.0F)
        {
            fp->physics.vel_air.x = 0.0F;
        }
    }
    else
    {
        fp->physics.vel_air.x -= friction;

        if (fp->physics.vel_air.x <= 0.0F)
        {
            fp->physics.vel_air.x = 0.0F;
        }
    }
}

static sb32 ftRemixPeachIsHoldingTurnip(FTStruct *fp)
{
    if (fp->item_gobj == NULL)
    {
        return FALSE;
    }
    return itGetStruct(fp->item_gobj)->kind == PEACH_DSP_TURNIP_ITEM_ID;
}

static sb32 ftRemixPeachIsHoldingBingoTurnip(FTStruct *fp)
{
    return ftRemixPeachIsHoldingTurnip(fp) && (itGetStruct(fp->item_gobj)->attack_coll.damage == PEACH_DSP_BINGO_BASE_DAMAGE);
}

/* ---------------------------------------------------------------------------
 * Float
 * ------------------------------------------------------------------------- */

/* Double jump start frame gate: JumpAerialF/B may not start Float before the frame. */
static sb32 ftRemixPeachFloatDoubleJumpGate(GObj *fighter_gobj, FTStruct *fp)
{
    if ((fp->status_id != nFTCommonStatusJumpAerialF) && (fp->status_id != nFTCommonStatusJumpAerialB))
    {
        return TRUE;
    }
    return fighter_gobj->anim_frame > PEACH_FLOAT_DOUBLE_JUMP_FRAME;
}

/* Start conditions of check_float_. A stick held down starts Float at once. */
static sb32 ftRemixPeachFloatCanStart(GObj *fighter_gobj, FTStruct *fp)
{
    if (!ftRemixPeachIsPeachOrPoly(fp) || (PEACH_FLOAT_TIMER(fp) != 0))
    {
        return FALSE;
    }
    if (!(fp->input.pl.button_hold & PEACH_C_BUTTONS))
    {
        return FALSE;
    }
    if (fp->input.pl.stick_range.y < PEACH_FLOAT_STICK_Y_START)
    {
        return TRUE;
    }
    if ((fp->jumps_used != fp->attr->jumps_max) && (fp->input.pl.button_tap & PEACH_C_BUTTONS))
    {
        return FALSE;
    }
    if (fp->physics.vel_air.y > 0.0F)
    {
        return FALSE;
    }
    return ftRemixPeachFloatDoubleJumpGate(fighter_gobj, fp);
}

/* PeachFloat.main_ (ProcUpdate): falls out of Float once the timer runs out. */
static void ftRemixPeachFloatProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (PEACH_FLOAT_TIMER(fp) == PEACH_FLOAT_TIMER_END)
    {
        ftCommonFallSetStatus(fighter_gobj);
    }
}

/* PeachFloat.interrupt_: a B or A action change ends Float unless it is an aerial attack. */
static void ftRemixPeachFloatProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!ftCommonSpecialAirCheckInterruptCommon(fighter_gobj) && !ftCommonAttackAirCheckInterruptCommon(fighter_gobj))
    {
        return;
    }
    if ((fp->status_id < nFTCommonStatusAttackAirN) || (fp->status_id > nFTCommonStatusAttackAirLw))
    {
        PEACH_FLOAT_TIMER(fp) = PEACH_FLOAT_TIMER_END;
    }
}

/* PeachFloat.initial_ (not in the action table): starts Float from a Jump/Fall hook. */
void ftRemixPeachFloatInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixPeachStatusFloat, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->physics.vel_air.y = (fp->physics.vel_air.y * PEACH_FLOAT_INIT_Y_DECAY) + PEACH_FLOAT_INIT_Y_VELOCITY;
}

/* Replaces ftCommonJumpAerialCheckInterruptCommon in the Jump, JumpAerial, Fall and Pass interrupts. */
sb32 ftRemixPeachJumpAerialCheckOrFloat(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixPeachFloatCanStart(fighter_gobj, fp))
    {
        PEACH_FLOAT_TIMER(fp) = PEACH_FLOAT_TIMER_START;
        ftRemixPeachFloatInitial(fighter_gobj);
        return TRUE;
    }
    return ftCommonJumpAerialCheckInterruptCommon(fighter_gobj);
}

/* Replaces the ftMainSetStatus call in ftCommonFallSetStatus; TRUE means Float took over. */
sb32 ftRemixPeachFallOverride(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (!ftRemixPeachIsPeachOrPoly(fp) || !ftRemixPeachIsFloatInUse(fp))
    {
        return FALSE;
    }
    if (!(fp->input.pl.button_hold & PEACH_C_BUTTONS))
    {
        return FALSE;
    }
    ftRemixPeachFloatInitial(fighter_gobj);
    return TRUE;
}

/* Replaces ftCommonLightThrowCheckItemTypeThrow in ftCommonAttackAirCheckInterruptCommon. */
sb32 ftRemixPeachLightThrowCheckItemTypeThrow(FTStruct *fp)
{
    if (ftRemixPeachIsPeachOrPoly(fp) && ftRemixPeachIsFloatInUse(fp))
    {
        return FALSE;
    }
    return ftCommonLightThrowCheckItemTypeThrow(fp);
}

/* Inserted after the ftMainSetStatus call in ftCommonAttackAirCheckInterruptCommon. */
void ftRemixPeachFloatAutoCancelAerial(FTStruct *fp)
{
    if (ftRemixPeachIsPeachOrPoly(fp) && ftRemixPeachIsFloatInUse(fp))
    {
        fp->proc_map = mpCommonProcFighterWaitOrLanding;
    }
}

/* Inserted before ftParamStopVoice in ftParamStopVoiceRunProcDamage. */
void ftRemixPeachEndFloatOnHit(FTStruct *fp)
{
    if (ftRemixPeachIsPeachOrPoly(fp) && (PEACH_FLOAT_TIMER(fp) != 0))
    {
        PEACH_FLOAT_TIMER(fp) = PEACH_FLOAT_TIMER_END;
    }
}

/* Replaces the status id passed to ftMainSetStatus in ftCommonFallSpecialSetStatus. */
s32 ftRemixPeachFallSpecialStatus(FTStruct *fp, s32 status_id)
{
    if (ftRemixPeachIsPeach(fp) && (fp->status_id == nFTRemixPeachStatusUSPClose))
    {
        return nFTRemixPeachStatusUSPFall;
    }
    return status_id;
}

/* Replaces the gravity call at the top of ftPhysicsApplyGravityClampTVel. TRUE skips gravity. */
sb32 ftRemixPeachFloatHoldsHeight(FTStruct *fp)
{
    u32 timer;

    if (!ftRemixPeachIsPeachOrPoly(fp))
    {
        return FALSE;
    }
    timer = PEACH_FLOAT_TIMER(fp);
    if ((timer == 0) || (timer == PEACH_FLOAT_TIMER_END))
    {
        return FALSE;
    }
    PEACH_FLOAT_TIMER(fp) = timer - 1;
    if (timer == PEACH_FLOAT_TIMER_START)
    {
        return FALSE;
    }
    if (!(fp->input.pl.button_hold & PEACH_C_BUTTONS))
    {
        PEACH_FLOAT_TIMER(fp) = PEACH_FLOAT_TIMER_END;
        return FALSE;
    }
    fp->physics.vel_air.y = 0.0F;
    return TRUE;
}

/* Remix ledge_flag: set when Peach lands, read by the grounded script to decide
 * whether Float is restored. A single global, as in Remix. */
static sb32 sFTRemixPeachLedgeFlag;

/* Inserted before the cliff-catch mpCommonSetFighterLandingParams call (0x800DE680) in
 * mpCommonRunFighterSpecialCollisions. */
void ftRemixPeachLandingSetLedgeFlag(FTStruct *fp)
{
    if (ftRemixPeachIsPeachOrPoly(fp))
    {
        sFTRemixPeachLedgeFlag = TRUE;
    }
}

/* Peach grounded_script_: restores Float unless the landing was a ledge grab, then clears the flag. */
void ftRemixPeachGroundedScriptRestoreFloat(FTStruct *fp)
{
    /* Peach.asm:507 installs grounded_script_ for PEACH only; NPEACH keeps Fox's. */
    if (!ftRemixPeachIsPeach(fp))
    {
        return;
    }
    if (!sFTRemixPeachLedgeFlag)
    {
        PEACH_FLOAT_TIMER(fp) = 0;
    }
    sFTRemixPeachLedgeFlag = FALSE;
}

/* Inserted after the cliffmotion.cliff_id assignment in ftCommonCliffQuickOrSlowSetStatus. */
void ftRemixPeachCliffRestoreFloat(FTStruct *fp)
{
    if (ftRemixPeachIsPeachOrPoly(fp))
    {
        PEACH_FLOAT_TIMER(fp) = 0;
    }
}

/* ---------------------------------------------------------------------------
 * Down Special (Peach DSP: turnip pull)
 * ------------------------------------------------------------------------- */

/* PeachDSP.create_and_assign_item_: a 1/6 Beam Sword, 2/6 Bob-omb or 3/6 Mr. Saturn
 * on a 1/128 roll, a Turnip otherwise. */
static GObj* ftRemixPeachDSPMakePulledItem(GObj *fighter_gobj, Vec3f *pos, Vec3f *vel)
{
    s32 extra_roll;

    if (syUtilsRandIntRange(PEACH_DSP_ITEM_ROLL_RANGE) != PEACH_DSP_ITEM_ROLL_EXTRA)
    {
        return itRemixTurnipMakeItem(fighter_gobj, pos, vel, 0);
    }
    extra_roll = syUtilsRandIntRange(PEACH_DSP_EXTRA_ROLL_RANGE);

    if (extra_roll == 0)
    {
        return itSwordMakeItem(fighter_gobj, pos, vel, 0);
    }
    if (extra_roll >= 3)
    {
        return itRemixMrSaturnMakeItem(fighter_gobj, pos, vel, 0);
    }
    return itBombHeiMakeItem(fighter_gobj, pos, vel, 0);
}

/* Direct spawns skip the spawn gfx, which is what Item.skip_item_spawn_gfx_ does in Remix. */
static void ftRemixPeachDSPCreateAndAssignItem(GObj *fighter_gobj)
{
    Vec3f vel = { 0.0F, 0.0F, 0.0F };
    GObj *item_gobj = ftRemixPeachDSPMakePulledItem(fighter_gobj, &DObjGetStruct(fighter_gobj)->translate.vec.f, &vel);

    if (item_gobj != NULL)
    {
        itMainSetFighterHold(item_gobj, fighter_gobj);
    }
}

/* PeachDSP.air_initial_ */
static void ftRemixPeachDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (ftRemixPeachIsHoldingTurnip(fp))
    {
        ftCommonItemThrowSetStatus(fighter_gobj, nFTCommonStatusLightThrowAirF4);
    }
}

/* PeachDSP.ground_initial_ */
static void ftRemixPeachDSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;
    if (fp->item_gobj == NULL)
    {
        ftMainSetStatus(fighter_gobj, nFTRemixPeachStatusDSPPull, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
        return;
    }
    if (ftRemixPeachIsHoldingTurnip(fp))
    {
        ftCommonItemThrowSetStatus(fighter_gobj, nFTCommonStatusLightThrowF4);
    }
}

/* PeachDSP.main (ProcUpdate of DSPPull). */
static void ftRemixPeachDSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag0 != 0)
    {
        fp->motion_vars.flags.flag0 = 0;
        ftRemixPeachDSPCreateAndAssignItem(fighter_gobj);
        return;
    }
    if (fp->motion_vars.flags.flag1 == 0)
    {
        if (fighter_gobj->anim_frame <= 0.0F)
        {
            mpCommonSetFighterWaitOrFall(fighter_gobj);
        }
        return;
    }
    fp->motion_vars.flags.flag1 = 0;
    if (ftRemixPeachIsHoldingBingoTurnip(fp))
    {
        func_800269C0_275C0(PEACH_DSP_BINGO_SFX);
    }
}

/* ---------------------------------------------------------------------------
 * Neutral Special (Peach NSP: bomber, Kirby copy via fkind)
 * ------------------------------------------------------------------------- */

static void ftRemixPeachNSPRecoilInitial(GObj *fighter_gobj);
static void ftRemixPeachNSPShieldClangHit(GObj *fighter_gobj);

/* Shared setup of the NSP initials: status, anim events, link NSP, cleared temp vars. */
static void ftRemixPeachNSPSetupStatus(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_MODELPART);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftLinkSpecialNProcStatus(fighter_gobj);
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->proc_shield = ftRemixPeachNSPShieldClangHit;
    fp->proc_hit = ftRemixPeachNSPRecoilInitial;
}

/* PeachNSP.ground_initial_ */
static void ftRemixPeachNSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachNSPSetupStatus(fighter_gobj, ftRemixPeachNSPStatus(fp, nFTRemixPeachStatusNSPG, nFTRemixPeachKirbyStatusNSPGround));
}

/* PeachNSP.air_initial_ */
static void ftRemixPeachNSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachNSPSetupStatus(fighter_gobj, ftRemixPeachNSPStatus(fp, nFTRemixPeachStatusNSPA, nFTRemixPeachKirbyStatusNSPAir));
    ftRemixPeachClearFastFallFlags(fp);
    fp->physics.vel_air.y = PEACH_NSP_Y_SPEED_INITIAL;
}

/* Shared recoil body for the NSP bounce (proc_hit and wall bounce). */
static void ftRemixPeachNSPRecoilInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id;

    if (fp->ga == nMPKineticsGround)
    {
        mpCommonSetFighterAir(fp);
    }
    status_id = ftRemixPeachNSPStatus(fp, nFTRemixPeachStatusNSPRecoil, nFTRemixPeachKirbyStatusNSPRecoil);
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->physics.vel_air.x = PEACH_NSP_RECOIL_X_SPEED * (f32)fp->lr;
    fp->physics.vel_air.y = PEACH_NSP_RECOIL_Y_SPEED;
}

/* proc_shield: restarts the current NSP status at frame 35 when shielded or clanged. */
static void ftRemixPeachNSPShieldClangHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, fp->status_id, PEACH_NSP_SHIELD_CLANG_FRAME, 1.0F, FTSTATUS_PRESERVE_NONE);
}

/* Remix ground_to_air_: switches the grounded NSP to its aerial status. */
static void ftRemixPeachNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id;

    mpCommonSetFighterAir(fp);
    status_id = ftRemixPeachNSPStatus(fp, nFTRemixPeachStatusNSPA, nFTRemixPeachKirbyStatusNSPAir);
    ftMainSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
    fp->proc_shield = ftRemixPeachNSPShieldClangHit;
    fp->proc_hit = ftRemixPeachNSPRecoilInitial;
}

/* Remix air_to_ground_: switches the aerial NSP to its grounded status. */
static void ftRemixPeachNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id;

    mpCommonSetFighterGround(fp);
    status_id = ftRemixPeachNSPStatus(fp, nFTRemixPeachStatusNSPG, nFTRemixPeachKirbyStatusNSPGround);
    ftMainSetStatus(fighter_gobj, status_id, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_HIT | FTSTATUS_PRESERVE_COLANIM);
    fp->proc_shield = ftRemixPeachNSPShieldClangHit;
    fp->proc_hit = ftRemixPeachNSPRecoilInitial;
}

/* Bounces Peach off a wall while the NSP move state is MOVE. */
static void ftRemixPeachNSPBounceOffWall(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u16 wall_mask;

    if (fp->motion_vars.flags.flag2 != nFTRemixPeachNSPMove)
    {
        return;
    }
    wall_mask = (fp->lr >= 0) ? PEACH_NSP_WALL_COLLISION_L : PEACH_NSP_WALL_COLLISION_R;
    if ((fp->coll_data.mask_prev & wall_mask) == 0)
    {
        return;
    }
    ftRemixPeachNSPRecoilInitial(fighter_gobj);
}

/* Grounded NSP movement (PeachNSP.ground_physics_). */
static void ftRemixPeachNSPBeginGroundMove(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->physics.vel_ground.x = (fp->status_vars.link.specialn.is_smash) ? PEACH_NSP_X_SMASH_SPEED : PEACH_NSP_X_SPEED;
    fp->motion_vars.flags.flag2 = nFTRemixPeachNSPMove;
    ftPhysicsSetGroundVelTransferAir(fighter_gobj);
}

static void ftRemixPeachNSPGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 move_state = fp->motion_vars.flags.flag2;

    if (move_state == nFTRemixPeachNSPMoveBegin)
    {
        ftRemixPeachNSPBeginGroundMove(fighter_gobj);
    }
    else if (move_state == nFTRemixPeachNSPMove)
    {
        return;
    }
    else if (move_state == nFTRemixPeachNSPMoveEnd)
    {
        fp->motion_vars.flags.flag2 = nFTRemixPeachNSPMoveDone;
    }
    else ftPhysicsApplyGroundVelFriction(fighter_gobj);
}

/* Non-actionable aerial NSP movement (air_end_physics_): gravity and fixed air friction. */
static void ftRemixPeachNSPEndAirPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    ftPhysicsApplyGravityClampTVel(fp, attr->gravity, attr->tvel_base);
    ftRemixPeachApplyAirVelXFriction(fp, PEACH_NSP_AIR_FRICTION);
}

/* Starts aerial NSP movement: x speed follows facing, y is cleared. */
static void ftRemixPeachNSPBeginAirMove(FTStruct *fp)
{
    f32 speed = (fp->status_vars.link.specialn.is_smash) ? PEACH_NSP_X_SMASH_SPEED : PEACH_NSP_X_SPEED;

    fp->physics.vel_air.x = speed * (f32)fp->lr;
    fp->physics.vel_air.y = 0.0F;
    fp->motion_vars.flags.flag2 = nFTRemixPeachNSPMove;
}

static void ftRemixPeachNSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 move_state = fp->motion_vars.flags.flag2;

    if (move_state == nFTRemixPeachNSPMoveBegin)
    {
        ftRemixPeachNSPBeginAirMove(fp);
    }
    else if (move_state == nFTRemixPeachNSPMove)
    {
        return;
    }
    else if (move_state == nFTRemixPeachNSPMoveEnd)
    {
        fp->physics.vel_air.x *= 0.5F;
        fp->motion_vars.flags.flag2 = nFTRemixPeachNSPMoveDone;
        ftRemixPeachNSPEndAirPhysics(fighter_gobj);
    }
    else if (move_state == nFTRemixPeachNSPMoveDone)
    {
        ftRemixPeachNSPEndAirPhysics(fighter_gobj);
    }
    else ftPhysicsApplyAirVelDrift(fighter_gobj);
}

static void ftRemixPeachNSPGroundProcMap(GObj *fighter_gobj)
{
    if (mpCommonProcFighterOnFloor(fighter_gobj, ftRemixPeachNSPGroundToAir) == FALSE)
    {
        return;
    }
    ftRemixPeachNSPBounceOffWall(fighter_gobj);
}

static void ftRemixPeachNSPAirProcMap(GObj *fighter_gobj)
{
    if (mpCommonProcFighterLanding(fighter_gobj, ftRemixPeachNSPAirToGround) != FALSE)
    {
        return;
    }
    ftRemixPeachNSPBounceOffWall(fighter_gobj);
}

/* ---------------------------------------------------------------------------
 * Up Special (Peach USP: parasol, Float state machine)
 * ------------------------------------------------------------------------- */

/* Shared USP initial: status with model parts kept, then anim events. */
static void ftRemixPeachUSPSetStatus(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, FTSTATUS_PRESERVE_MODELPART);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

static void ftRemixPeachUSPOpenInitial(GObj *fighter_gobj);
static void ftRemixPeachUSPFloatInitial(GObj *fighter_gobj);
static void ftRemixPeachUSPCloseInitial(GObj *fighter_gobj);

/* PeachUSP.air_initial_ */
static void ftRemixPeachUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachUSPSetStatus(fighter_gobj, nFTRemixPeachStatusUSPA);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = nFTRemixPeachUSPMoveBegin;
    ftRemixPeachClearFastFallFlags(fp);
    fp->physics.vel_air.y = fp->attr->gravity;
}

/* PeachUSP.ground_initial_ */
static void ftRemixPeachUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachUSPSetStatus(fighter_gobj, nFTRemixPeachStatusUSPG);
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = nFTRemixPeachUSPMoveBegin;
}

/* Remix open_initial_ */
static void ftRemixPeachUSPOpenInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachUSPSetStatus(fighter_gobj, nFTRemixPeachStatusUSPOpen);
    fp->motion_vars.flags.flag1 = 1;
    ftRemixPeachClearFastFallFlags(fp);
    fp->physics.vel_air.y = 0.0F;
}

/* Remix float_initial_ */
static void ftRemixPeachUSPFloatInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachUSPSetStatus(fighter_gobj, nFTRemixPeachStatusUSPFloat);
    fp->motion_vars.flags.flag1 = 1;
}

/* Remix close_initial_ */
static void ftRemixPeachUSPCloseInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixPeachUSPSetStatus(fighter_gobj, nFTRemixPeachStatusUSPClose);
    fp->motion_vars.flags.flag1 = 1;
}

/* PeachUSP.main_ (ProcUpdate of USPG/USPA): opens the parasol when the animation ends. */
static void ftRemixPeachUSPProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixPeachUSPOpenInitial(fighter_gobj);
    }
}

/* PeachUSP.open_main_ */
static void ftRemixPeachUSPOpenProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftRemixPeachUSPFloatInitial(fighter_gobj);
    }
}

/* PeachUSP.close_main_: Fox's up special end copied with Peach's landing lag and no interrupt. */
static void ftRemixPeachUSPCloseProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        ftCommonFallSpecialSetStatus(fighter_gobj, FTFOX_FIREFOX_AIR_DRIFT, FALSE, TRUE, FALSE, PEACH_USP_LANDING_FSM, FALSE);
    }
}

/* PeachUSP.change_direction_: turns Peach when temp variable 2 is 2. */
static void ftRemixPeachUSPChangeDirection(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 2)
    {
        ftCaptainSpecialHiProcInterrupt(fighter_gobj);
    }
}

/* PeachUSP.float_interrupt_: a down stick during Float starts the close. */
static void ftRemixPeachUSPFloatProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.stick_range.y < PEACH_USP_STICK_Y_CLOSE)
    {
        ftRemixPeachUSPCloseInitial(fighter_gobj);
    }
}

/* PeachUSP.fall_interrupt_: an up stick during the fall special opens the parasol. */
static void ftRemixPeachUSPFallProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->input.pl.stick_range.y >= PEACH_USP_STICK_Y_OPEN)
    {
        ftRemixPeachUSPOpenInitial(fighter_gobj);
    }
}

/* Remix air control for USP: clamps drift with Peach's own accel and speed. */
static void ftRemixPeachUSPAirControl(FTStruct *fp)
{
    ftPhysicsClampAirVelXStickRange(fp, FTPHYSICS_AIRDRIFT_CLAMP_RANGE_MIN, PEACH_USP_AIR_ACCELERATION, PEACH_USP_AIR_SPEED);
}

/* Air drift of PeachUSP.physics_: vanilla air physics, control only in the MOVE state. */
static void ftRemixPeachUSPApplyAirDrift(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    (fp->is_fastfall) ? ftPhysicsApplyFastFall(fp, attr) : ftPhysicsApplyGravityDefault(fp, attr);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) != FALSE)
    {
        return;
    }
    if (fp->motion_vars.flags.flag2 == nFTRemixPeachUSPMove)
    {
        ftRemixPeachUSPAirControl(fp);
    }
    ftPhysicsApplyAirVelXFriction(fp, attr);
}

/* PeachUSP.physics_ state changes that run after the air drift (begin slowdown and move start). */
static void ftRemixPeachUSPUpdateMoveState(FTStruct *fp)
{
    u32 move_state = fp->motion_vars.flags.flag2;
    s32 is_grounded_action = (fp->status_id == nFTRemixPeachStatusUSPG);
    f32 y_speed;

    if ((move_state == nFTRemixPeachUSPMoveBegin) && !is_grounded_action)
    {
        fp->physics.vel_air.x *= PEACH_USP_DECELERATE;
        fp->physics.vel_air.y = 0.0F;
    }
    if (move_state == nFTRemixPeachUSPMoveBeginMove)
    {
        y_speed = is_grounded_action ? PEACH_USP_GROUND_Y_SPEED : PEACH_USP_AIR_Y_SPEED;
        fp->physics.vel_air.x = PEACH_USP_X_SPEED * (f32)fp->lr;
        fp->physics.vel_air.y = y_speed;
        fp->motion_vars.flags.flag2 = nFTRemixPeachUSPMove;
        fp->jumps_used = fp->attr->jumps_max;
    }
    if (move_state == nFTRemixPeachUSPMoveEnd)
    {
        fp->physics.vel_air.x *= PEACH_USP_DECELERATE;
    }
}

/* PeachUSP.physics_ (ProcPhysics of USPG and USPA). */
static void ftRemixPeachUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        ftPhysicsApplyGroundVelFriction(fighter_gobj);
        return;
    }
    ftRemixPeachUSPApplyAirDrift(fighter_gobj);
    ftRemixPeachUSPUpdateMoveState(fp);
}

/* PeachUSP.float_physics_ (ProcPhysics of USPOpen and USPFloat): reduced gravity, no fast fall. */
static void ftRemixPeachUSPFloatProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTAttributes *attr = fp->attr;

    ftPhysicsApplyGravityClampTVel(fp, PEACH_USP_FLOAT_GRAVITY, PEACH_USP_FLOAT_FALL_SPEED);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)
    {
        ftPhysicsClampAirVelXStickDefault(fp, attr);
        ftPhysicsApplyAirVelXFriction(fp, attr);
    }
}

/* PeachUSP.collision_: Mario's up special collision with Peach's landing lag. */
static void ftRemixPeachUSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsAir)
    {
        mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
        return;
    }
    if ((fp->motion_vars.flags.flag1 == 0) || (fp->physics.vel_air.y >= 0.0F))
    {
        mpCommonCheckFighterProject(fighter_gobj);
    }
    else if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) != FALSE)
    {
        if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
        {
            ftCommonCliffCatchSetStatus(fighter_gobj);
        }
        else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, PEACH_USP_LANDING_FSM);
    }
}

/* Remix address -> port routine for every Peach routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixPeachProcs[] =
{
    { REMIX_SYM_PeachDSP__air_initial_, ftRemixPeachDSPAirInitial },
    { REMIX_SYM_PeachDSP__ground_initial_, ftRemixPeachDSPGroundInitial },
    { REMIX_SYM_PeachDSP__main, ftRemixPeachDSPProcUpdate },
    { REMIX_SYM_PeachFloat__interrupt_, ftRemixPeachFloatProcInterrupt },
    { REMIX_SYM_PeachFloat__main_, ftRemixPeachFloatProcUpdate },
    { REMIX_SYM_PeachNSP__air_collision_, ftRemixPeachNSPAirProcMap },
    { REMIX_SYM_PeachNSP__air_initial_, ftRemixPeachNSPAirInitial },
    { REMIX_SYM_PeachNSP__air_physics_, ftRemixPeachNSPAirProcPhysics },
    { REMIX_SYM_PeachNSP__ground_collision_, ftRemixPeachNSPGroundProcMap },
    { REMIX_SYM_PeachNSP__ground_initial_, ftRemixPeachNSPGroundInitial },
    { REMIX_SYM_PeachNSP__ground_physics_, ftRemixPeachNSPGroundProcPhysics },
    { REMIX_SYM_PeachUSP__air_initial_, ftRemixPeachUSPAirInitial },
    { REMIX_SYM_PeachUSP__change_direction_, ftRemixPeachUSPChangeDirection },
    { REMIX_SYM_PeachUSP__close_main_, ftRemixPeachUSPCloseProcUpdate },
    { REMIX_SYM_PeachUSP__collision_, ftRemixPeachUSPProcMap },
    { REMIX_SYM_PeachUSP__fall_interrupt_, ftRemixPeachUSPFallProcInterrupt },
    { REMIX_SYM_PeachUSP__float_interrupt_, ftRemixPeachUSPFloatProcInterrupt },
    { REMIX_SYM_PeachUSP__float_physics_, ftRemixPeachUSPFloatProcPhysics },
    { REMIX_SYM_PeachUSP__ground_initial_, ftRemixPeachUSPGroundInitial },
    { REMIX_SYM_PeachUSP__main_, ftRemixPeachUSPProcUpdate },
    { REMIX_SYM_PeachUSP__open_main_, ftRemixPeachUSPOpenProcUpdate },
    { REMIX_SYM_PeachUSP__physics_, ftRemixPeachUSPProcPhysics }
};

const s32 dFTRemixPeachProcsNum = ARRAY_COUNT(dFTRemixPeachProcs);

#endif
