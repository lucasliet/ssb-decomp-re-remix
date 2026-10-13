/**
 * ftremixfalco.c - C ports of Smash Remix Falco routines
 * (.toolchain/pkgs/smashremix-src/src/Falco/Phantasm.asm).
 *
 * Phantasm's motion scripts drive it through temp variable 3
 * (motion_vars.flags.flag2, set by command 5C0000XX); Remix's set_variables_
 * patch arms it with PHANTASM_INITIAL_SETUP (ftFoxSpecialNInitStatusVars).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ft/ftchar/ftmario/ftmario.h>

#define PHANTASM_X_SPEED 460.0F
#define PHANTASM_X_SPEED_END_AIR 30.0F
#define PHANTASM_X_SPEED_END_GROUND 60.0F
#define PHANTASM_Y_SPEED_INITIAL 50.0F
#define PHANTASM_SLOW_FALL_ACCEL 1.6015625F    /* 0x3FCD0000 */
#define PHANTASM_LANDING_LAG 0.349609375F      /* 0x3EB30000 */

enum
{
    PHANTASM_GROUND_MOVE = 2,
    PHANTASM_GROUND_END_MOVE = 3
};

enum
{
    PHANTASM_AIR_INITIAL_SETUP = 1,
    PHANTASM_AIR_FREEZE_Y = 2,
    PHANTASM_AIR_MOVE = 3,
    PHANTASM_AIR_END_MOVE = 4,
    PHANTASM_AIR_SLOW_FALL = 5
};

extern sb32 ftMarioSpecialHiProcPass(GObj *fighter_gobj);

/* Phantasm.button_press_buffer: last frame's tapped buttons per port, a
 * one-frame buffer for the B "shorten" input. */
static u16 sFTRemixFalcoButtonPressBuffer[MAXCONTROLLERS];

/* Returns this frame's or last frame's tapped buttons and records this frame's. */
static u16 ftRemixFalcoUpdateButtonBuffer(FTStruct *fp)
{
    u16 buffered = sFTRemixFalcoButtonPressBuffer[fp->player];

    sFTRemixFalcoButtonPressBuffer[fp->player] = fp->input.pl.button_tap;

    return fp->input.pl.button_tap | buffered;
}

static void ftRemixFalcoEndMove(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->colanim.is_use_color1 = FALSE;
    ftParamClearAttackCollAll(fighter_gobj);
}

/* Phantasm.ground_subroutine_ (interrupt of the grounded Phantasm). */
static void ftRemixFalcoPhantasmGroundProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 state = fp->motion_vars.flags.flag2;
    u16 buttons = ftRemixFalcoUpdateButtonBuffer(fp);

    if (state == PHANTASM_GROUND_MOVE)
    {
        fp->physics.vel_ground.x = PHANTASM_X_SPEED;

        if (!(buttons & B_BUTTON))
        {
            return;
        }
        fp->motion_vars.flags.flag2 = PHANTASM_GROUND_END_MOVE;
    }
    else if (state != PHANTASM_GROUND_END_MOVE)
    {
        return;
    }
    fp->physics.vel_ground.x = PHANTASM_X_SPEED_END_GROUND;
    fp->motion_vars.flags.flag2 = 0;
    ftRemixFalcoEndMove(fighter_gobj);
}

/* Phantasm.air_subroutine_ (interrupt of the aerial Phantasm). */
static void ftRemixFalcoPhantasmAirProcInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 state = fp->motion_vars.flags.flag2;
    u16 buttons = ftRemixFalcoUpdateButtonBuffer(fp);

    if (state == PHANTASM_AIR_INITIAL_SETUP)
    {
        /* lbu 0x18D; andi 0x07: clears is_absorb..is_fastfall. */
        fp->is_absorb = FALSE;
        fp->absorb_lr = 0;
        fp->is_goto_attack100 = FALSE;
        fp->is_fastfall = FALSE;
        fp->physics.vel_air.x = 0.0F;
        fp->physics.vel_air.y = PHANTASM_Y_SPEED_INITIAL;
    }
    if (state != PHANTASM_AIR_FREEZE_Y)
    {
        if (state == PHANTASM_AIR_MOVE)
        {
            fp->physics.vel_air.x = PHANTASM_X_SPEED * fp->lr;

            if (buttons & B_BUTTON)
            {
                fp->motion_vars.flags.flag2 = PHANTASM_AIR_END_MOVE;
                goto end_move;
            }
        }
        else goto check_end_move;
    }
    /* Freezing y at 0 would still fall by one frame of gravity, so Remix
     * stores the gravity value instead. */
    fp->physics.vel_air.y = fp->attr->gravity;

check_end_move:
    if (state != PHANTASM_AIR_END_MOVE)
    {
        goto slow_fall;
    }
end_move:
    fp->physics.vel_air.x = PHANTASM_X_SPEED_END_AIR * fp->lr;
    fp->motion_vars.flags.flag2 = PHANTASM_AIR_SLOW_FALL;
    ftRemixFalcoEndMove(fighter_gobj);

slow_fall:
    if ((state != PHANTASM_AIR_SLOW_FALL) || (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        return;
    }
    fp->physics.vel_air.y += PHANTASM_SLOW_FALL_ACCEL;
}

/* Phantasm.air_physics_: no drift until the slow-fall phase. */
static void ftRemixFalcoPhantasmAirProcPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag2 == PHANTASM_AIR_SLOW_FALL)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
    }
    else ftPhysicsApplyAirVelFriction(fighter_gobj);
}

/* Phantasm.air_collision_: ftMarioSpecialHiProcMap with Falco's landing lag. */
static void ftRemixFalcoPhantasmAirProcMap(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga == nMPKineticsAir)
    {
        if ((fp->motion_vars.flags.flag1 == 0) || ((fp->physics.vel_air.y >= 0.0F)))
        {
            mpCommonCheckFighterProject(fighter_gobj);
        }
        else if (mpCommonCheckFighterPassCliff(fighter_gobj, ftMarioSpecialHiProcPass) != FALSE)
        {
            if (fp->coll_data.mask_stat & MAP_FLAG_CLIFF_MASK)
            {
                ftCommonCliffCatchSetStatus(fighter_gobj);
            }
            else ftCommonLandingFallSpecialSetStatus(fighter_gobj, FALSE, PHANTASM_LANDING_LAG);
        }
    }
    else mpCommonSetFighterFallOnEdgeBreak(fighter_gobj);
}

/* Remix address -> port routine for every Falco routine its action array or
 * special tables reference. */
const FTRemixProc dFTRemixFalcoProcs[] =
{
    { REMIX_SYM_Phantasm__ground_subroutine_, ftRemixFalcoPhantasmGroundProcInterrupt },
    { REMIX_SYM_Phantasm__air_subroutine_, ftRemixFalcoPhantasmAirProcInterrupt },
    { REMIX_SYM_Phantasm__air_physics_, ftRemixFalcoPhantasmAirProcPhysics },
    { REMIX_SYM_Phantasm__air_collision_, ftRemixFalcoPhantasmAirProcMap }
};

const s32 dFTRemixFalcoProcsNum = ARRAY_COUNT(dFTRemixFalcoProcs);

#endif
