/**
 * ftremixcrashnsp.c - C ports of Smash Remix Crash neutral special routines
 * (.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm, scope CrashNSP).
 *
 * Kirby and J Kirby copy this move with their own action ids (Kirby.asm
 * add_new_action_params CRASH_NSP_*). The shield-hit routine is installed on
 * the fighter through proc_shield (N64 0x09F4), so it is a C function pointer.
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <mp/mpcommon.h>

enum
{
    nFTRemixCrashStatusNSPG = 0xDF,
    nFTRemixCrashStatusNSPA = 0xE0,
    nFTRemixCrashStatusNSPGBlocked = 0xE1,
    nFTRemixCrashStatusNSPABlocked = 0xE2
};

/* Kirby copies: 0xDC + 83 (KIRBY parent size 0x67C / 0x14) + index 127..130 of the KIRBY add_new_action calls
 * (Kirby.asm lines 307-310). Derived, not read from remix.z64. See the notes file. */
enum
{
    nFTRemixCrashKirbyStatusNSPG = 0x1AE,
    nFTRemixCrashKirbyStatusNSPA = 0x1AF,
    nFTRemixCrashKirbyStatusNSPGBlocked = 0x1B0,
    nFTRemixCrashKirbyStatusNSPABlocked = 0x1B1
};

/* Flags of the transition routines (asm argument 4 = 0x0807), named by the asm comment:
 * 3C FGM = LOOPSFX, attached gfx = EFFECT, gfx routines = COLANIM, hitboxes = HIT. */
#define CRASH_NSP_CONTINUE_FLAGS (FTSTATUS_PRESERVE_LOOPSFX | FTSTATUS_PRESERVE_EFFECT | FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_HIT)

#define CRASH_NSP_INITIAL_GROUND_VEL_SCALE 1.5F   /* 0x3FC00000 */
#define CRASH_NSP_INITIAL_AIR_VEL_SCALE 1.25F     /* 0x3FA00000 */
#define CRASH_NSP_SHIELD_AIR_VEL_X_SCALE 0.25F    /* 0x3E800000 */
#define CRASH_NSP_SHIELD_VEL_Y_SCALE 0.5F         /* 0x3F000000 */
#define CRASH_NSP_SHIELD_GROUND_VEL_X_SCALE 0.5F  /* 0x3F000000 */
#define CRASH_NSP_MIN_SPEED 2.0F                  /* 0x40000000 */
#define CRASH_NSP_MAX_SPEED 78.0F                 /* 0x429C0000 */
#define CRASH_NSP_ACCELERATION 2.0F               /* 0x40000000 */
#define CRASH_NSP_NEUTRAL_ACCELERATION 1.25F      /* 0x3FA00000: used by the grounded routine when the stick is neutral */
#define CRASH_NSP_END_ACCELERATION 4.5F           /* 0x40900000 */
#define CRASH_NSP_G_SPEED_MULTIPLIER 0.625F       /* 0x3F200000 */
#define CRASH_NSP_A_SPEED_MULTIPLIER 0.5F         /* 0x3F000000 */
#define CRASH_NSP_STICK_NEUTRAL_RANGE 11          /* |stick_x| below this counts as neutral */
#define CRASH_NSP_GRAVITY 2.625F                  /* 0x40280000 */
#define CRASH_NSP_KIRBY_GRAVITY 2.109375F         /* 0x40070000 */
#define CRASH_NSP_FALL_SPEED 48.0F                /* 0x42400000 */
#define CRASH_NSP_JUMP_SPEED 72.0F                /* 0x42900000 */
#define CRASH_NSP_KIRBY_JUMP_SPEED 64.0F          /* 0x42800000 */
#define CRASH_NSP_SHIELD_HITBOX_NUM 3             /* Character.get_hitbox_collision_flags_ loops over three hitboxes */
#define CRASH_NSP_JUMP_SMOKE_LR 1

/* Per-fighter values that Remix keeps in status vars 0x0B18..0x0B24 while the blocked
 * routines run, which have no typed field in FTStatusVars. */
typedef struct FTRemixCrashNSPShieldVars
{
    f32 vel_air_x;
    f32 vel_air_y;
    f32 vel_ground_x;
    sb32 is_hitlag_over;
} FTRemixCrashNSPShieldVars;

static FTRemixCrashNSPShieldVars sFTRemixCrashNSPShieldVars[MAXCONTROLLERS];

static void ftRemixCrashNSPShieldHit(GObj *fighter_gobj);

/* Shared switch between Crash and Kirby: the fighter's fkind picks the status id. */
static void ftRemixCrashNSPChangeStatus(GObj *fighter_gobj, s32 crash_status, s32 kirby_status, f32 frame, u32 flags)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 status_id = (fp->fkind == REMIX_ID_CRASH) ? crash_status : kirby_status;

    ftMainSetStatus(fighter_gobj, status_id, frame, 1.0F, flags);
}

/* REMIX-TODO(GFX): create_spin_gfx_ (CrashSpecial.asm:862-887) and setup_spin_gfx_ (119-155) are not ported.
 * The spin GFX is an attached effect with the Crash/Kirby spin file, and its colour tables are per costume.
 * Without it the spin is invisible, but the moveset and physics still run. */
static void ftRemixCrashNSPCreateSpinGFX(GObj *fighter_gobj)
{
}

/* REMIX-TODO(Item): spin_detect_item_ (CrashSpecial.asm:264-329) and the item fling in ground_main_ and air_main_
 * (asm 371-440, item_fling_ at 891-907 with Remix's flung_item_state_table) are not ported. Crash does not pick up
 * items with the spin. */
static void ftRemixCrashNSPSpinItem(void)
{
}

/* Character.get_hitbox_collision_flags_ (Character.asm:3322), reduced to the shield test that shield_hit_routine_
 * needs. The asm ORs flag bytes of the active hitboxes. The flag is mapped to GMHitFlags.is_interact_shield,
 * which itprocess.c sets on attack records that hit a shield. See the notes file for this uncertainty. */
static sb32 ftRemixCrashNSPIsShieldCollided(FTStruct *fp)
{
    s32 hitbox_index;
    s32 record_index;

    for (hitbox_index = 0; hitbox_index < CRASH_NSP_SHIELD_HITBOX_NUM; hitbox_index++)
    {
        FTAttackColl *attack_coll = &fp->attack_colls[hitbox_index];

        if (attack_coll->attack_state == 0)
        {
            continue;
        }
        for (record_index = 0; record_index < GMATTACKREC_NUM_MAX; record_index++)
        {
            if (attack_coll->attack_records[record_index].victim_flags.is_interact_shield)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* The asm treats |stick_x| < 11 as neutral. */
static sb32 ftRemixCrashNSPIsStickNeutral(FTStruct *fp)
{
    s32 stick_x = fp->input.pl.stick_range.x;

    return (stick_x > -CRASH_NSP_STICK_NEUTRAL_RANGE) && (stick_x < CRASH_NSP_STICK_NEUTRAL_RANGE);
}

/* _apply_movement and _set_velocity of ground_physics_ and air_physics_: moves air x velocity toward
 * target_x by accel once the gap is at least 2.0, then clamps it to MAX_SPEED. */
static void ftRemixCrashNSPApplyVelX(FTStruct *fp, f32 target_x, f32 accel)
{
    f32 diff = target_x - fp->physics.vel_air.x;

    if ((diff >= CRASH_NSP_MIN_SPEED) || (diff <= -CRASH_NSP_MIN_SPEED))
    {
        if (diff >= 0.0F)
        {
            fp->physics.vel_air.x += accel;
        }
        else
        {
            fp->physics.vel_air.x -= accel;
        }
    }
    if (fp->physics.vel_air.x > CRASH_NSP_MAX_SPEED)
    {
        fp->physics.vel_air.x = CRASH_NSP_MAX_SPEED;
    }
    else if (fp->physics.vel_air.x < -CRASH_NSP_MAX_SPEED)
    {
        fp->physics.vel_air.x = -CRASH_NSP_MAX_SPEED;
    }
}

/* ground_to_air_: switches a grounded NSP to its aerial status and keeps the frame and flags. */
static void ftRemixCrashNSPGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPA, nFTRemixCrashKirbyStatusNSPA,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
}

/* ground_to_air_jump_: the grounded NSP jump. Kirby gets the lower jump speed. */
static void ftRemixCrashNSPGroundToAirJump(GObj *fighter_gobj)
{
    FTStruct *fp;
    f32 jump_speed;

    ftRemixCrashNSPGroundToAir(fighter_gobj);
    fp = ftGetStruct(fighter_gobj);
    /* asm 737-742 compares `at` (the float bits) instead of the reloaded id for J Kirby, so J Kirby never
     * matches and keeps CRASH_NSP_JUMP_SPEED. Kept as is. */
    jump_speed = (fp->fkind == nFTKindKirby) ? CRASH_NSP_KIRBY_JUMP_SPEED : CRASH_NSP_JUMP_SPEED;
    fp->physics.vel_air.y = jump_speed;
    efManagerDustHeavyDoubleMakeEffect(fp->coll_data.p_translate, CRASH_NSP_JUMP_SMOKE_LR, 1.0F);
}

/* air_to_ground_: switches an aerial NSP to its grounded status. */
static void ftRemixCrashNSPAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPG, nFTRemixCrashKirbyStatusNSPG,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
}

/* blocked_ground_to_air_: the blocked grounded NSP becomes the blocked aerial one. */
static void ftRemixCrashNSPBlockedGroundToAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterAir(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPABlocked, nFTRemixCrashKirbyStatusNSPABlocked,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
}

/* blocked_air_to_ground_: the blocked aerial NSP becomes the blocked grounded one. */
static void ftRemixCrashNSPBlockedAirToGround(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    mpCommonSetFighterGround(fp);
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPGBlocked, nFTRemixCrashKirbyStatusNSPGBlocked,
                                fighter_gobj->anim_frame, CRASH_NSP_CONTINUE_FLAGS);
}

/* blocked_ground_initial_ */
static void ftRemixCrashNSPBlockedGroundInitial(GObj *fighter_gobj)
{
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPGBlocked, nFTRemixCrashKirbyStatusNSPGBlocked,
                                0.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* blocked_air_initial_ */
static void ftRemixCrashNSPBlockedAirInitial(GObj *fighter_gobj)
{
    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPABlocked, nFTRemixCrashKirbyStatusNSPABlocked,
                                0.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
}

/* shield_hit_routine_: installed as proc_shield. Runs when the NSP hits a shield and turns the
 * move into its blocked form, with the velocities saved for blocked_main_ to restore. */
static void ftRemixCrashNSPShieldHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashNSPShieldVars *shield_vars = &sFTRemixCrashNSPShieldVars[fp->player];

    if (!ftRemixCrashNSPIsShieldCollided(fp))
    {
        return;
    }
    fp->motion_vars.flags.flag1 = 0;
    shield_vars->vel_air_x = fp->physics.vel_air.x * CRASH_NSP_SHIELD_AIR_VEL_X_SCALE;
    shield_vars->vel_air_y = fp->physics.vel_air.y * CRASH_NSP_SHIELD_VEL_Y_SCALE;
    shield_vars->vel_ground_x = fp->physics.vel_ground.x * CRASH_NSP_SHIELD_GROUND_VEL_X_SCALE;
    shield_vars->is_hitlag_over = FALSE;
    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_ground.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;
    if (fp->ga != nMPKineticsGround)
    {
        fp->is_absorb = FALSE;
        fp->absorb_lr = 0;
        fp->is_goto_attack100 = FALSE;
        fp->is_fastfall = FALSE;
        ftRemixCrashNSPBlockedAirInitial(fighter_gobj);
        return;
    }
    ftRemixCrashNSPBlockedGroundInitial(fighter_gobj);
}

/* Shield hitlag hook for Crash and Kirby (the port_fighter_shield_hitlag_skip callback, see fighter_registry.h).
 * Returns TRUE when the NSP is in its blocked form, so the hitlag freeze is skipped (asm `or t6, r0, r0`).
 * Kirby's hat check (asm 941-944) is not repeated: the Kirby blocked ids are only set by the Crash copy. */
int ftRemixCrashNSPShieldHitlagSkip(GObj *fighter_gobj, int status_id)
{
    switch (status_id)
    {
        case nFTRemixCrashStatusNSPGBlocked:
        case nFTRemixCrashStatusNSPABlocked:
        case nFTRemixCrashKirbyStatusNSPGBlocked:
        case nFTRemixCrashKirbyStatusNSPABlocked:
            return TRUE;
        default:
            return FALSE;
    }
}

/* ground_initial_ */
static void ftRemixCrashNSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPG, nFTRemixCrashKirbyStatusNSPG, 0.0F,
                                FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixCrashNSPCreateSpinGFX(fighter_gobj);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
    fp->physics.vel_air.x *= CRASH_NSP_INITIAL_GROUND_VEL_SCALE;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
}

/* air_initial_ */
static void ftRemixCrashNSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixCrashNSPChangeStatus(fighter_gobj, nFTRemixCrashStatusNSPA, nFTRemixCrashKirbyStatusNSPA, 0.0F,
                                FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);
    ftRemixCrashNSPCreateSpinGFX(fighter_gobj);
    fp->proc_shield = ftRemixCrashNSPShieldHit;
    fp->physics.vel_air.x *= CRASH_NSP_INITIAL_AIR_VEL_SCALE;
    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 0;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* ground_main_ */
static void ftRemixCrashNSPGroundProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
        ftRemixCrashNSPSpinItem();
        return;
    }
    if ((fp->motion_vars.flags.flag1 == 0) && (ftCommonKneeBendGetInputTypeCommon(fp) != 0))
    {
        ftRemixCrashNSPGroundToAirJump(fighter_gobj);
        return;
    }
    ftRemixCrashNSPSpinItem();
}

/* air_main_ */
static void ftRemixCrashNSPAirProcUpdate(GObj *fighter_gobj)
{
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
    ftRemixCrashNSPSpinItem();
}

/* blocked_main_ */
static void ftRemixCrashNSPBlockedProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixCrashNSPShieldVars *shield_vars = &sFTRemixCrashNSPShieldVars[fp->player];

    if (fp->motion_vars.flags.flag1 != 0)
    {
        shield_vars->is_hitlag_over = TRUE;
        fp->physics.vel_air.x = shield_vars->vel_air_x;
        fp->physics.vel_air.y = shield_vars->vel_air_y;
        fp->physics.vel_ground.x = shield_vars->vel_ground_x;
        fp->motion_vars.flags.flag1 = 0;
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* ground_physics_ */
static void ftRemixCrashNSPGroundProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 target_x = 0.0F;
    f32 accel = CRASH_NSP_ACCELERATION;

    if (fp->motion_vars.flags.flag1 != 0)
    {
        accel = CRASH_NSP_END_ACCELERATION;
    }
    else if (ftRemixCrashNSPIsStickNeutral(fp))
    {
        if ((fp->physics.vel_air.x >= -CRASH_NSP_MIN_SPEED) && (fp->physics.vel_air.x <= CRASH_NSP_MIN_SPEED))
        {
            fp->physics.vel_air.x = 0.0F;
            return;
        }
        accel = CRASH_NSP_NEUTRAL_ACCELERATION;
    }
    else
    {
        target_x = (f32) fp->input.pl.stick_range.x * CRASH_NSP_G_SPEED_MULTIPLIER;
    }
    ftRemixCrashNSPApplyVelX(fp, target_x, accel);
    fp->physics.vel_ground.x = fp->physics.vel_air.x * (f32) fp->lr;
}

/* air_physics_ */
static void ftRemixCrashNSPAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 target_x = 0.0F;
    f32 gravity = CRASH_NSP_GRAVITY;

    if (ftRemixCrashNSPIsStickNeutral(fp))
    {
        if ((fp->physics.vel_air.x >= -CRASH_NSP_MIN_SPEED) && (fp->physics.vel_air.x <= CRASH_NSP_MIN_SPEED))
        {
            fp->physics.vel_air.x = 0.0F;
        }
        else
        {
            ftRemixCrashNSPApplyVelX(fp, target_x, CRASH_NSP_ACCELERATION);
        }
    }
    else
    {
        target_x = (f32) fp->input.pl.stick_range.x * CRASH_NSP_A_SPEED_MULTIPLIER;
        ftRemixCrashNSPApplyVelX(fp, target_x, CRASH_NSP_ACCELERATION);
    }
    if ((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        gravity = CRASH_NSP_KIRBY_GRAVITY;
    }
    ftPhysicsApplyGravityClampTVel(fp, gravity, CRASH_NSP_FALL_SPEED);
}

/* blocked_air_physics_: normal air physics once the hitlag is over. */
static void ftRemixCrashNSPBlockedAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (sFTRemixCrashNSPShieldVars[fp->player].is_hitlag_over)
    {
        ftPhysicsApplyAirVelFriction(fighter_gobj);
    }
}

/* ground_collision_ */
static void ftRemixCrashNSPGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixCrashNSPGroundToAir);
}

/* air_collision_ */
static void ftRemixCrashNSPAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixCrashNSPAirToGround);
}

/* blocked_ground_collision_ */
static void ftRemixCrashNSPBlockedGroundProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterOnFloor(fighter_gobj, ftRemixCrashNSPBlockedGroundToAir);
}

/* blocked_air_collision_ */
static void ftRemixCrashNSPBlockedAirProcMap(GObj *fighter_gobj)
{
    mpCommonProcFighterLanding(fighter_gobj, ftRemixCrashNSPBlockedAirToGround);
}

/* Remix address -> port routine for every CrashNSP routine its action array or special tables reference. */
const FTRemixProc dFTRemixCrashNSPProcs[] =
{
    { REMIX_SYM_CrashNSP__air_collision_, ftRemixCrashNSPAirProcMap },
    { REMIX_SYM_CrashNSP__air_initial_, ftRemixCrashNSPAirInitial },
    { REMIX_SYM_CrashNSP__air_main_, ftRemixCrashNSPAirProcUpdate },
    { REMIX_SYM_CrashNSP__air_physics_, ftRemixCrashNSPAirProcPhysics },
    { REMIX_SYM_CrashNSP__blocked_air_collision_, ftRemixCrashNSPBlockedAirProcMap },
    { REMIX_SYM_CrashNSP__blocked_air_physics_, ftRemixCrashNSPBlockedAirProcPhysics },
    { REMIX_SYM_CrashNSP__blocked_ground_collision_, ftRemixCrashNSPBlockedGroundProcMap },
    { REMIX_SYM_CrashNSP__blocked_main_, ftRemixCrashNSPBlockedProcUpdate },
    { REMIX_SYM_CrashNSP__ground_collision_, ftRemixCrashNSPGroundProcMap },
    { REMIX_SYM_CrashNSP__ground_initial_, ftRemixCrashNSPGroundInitial },
    { REMIX_SYM_CrashNSP__ground_main_, ftRemixCrashNSPGroundProcUpdate },
    { REMIX_SYM_CrashNSP__ground_physics_, ftRemixCrashNSPGroundProcPhysics }
};

const s32 dFTRemixCrashNSPProcsNum = ARRAY_COUNT(dFTRemixCrashNSPProcs);

#endif
