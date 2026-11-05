/*
 * Recovery routines registered by the Remix family shared files (marioshared, pikashared, jigglypuffkirbyshared, yoshishared,
 * nessshared, captainshared, linkshared, foxshared): AI.asm custom_recovery_logic calls them after the walk step.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <math.h>
#include "ft/ftremix/ftremixairecoverylogicids.h"
#include "ft/ftremix/ftremixairecoverylogicrules.h"

#define FTREMIX_AI_RLOGIC_MARIO_UP_HIGH_Y (-2000.0F)
#define FTREMIX_AI_RLOGIC_MARIO_DSP_MIN_X 2560.0F
#define FTREMIX_AI_RLOGIC_MARIO_MASH_END_FRAME 50
#define FTREMIX_AI_RLOGIC_PIKACHU_ZIP_REACH 2000.0F
#define FTREMIX_AI_RLOGIC_PIKACHU_ZIP_DEVIATION 0.43633232F /* 25 degrees, 0x3EDF66F3 */
#define FTREMIX_AI_RLOGIC_PIKACHU_ZIP_AIM_DISTANCE 1000.0F
#define FTREMIX_AI_RLOGIC_PUFF_MIN_X 1000.0F
#define FTREMIX_AI_RLOGIC_PUFF_UP_HIGH_Y (-2000.0F)
#define FTREMIX_AI_RLOGIC_CAPTAIN_MIN_X 4000.0F
#define FTREMIX_AI_RLOGIC_CAPTAIN_UP_HIGH_Y (-100.0F)
#define FTREMIX_AI_RLOGIC_CAPTAIN_NSP_MAX_FRAME 20
#define FTREMIX_AI_RLOGIC_LINK_MIN_X 2000.0F
#define FTREMIX_AI_RLOGIC_LINK_UP_HIGH_Y (-2000.0F)
#define FTREMIX_AI_RLOGIC_FOX_REACH_WITH_JUMP 4000.0F
#define FTREMIX_AI_RLOGIC_FOX_REACH_NO_JUMP 2000.0F
#define FTREMIX_AI_RLOGIC_FOX_DSP_ROLL 20

void ftRemixAiRecoveryLogicMario(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float y_diff;

    if (state->level_ten == 0)
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    y_diff = ledge_y - state->y;

    if (state->action_id == FTREMIX_AI_RLOGIC_ACTION_MARIO_TORNADO_AIR)
    {
        if (state->status_frame >= FTREMIX_AI_RLOGIC_MARIO_MASH_END_FRAME)
        {
            return;
        }
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET, ledge_x, ledge_y);
        result->sets_buttons = 1;
        result->buttons = (state->frame_is_odd != 0) ? (state->buttons | FTREMIX_AI_RECOVERY_LOGIC_BUTTON_B) : 0;
        return;
    }
    if (y_diff <= FTREMIX_AI_RLOGIC_MARIO_UP_HIGH_Y)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
        return;
    }
    if ((state->word_adc != 0) || (fabsf(x_diff) <= FTREMIX_AI_RLOGIC_MARIO_DSP_MIN_X))
    {
        return;
    }
    if (y_diff >= 0.0F)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, ledge_x, ledge_y);
    }
}

/* First zip: aims 25 degrees above the line to the ledge (the target here is the ledge the walk step chose), so the second zip
 * can come out. A ledge within one zip is pointed at directly. */
static void ftRemixAiRecoveryLogicPikachuFirstZip(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float x_diff = state->target_x - state->x;
    float y_diff = state->target_y - state->y;
    float angle;

    result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET;

    if (sqrtf((x_diff * x_diff) + (y_diff * y_diff)) <= FTREMIX_AI_RLOGIC_PIKACHU_ZIP_REACH)
    {
        return;
    }
    angle = atan2f(y_diff, x_diff);
    angle += (x_diff < 0.0F) ? -FTREMIX_AI_RLOGIC_PIKACHU_ZIP_DEVIATION : FTREMIX_AI_RLOGIC_PIKACHU_ZIP_DEVIATION;

    result->sets_target = 1;
    result->target_x = state->x + (cosf(angle) * FTREMIX_AI_RLOGIC_PIKACHU_ZIP_AIM_DISTANCE);
    result->target_y = state->y + (sinf(angle) * FTREMIX_AI_RLOGIC_PIKACHU_ZIP_AIM_DISTANCE);
}

void ftRemixAiRecoveryLogicPikachu(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    if (state->level_ten == 0)
    {
        return;
    }
    switch (state->action_id)
    {
    case FTREMIX_AI_RLOGIC_ACTION_PIKACHU_QUICK_ATTACK_START_AIR:
        ftRemixAiRecoveryLogicPikachuFirstZip(state, result);
        break;

    case FTREMIX_AI_RLOGIC_ACTION_PIKACHU_QUICK_ATTACK_AIR:
    case FTREMIX_AI_RLOGIC_ACTION_PIKACHU_QUICK_ATTACK_END_AIR:
        result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET;
        break;
    }
}

/* The multi jump statuses of Jigglypuff and its clones (Jump2 to Jump5; the last jump is left out on purpose, so the ledge can
 * be grabbed). */
static int ftRemixAiRecoveryLogicPuffIsMidJump(int action_id)
{
    if (action_id == FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F)
    {
        return 1;
    }
    return (action_id >= FTREMIX_AI_RLOGIC_ACTION_JIGGLY_JUMP2) && (action_id <= FTREMIX_AI_RLOGIC_ACTION_JIGGLY_JUMP5);
}

/* DEVIATION: during Pound the asm reads f14 before it is computed (a stale register); the nearest ledge side is used. */
void ftRemixAiRecoveryLogicPuff(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float y_diff;

    if (state->level_ten == 0)
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    y_diff = ledge_y - state->y;

    if (state->action_id == FTREMIX_AI_RLOGIC_ACTION_JIGGLY_POUND_AIR)
    {
        ftRemixAiRecoveryLogicHoldUpToward(result, x_diff);
        return;
    }
    if (fabsf(x_diff) <= FTREMIX_AI_RLOGIC_PUFF_MIN_X)
    {
        return;
    }
    if ((y_diff <= FTREMIX_AI_RLOGIC_PUFF_UP_HIGH_Y) || ftRemixAiRecoveryLogicPuffIsMidJump(state->action_id))
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

/* Yoshi, J Yoshi, Ness, J Ness and Lucas share one body (yoshishared.asm and nessshared.asm): DSP after the double jump, when
 * the ledge cannot be reached by the grab box. The asm reads attributes 0xB0 for both the grab X and the grab Y. */
void ftRemixAiRecoveryLogicYoshi(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float grab_x;

    if ((state->level_ten == 0) || (state->action_id != FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F) || (state->vel_y <= 0.0F))
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    grab_x = state->x + ((float)state->lr * state->cliff_catch_y);

    if (((state->y + state->cliff_catch_y) <= ledge_y) || (ledge_y <= state->y))
    {
        return;
    }
    if (x_diff < 0.0F)
    {
        if (ledge_x > grab_x)
        {
            result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP;
        }
        return;
    }
    if (grab_x > ledge_x)
    {
        result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP;
    }
}

/* DEVIATION: during Falcon Punch the asm reads f14 before it is computed (a stale register); the nearest ledge side is used. */
void ftRemixAiRecoveryLogicCaptain(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float y_diff;

    if (state->level_ten == 0)
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    y_diff = ledge_y - state->y;

    if (state->action_id == FTREMIX_AI_RLOGIC_ACTION_CAPTAIN_FALCON_PUNCH_AIR)
    {
        ftRemixAiRecoveryLogicHoldUpToward(result, x_diff);
        return;
    }
    if ((fabsf(x_diff) <= FTREMIX_AI_RLOGIC_CAPTAIN_MIN_X) || (y_diff > FTREMIX_AI_RLOGIC_CAPTAIN_UP_HIGH_Y))
    {
        return;
    }
    if (state->action_id != FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F)
    {
        return;
    }
    if (fabsf(state->vel_x) < (state->air_speed_max_x - state->air_friction))
    {
        return;
    }
    if (state->status_frame < FTREMIX_AI_RLOGIC_CAPTAIN_NSP_MAX_FRAME)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

void ftRemixAiRecoveryLogicLink(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;

    if ((state->level_ten == 0) || (state->has_item != 0))
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    if (fabsf(ledge_x - state->x) <= FTREMIX_AI_RLOGIC_LINK_MIN_X)
    {
        return;
    }
    if ((ledge_y - state->y) <= FTREMIX_AI_RLOGIC_LINK_UP_HIGH_Y)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, ledge_x, ledge_y);
    }
}

void ftRemixAiRecoveryLogicFox(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float y_diff;
    float reach;

    if (state->level_ten == 0)
    {
        return;
    }
    if ((state->action_id >= FTREMIX_AI_RLOGIC_ACTION_FOX_FIRE_FOX_FIRST) &&
        (state->action_id <= FTREMIX_AI_RLOGIC_ACTION_FOX_FIRE_FOX_LAST))
    {
        return;
    }
    if (state->vel_y > 0.0F)
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    y_diff = ledge_y - state->y;
    reach = ftRemixAiRecoveryLogicHasNoJumpsLeft(state) ? FTREMIX_AI_RLOGIC_FOX_REACH_NO_JUMP
                                                        : FTREMIX_AI_RLOGIC_FOX_REACH_WITH_JUMP;

    if (sqrtf((x_diff * x_diff) + (y_diff * y_diff)) > reach)
    {
        return;
    }
    result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP;
    result->roll_modulus = FTREMIX_AI_RLOGIC_FOX_DSP_ROLL;
}

#endif
