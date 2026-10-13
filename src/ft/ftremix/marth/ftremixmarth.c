/**
 * ftremixmarth.c - C ports of Smash Remix Marth routines
 * (.toolchain/pkgs/smashremix-src/src/Marth/MarthSpecial.asm).
 *
 * The specials keep their phase in temp variables (motion_vars.flags flag0/1/2,
 * set by moveset commands 5C0000XX and 580000XX) and in player struct words the
 * decomp union does not name (MARTH_OFF_*, read through ftRemixMarthWordAt).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_symbols.h"
#include <ft/ftchar/ftmario/ftmario.h>

#define MARTH_USP_BEGIN 1
#define MARTH_USP_BEGIN_MOVE 2
#define MARTH_USP_MOVE 3
#define MARTH_USP_END_MOVE 4
#define MARTH_USP_END 5
#define MARTH_USP_CHANGE_DIRECTION 2

#define MARTH_USP_AIR_Y_SPEED 400.0F                   /* 0x43C80000 */
#define MARTH_USP_AIR_Y_SPEED_ROY 116.0F               /* 0x42E80000 */
#define MARTH_USP_GROUND_Y_SPEED 420.0F                /* 0x43D20000 */
#define MARTH_USP_GROUND_Y_SPEED_ROY 120.0F            /* 0x42F00000 */
#define MARTH_USP_X_SPEED 10.0F                        /* 0x41200000 */
#define MARTH_USP_STICK_X_FACTOR 2.0F                  /* 0x40000000 */
#define MARTH_USP_STICK_X_FACTOR_ROY 0.75F             /* 0x3F400000 */
#define MARTH_USP_STICK_Y_FACTOR 0.5F                  /* 0x3F000000; the asm comment says 0.375 but the constant is 0.5 */
#define MARTH_USP_BEGIN_X_DECAY 0.875F                 /* 0x3F600000 */
#define MARTH_USP_END_X_DECAY 0.125F                   /* 0x3E000000 */
#define MARTH_USP_END_Y_DECAY 0.09375F                 /* 0x3DC00000 */
#define MARTH_USP_END_Y_DECAY_ROY 0.375F               /* 0x3EC00000 */
#define MARTH_USP_END_AIR_ACCELERATION 0.009765625F    /* 0x3C200000 */
#define MARTH_USP_END_AIR_SPEED 24.0F                  /* 0x41C00000 */
#define MARTH_USP_LANDING_LAG 0.375F                   /* 0x3EC00000 */

#define MARTH_NSP_Y_SPEED 36.0F                        /* 0x42100000 */
#define MARTH_NSP_Y_SPEED_STALE 16.0F                  /* 0x41800000 */
#define MARTH_NSP_Y_SPEED_SECOND 8.0F                  /* 0x41000000 */
#define MARTH_AIR_X_DECAY 0.875F                       /* 0x3F600000 */
#define MARTH_KIRBY_ROY_HAT 0x2C
#define MARTH_KIRBY_ROY_NSP_OFFSET 101

#define MARTH_DSP_AIR_X_DECAY 0.5F                     /* 0x3F000000 */
#define MARTH_DSP_AIR_GRAVITY_BOOST 1.5F               /* 0x3FC00000 */
#define MARTH_DSP_INVINCIBLE_ARMOR 131072.0F           /* 0x48000000 */
#define MARTH_DSP_FACING_ROTATION F_CST_DTOR32(90.0F)  /* RAM 0x8012FE90 assumed to hold this value; not verified */

#define MARTH_TRANSITION_FLAGS_NSP 0x2003              /* keeps sword trails, gfx routines, hitboxes */
#define MARTH_TRANSITION_FLAGS_DSP 0x2803              /* also keeps the 3C command FGM */

#define MARTH_OFF_KIRBY_HAT 0x0980                     /* s8 hat id; overlaps FTStruct.modelpart_status */
#define MARTH_OFF_PSEUDO_JUMP_MARTH 0x0ADC             /* s32 NSPA first-use flag */
#define MARTH_OFF_PSEUDO_JUMP_KIRBY 0x0AE0             /* s32 NSPA first-use flag for Kirby copies */
#define MARTH_OFF_NSP_STAGE 0x0B20                     /* s32 neutral special stage: 0, 1 or 2 */
#define MARTH_OFF_DSP_HIT_DETECTION 0x0B18             /* s32 counter hit flag */

enum
{
    nFTRemixMarthStatusUSPG = 0xDE,
    nFTRemixMarthStatusUSPA = 0xDF,
    nFTRemixMarthStatusNSPG_1 = 0xE0,
    nFTRemixMarthStatusNSPG_2_High = 0xE1,
    nFTRemixMarthStatusNSPG_2_Mid = 0xE2,
    nFTRemixMarthStatusNSPG_2_Low = 0xE3,
    nFTRemixMarthStatusNSPG_3_High = 0xE4,
    nFTRemixMarthStatusNSPG_3_Mid = 0xE5,
    nFTRemixMarthStatusNSPG_3_Low = 0xE6,
    nFTRemixMarthStatusNSPA_1 = 0xE7,
    nFTRemixMarthStatusNSPA_2_High = 0xE8,
    nFTRemixMarthStatusNSPA_2_Mid = 0xE9,
    nFTRemixMarthStatusNSPA_2_Low = 0xEA,
    nFTRemixMarthStatusNSPA_3_High = 0xEB,
    nFTRemixMarthStatusNSPA_3_Mid = 0xEC,
    nFTRemixMarthStatusNSPA_3_Low = 0xED,
    nFTRemixMarthStatusDSPG = 0xEF,
    nFTRemixMarthStatusDSPG_Attack = 0xF0,
    nFTRemixMarthStatusDSPGA = 0xF1,
    nFTRemixMarthStatusDSPGA_Attack = 0xF2
};

/* Kirby action ids: 0x12F (Kirby action array 0x67C / 0x14 + 0xDC) plus the index of
 * each MARTH_* entry among Kirby's add_new_action calls in Kirby.asm. */
enum
{
    nFTRemixKirbyStatusMarthNSPG_1 = 0x150,
    nFTRemixKirbyStatusMarthNSPG_2_Mid = 0x152,
    nFTRemixKirbyStatusMarthNSPG_3_Mid = 0x155,
    nFTRemixKirbyStatusMarthNSPA_1 = 0x157,
    nFTRemixKirbyStatusMarthNSPA_2_Mid = 0x159,
    nFTRemixKirbyStatusMarthNSPA_3_Mid = 0x15C
};

extern void ftCaptainSpecialHiProcInterrupt(GObj *fighter_gobj);
extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);

s32 ftRemixMarthDSPCounterDamage(FTStruct *fp, s32 damage);

/* Byte-offset access for Remix words the decomp union does not name. */
static s32 *ftRemixMarthWordAt(FTStruct *fp, u32 offset)
{
    return ftRemixFighterWord(fp, offset);
}

static s8 ftRemixMarthKirbyHat(FTStruct *fp)
{
    return *ftRemixFighterKirbyHat(fp);
}

static sb32 ftRemixMarthIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/* Marth's status id, or Kirby's copy of it when Kirby or J Kirby uses the move. */
static s32 ftRemixMarthStatusFor(FTStruct *fp, s32 marth_status, s32 kirby_status)
{
    return ftRemixMarthIsKirby(fp) ? kirby_status : marth_status;
}

static void ftRemixMarthClearTempVars(FTStruct *fp)
{
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* Clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall; the asm does this
 * with a mask of 0x07 on the byte at 0x18D. */
static void ftRemixMarthClearFastFallFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* Starts `status_id` from its first frame and plays its animation events. */
static void ftRemixMarthSetStatusWithAnim(GObj *fighter_gobj, s32 status_id)
{
    ftMainSetStatus(fighter_gobj, status_id, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* Sets the animation to idle or fall once the animation has ended. */
static void ftRemixMarthAnimEndToIdle(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* MarthUSP.air_initial_ */
static void ftRemixMarthUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarthSetStatusWithAnim(fighter_gobj, nFTRemixMarthStatusUSPA);
    ftRemixMarthClearTempVars(fp);
    fp->motion_vars.flags.flag2 = MARTH_USP_BEGIN;
    ftRemixMarthClearFastFallFlags(fp);
    fp->physics.vel_air.y = fp->attr->gravity;
}

/* MarthUSP.ground_initial_ */
static void ftRemixMarthUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarthSetStatusWithAnim(fighter_gobj, nFTRemixMarthStatusUSPG);
    ftRemixMarthClearTempVars(fp);
    fp->motion_vars.flags.flag2 = MARTH_USP_BEGIN;
}

/* MarthUSP.change_direction_: the turn routine is Captain Falcon's special hi interrupt. */
static void ftRemixMarthUSPChangeDirection(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == MARTH_USP_CHANGE_DIRECTION)
    {
        ftCaptainSpecialHiProcInterrupt(fighter_gobj);
    }
}

/* MarthUSP.main_: lands the special fall once the animation has ended. */
static void ftRemixMarthUSPProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame > 0.0F)
    {
        return;
    }
    ftCommonFallSpecialSetStatus(fighter_gobj, 1.0F, FALSE, TRUE, FALSE, MARTH_USP_LANDING_LAG, FALSE);
}

/* MarthUSP.air_control_: horizontal control during the END phase. */
static void ftRemixMarthUSPAirControl(FTStruct *fp)
{
    ftPhysicsClampAirVelXStickRange(fp, 8, MARTH_USP_END_AIR_ACCELERATION, MARTH_USP_END_AIR_SPEED);
}

/* Copy of the vanilla aerial physics (0x800D8E50, 0x800D8DA0, 0x800D8FA8, 0x800D9074)
 * with Marth's air control. The asm re-calls ftPhysicsCheckClampAirVelXDecMax through
 * t8 on the non-END path; it returns FALSE there and changes nothing, so it is omitted. */
static void ftRemixMarthUSPAirDrift(FTStruct *fp)
{
    FTAttributes *attr = fp->attr;

    if (fp->is_fastfall)
    {
        ftPhysicsApplyFastFall(fp, attr);
    }
    else ftPhysicsApplyGravityDefault(fp, attr);

    if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) != FALSE)
    {
        return;
    }
    if (fp->motion_vars.flags.flag2 == MARTH_USP_END)
    {
        ftRemixMarthUSPAirControl(fp);
    }
    ftPhysicsApplyAirVelXFriction(fp, attr);
}

/* MarthUSP.physics_, aerial BEGIN phase: slows x and freezes y while not in USPG. */
static void ftRemixMarthUSPBeginAir(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag2 != MARTH_USP_BEGIN)
    {
        return;
    }
    if (fp->status_id == nFTRemixMarthStatusUSPG)
    {
        return;
    }
    fp->physics.vel_air.x *= MARTH_USP_BEGIN_X_DECAY;
    fp->physics.vel_air.y = 0.0F;
}

/* Launch y speed for BEGIN_MOVE: Roy and the grounded move each have their own value. */
static f32 ftRemixMarthUSPLaunchYSpeed(FTStruct *fp)
{
    sb32 is_ground = (fp->status_id == nFTRemixMarthStatusUSPG);

    if (fp->fkind == REMIX_ID_ROY)
    {
        return is_ground ? MARTH_USP_GROUND_Y_SPEED_ROY : MARTH_USP_AIR_Y_SPEED_ROY;
    }
    return is_ground ? MARTH_USP_GROUND_Y_SPEED : MARTH_USP_AIR_Y_SPEED;
}

/* MarthUSP.physics_, BEGIN_MOVE phase: sets the launch velocity from the stick. */
static void ftRemixMarthUSPBeginMove(FTStruct *fp)
{
    f32 direction = (f32)fp->lr;
    f32 stick_dir = (f32)fp->input.pl.stick_range.x * direction;
    f32 y_speed = ftRemixMarthUSPLaunchYSpeed(fp);
    f32 x_speed = MARTH_USP_X_SPEED;

    if (stick_dir >= MARTH_USP_X_SPEED)
    {
        if (fp->fkind == REMIX_ID_ROY)
        {
            x_speed = stick_dir * MARTH_USP_STICK_X_FACTOR_ROY;
        }
        else x_speed = stick_dir * MARTH_USP_STICK_X_FACTOR;
        y_speed -= x_speed * MARTH_USP_STICK_Y_FACTOR;
    }
    fp->physics.vel_air.x = (x_speed + MARTH_USP_X_SPEED) * direction;
    fp->physics.vel_air.y = y_speed;
    fp->motion_vars.flags.flag2 = MARTH_USP_MOVE;
    fp->jumps_used = (u8)fp->attr->jumps_max;
}

/* MarthUSP.physics_, END_MOVE phase: decays the launch velocity and ends the move. */
static void ftRemixMarthUSPEndMove(FTStruct *fp)
{
    f32 direction = (f32)fp->lr;
    f32 y_decay = (fp->fkind == REMIX_ID_ROY) ? MARTH_USP_END_Y_DECAY_ROY : MARTH_USP_END_Y_DECAY;

    fp->physics.vel_air.x = (fp->physics.vel_air.x * MARTH_USP_END_X_DECAY) + (MARTH_USP_X_SPEED * direction);
    fp->physics.vel_air.y *= y_decay;
    fp->motion_vars.flags.flag2 = MARTH_USP_END;
}

/* MarthUSP.physics_ */
static void ftRemixMarthUSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsGround)
    {
        ftPhysicsApplyGroundVelFriction(fighter_gobj);
        return;
    }
    ftRemixMarthUSPAirDrift(fp);
    ftRemixMarthUSPBeginAir(fp);
    if (fp->motion_vars.flags.flag2 == MARTH_USP_BEGIN_MOVE)
    {
        ftRemixMarthUSPBeginMove(fp);
    }
    if (fp->motion_vars.flags.flag2 == MARTH_USP_END_MOVE)
    {
        ftRemixMarthUSPEndMove(fp);
    }
}

/* MarthUSP.collision_: ftMarioSpecialHiProcMap with Marth's landing lag. */
static void ftRemixMarthUSPProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsAir)
    {
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
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, MARTH_USP_LANDING_LAG);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* MarthNSP.ground_shared_initial_, used by both grounded and aerial stages: Kirby with
 * a Roy hat adds 101 to the status id. */
static void ftRemixMarthNSPSharedInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 hat_status_id = status_id;

    if (ftRemixMarthIsKirby(fp) && (ftRemixMarthKirbyHat(fp) == MARTH_KIRBY_ROY_HAT))
    {
        hat_status_id += MARTH_KIRBY_ROY_NSP_OFFSET;
    }
    ftRemixMarthSetStatusWithAnim(fighter_gobj, hat_status_id);
    ftRemixMarthClearTempVars(fp);
}

/* MarthNSP.air_shared_initial_, y velocity: NSPA_1 boosts only on first use and
 * sets the pseudo-jump flag; every other stage uses NSP_Y_SPEED_SECOND. */
static f32 ftRemixMarthNSPAirYSpeed(FTStruct *fp, s32 status_id)
{
    s32 *pseudo_jump_flag;

    if (status_id != ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPA_1, nFTRemixKirbyStatusMarthNSPA_1))
    {
        return MARTH_NSP_Y_SPEED_SECOND;
    }
    pseudo_jump_flag = ftRemixMarthWordAt(fp, ftRemixMarthIsKirby(fp) ? MARTH_OFF_PSEUDO_JUMP_KIRBY : MARTH_OFF_PSEUDO_JUMP_MARTH);
    if (*pseudo_jump_flag != FALSE)
    {
        return MARTH_NSP_Y_SPEED_STALE;
    }
    *pseudo_jump_flag = TRUE;
    return MARTH_NSP_Y_SPEED;
}

/* MarthNSP.air_shared_initial_. `status_id` is the stage id before the Kirby hat adjustment. */
static void ftRemixMarthNSPAirSharedInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 y_speed = ftRemixMarthNSPAirYSpeed(fp, status_id);

    ftRemixMarthNSPSharedInitial(fighter_gobj, status_id);
    ftRemixMarthClearFastFallFlags(fp);
    fp->physics.vel_air.x *= MARTH_AIR_X_DECAY;
    fp->physics.vel_air.y = y_speed;
}

/* MarthNSP.ground_1_initial_ */
static void ftRemixMarthNSPGround1Initial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE) = 0;
    ftRemixMarthNSPSharedInitial(fighter_gobj, ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPG_1, nFTRemixKirbyStatusMarthNSPG_1));
}

/* Stick-selected variant of a stage: stick_y >= 40 gives High, stick_y < -39 gives Low. */
static s32 ftRemixMarthNSPStageStatus(FTStruct *fp, s32 mid_status_id)
{
    s32 stick_y = fp->input.pl.stick_range.y;

    if (stick_y >= 40)
    {
        return mid_status_id - 1;
    }
    if (stick_y < -39)
    {
        return mid_status_id + 1;
    }
    return mid_status_id;
}

/* MarthNSP.ground_2_initial_ */
static void ftRemixMarthNSPGround2Initial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 mid_status_id = ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPG_2_Mid, nFTRemixKirbyStatusMarthNSPG_2_Mid);

    *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE) = 1;
    ftRemixMarthNSPSharedInitial(fighter_gobj, ftRemixMarthNSPStageStatus(fp, mid_status_id));
}

/* MarthNSP.ground_3_initial_ */
static void ftRemixMarthNSPGround3Initial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 mid_status_id = ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPG_3_Mid, nFTRemixKirbyStatusMarthNSPG_3_Mid);

    *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE) = 2;
    ftRemixMarthNSPSharedInitial(fighter_gobj, ftRemixMarthNSPStageStatus(fp, mid_status_id));
}

/* MarthNSP.air_1_initial_ */
static void ftRemixMarthNSPAir1Initial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE) = 0;
    ftRemixMarthNSPAirSharedInitial(fighter_gobj, ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPA_1, nFTRemixKirbyStatusMarthNSPA_1));
}

/* MarthNSP.air_2_initial_ */
static void ftRemixMarthNSPAir2Initial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 mid_status_id = ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPA_2_Mid, nFTRemixKirbyStatusMarthNSPA_2_Mid);

    *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE) = 1;
    ftRemixMarthNSPAirSharedInitial(fighter_gobj, ftRemixMarthNSPStageStatus(fp, mid_status_id));
}

/* MarthNSP.air_3_initial_ */
static void ftRemixMarthNSPAir3Initial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 mid_status_id = ftRemixMarthStatusFor(fp, nFTRemixMarthStatusNSPA_3_Mid, nFTRemixKirbyStatusMarthNSPA_3_Mid);

    *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE) = 2;
    ftRemixMarthNSPAirSharedInitial(fighter_gobj, ftRemixMarthNSPStageStatus(fp, mid_status_id));
}

/* True when the stage window is open and B was tapped inside it. */
static sb32 ftRemixMarthNSPIsStageInput(FTStruct *fp)
{
    if (fp->motion_vars.flags.flag1 == 0)
    {
        return FALSE;
    }
    return (fp->input.pl.button_tap & B_BUTTON) != 0;
}

/* Shared main for the grounded and aerial neutral special stages. */
static void ftRemixMarthNSPProcUpdate(GObj *fighter_gobj, void (*start_stage_2)(GObj *), void (*start_stage_3)(GObj *))
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 stage = *ftRemixMarthWordAt(fp, MARTH_OFF_NSP_STAGE);

    if (ftRemixMarthNSPIsStageInput(fp))
    {
        if (stage == 0)
        {
            start_stage_2(fighter_gobj);
            return;
        }
        if (stage == 1)
        {
            start_stage_3(fighter_gobj);
            return;
        }
    }
    ftRemixMarthAnimEndToIdle(fighter_gobj);
}

/* MarthNSP.ground_main_ */
static void ftRemixMarthNSPGroundProcUpdate(GObj *fighter_gobj)
{
    ftRemixMarthNSPProcUpdate(fighter_gobj, ftRemixMarthNSPGround2Initial, ftRemixMarthNSPGround3Initial);
}

/* MarthNSP.air_main_ */
static void ftRemixMarthNSPAirProcUpdate(GObj *fighter_gobj)
{
    ftRemixMarthNSPProcUpdate(fighter_gobj, ftRemixMarthNSPAir2Initial, ftRemixMarthNSPAir3Initial);
}

/* MarthNSP.ground_to_air_: switches to the aerial status (id + 7) and keeps the momentum. */
static void ftRemixMarthNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id + 7, fighter_gobj->anim_frame, 1.0F, MARTH_TRANSITION_FLAGS_NSP);
    ftPhysicsClampAirVelXMax(fp);
}

/* MarthNSP.air_to_ground_: switches to the grounded status (id - 7). */
static void ftRemixMarthNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id - 7, fighter_gobj->anim_frame, 1.0F, MARTH_TRANSITION_FLAGS_NSP);
}

/* MarthNSP.ground_collision_ */
static void ftRemixMarthNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMarthNSPGroundToAir);
}

/* MarthNSP.kirby_ground_collision_: Kirby's grounded neutral special slides off a ledge (0x800DDDDC, not 0x800DDE84). */
static void ftRemixMarthNSPKirbyGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixMarthNSPGroundToAir);
}

/* MarthNSP.air_collision_ */
static void ftRemixMarthNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMarthNSPAirToGround);
}

/* MarthDSP.ground_initial_ and air_initial_ share this start: status, temp vars, no counter hit. */
static void ftRemixMarthDSPInitial(GObj *fighter_gobj, s32 status_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarthSetStatusWithAnim(fighter_gobj, status_id);
    ftRemixMarthClearTempVars(fp);
    *ftRemixMarthWordAt(fp, MARTH_OFF_DSP_HIT_DETECTION) = FALSE;
}

/* MarthDSP.ground_initial_ */
static void ftRemixMarthDSPGroundInitial(GObj *fighter_gobj)
{
    ftRemixMarthDSPInitial(fighter_gobj, nFTRemixMarthStatusDSPG);
}

/* MarthDSP.air_initial_ */
static void ftRemixMarthDSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarthDSPInitial(fighter_gobj, nFTRemixMarthStatusDSPGA);
    fp->physics.vel_air.y = 0.0F;
    fp->physics.vel_air.x *= MARTH_DSP_AIR_X_DECAY;
}

/* MarthDSP.ground_attack_initial_ and air_attack_initial_: the counter announcement
 * (PokemonAnnouncer.counter_announcement_) is not ported. */
static void ftRemixMarthDSPStartAttack(GObj *fighter_gobj, s32 status_id)
{
    /* REMIX-TODO(PokemonAnnouncer): counter_announcement_ voice line is skipped */
    ftRemixMarthSetStatusWithAnim(fighter_gobj, status_id);
}

/* Remix counter patch for ftParamUpdateDamage (ftparam.c). While DSP is active with
 * flag0 set, Marth takes no damage and the counter hit flag is raised. */
s32 ftRemixMarthDSPCounterDamage(FTStruct *fp, s32 damage)
{
    s32 status_id = fp->status_id;

    if (fp->fkind != REMIX_ID_MARTH)
    {
        return damage;
    }
    if ((status_id != nFTRemixMarthStatusDSPG) && (status_id != nFTRemixMarthStatusDSPGA))
    {
        return damage;
    }
    if (fp->motion_vars.flags.flag0 == 0)
    {
        return damage;
    }
    *ftRemixMarthWordAt(fp, MARTH_OFF_DSP_HIT_DETECTION) = TRUE;
    return 0;
}

/* Armor is full while flag0 is set and empty otherwise. */
static void ftRemixMarthDSPSetArmor(FTStruct *fp)
{
    fp->knockback_resist_status = (fp->motion_vars.flags.flag0 != 0) ? MARTH_DSP_INVINCIBLE_ARMOR : 0.0F;
}

/* Turns Marth to the hit direction, then starts the counter attack for her current move. */
static void ftRemixMarthDSPCounterAttack(GObj *fighter_gobj, FTStruct *fp)
{
    fp->lr = fp->damage_lr;
    fp->joints[nFTPartsJointTopN]->rotate.vec.f.y = MARTH_DSP_FACING_ROTATION * (f32)fp->damage_lr;

    if (fp->ga != nMPKineticsGround)
    {
        ftRemixMarthDSPStartAttack(fighter_gobj, nFTRemixMarthStatusDSPGA_Attack);
    }
    else ftRemixMarthDSPStartAttack(fighter_gobj, nFTRemixMarthStatusDSPG_Attack);
}

/* MarthDSP.main_ */
static void ftRemixMarthDSPProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixMarthDSPSetArmor(fp);
    if (*ftRemixMarthWordAt(fp, MARTH_OFF_DSP_HIT_DETECTION) != FALSE)
    {
        ftRemixMarthDSPCounterAttack(fighter_gobj, fp);
        return;
    }
    ftRemixMarthAnimEndToIdle(fighter_gobj);
}

/* MarthDSP.ground_to_air_ */
static void ftRemixMarthDSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id + 2, fighter_gobj->anim_frame, 1.0F, MARTH_TRANSITION_FLAGS_DSP);
    ftPhysicsClampAirVelXMax(fp);
}

/* MarthDSP.air_to_ground_ */
static void ftRemixMarthDSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftMainSetStatus(fighter_gobj, fp->status_id - 2, fighter_gobj->anim_frame, 1.0F, MARTH_TRANSITION_FLAGS_DSP);
}

/* MarthDSP.ground_collision_ */
static void ftRemixMarthDSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnEdge(fighter_gobj, ftRemixMarthDSPGroundToAir);
}

/* MarthDSP.air_collision_ */
static void ftRemixMarthDSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixMarthDSPAirToGround);
}

/* MarthDSP.air_physics_: normal aerial friction plus a fixed 1.5 gravity boost. */
static void ftRemixMarthDSPProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftPhysicsApplyAirVelFriction(fighter_gobj);
    fp->physics.vel_air.y += MARTH_DSP_AIR_GRAVITY_BOOST;
}

/* Remix address -> port routine for every Marth routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixMarthProcs[] =
{
    { REMIX_SYM_MarthUSP__air_initial_, ftRemixMarthUSPAirInitial },
    { REMIX_SYM_MarthUSP__ground_initial_, ftRemixMarthUSPGroundInitial },
    { REMIX_SYM_MarthUSP__main_, ftRemixMarthUSPProcUpdate },
    { REMIX_SYM_MarthUSP__change_direction_, ftRemixMarthUSPChangeDirection },
    { REMIX_SYM_MarthUSP__physics_, ftRemixMarthUSPProcPhysics },
    { REMIX_SYM_MarthUSP__collision_, ftRemixMarthUSPProcMap },
    { REMIX_SYM_MarthNSP__ground_1_initial_, ftRemixMarthNSPGround1Initial },
    { REMIX_SYM_MarthNSP__air_1_initial_, ftRemixMarthNSPAir1Initial },
    { REMIX_SYM_MarthNSP__ground_main_, ftRemixMarthNSPGroundProcUpdate },
    { REMIX_SYM_MarthNSP__air_main_, ftRemixMarthNSPAirProcUpdate },
    { REMIX_SYM_MarthNSP__ground_collision_, ftRemixMarthNSPGroundProcMap },
    { REMIX_SYM_MarthNSP__kirby_ground_collision_, ftRemixMarthNSPKirbyGroundProcMap },
    { REMIX_SYM_MarthNSP__air_collision_, ftRemixMarthNSPAirProcMap },
    { REMIX_SYM_MarthDSP__ground_initial_, ftRemixMarthDSPGroundInitial },
    { REMIX_SYM_MarthDSP__air_initial_, ftRemixMarthDSPAirInitial },
    { REMIX_SYM_MarthDSP__main_, ftRemixMarthDSPProcUpdate },
    { REMIX_SYM_MarthDSP__air_physics_, ftRemixMarthDSPProcPhysics },
    { REMIX_SYM_MarthDSP__ground_collision_, ftRemixMarthDSPGroundProcMap },
    { REMIX_SYM_MarthDSP__air_collision_, ftRemixMarthDSPAirProcMap }
};

const s32 dFTRemixMarthProcsNum = ARRAY_COUNT(dFTRemixMarthProcs);

#endif
