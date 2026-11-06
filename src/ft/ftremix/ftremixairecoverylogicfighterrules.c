/*
 * Recovery routines that single fighters register in Character.recovery_logic (Banjo, Wario, Marth and Roy, Crash, Dedede,
 * Falco, Sheik, Marina, Sonic, Peach): AI.asm custom_recovery_logic calls them after the walk step. Unlike the shared
 * families, most of these have no level 10 gate.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <math.h>
#include "ft/ftremix/ftremixairecoverylogicids.h"
#include "ft/ftremix/ftremixairecoverylogicrules.h"

#define FTREMIX_AI_RLOGIC_BANJO_MIN_X 1800.0F
#define FTREMIX_AI_RLOGIC_BANJO_REACH_WITH_JUMP 1200.0F
#define FTREMIX_AI_RLOGIC_BANJO_REACH_NO_JUMP 800.0F
#define FTREMIX_AI_RLOGIC_WARIO_MIN_X 2496.0F
#define FTREMIX_AI_RLOGIC_WARIO_UP_HIGH_Y (-1000.0F)
#define FTREMIX_AI_RLOGIC_WARIO_REACH_WITH_JUMP 1496.0F
#define FTREMIX_AI_RLOGIC_WARIO_REACH_NO_JUMP 200.0F
#define FTREMIX_AI_RLOGIC_MARTH_MIN_X 1000.0F
#define FTREMIX_AI_RLOGIC_MARTH_LOW_Y 1000.0F
#define FTREMIX_AI_RLOGIC_CRASH_MIN_X 2000.0F
#define FTREMIX_AI_RLOGIC_DEDEDE_MAX_X 2992.0F /* 0x453B */
#define FTREMIX_AI_RLOGIC_DEDEDE_UP_HIGH_Y (-1200.0F)
#define FTREMIX_AI_RLOGIC_FALCO_MAX_X 2000.0F
#define FTREMIX_AI_RLOGIC_FALCO_NSP_ROLL 8
#define FTREMIX_AI_RLOGIC_SHEIK_MIN_X 2000.0F
#define FTREMIX_AI_RLOGIC_MARINA_MIN_X 1000.0F
#define FTREMIX_AI_RLOGIC_MARINA_UP_HIGH_Y (-2000.0F)
#define FTREMIX_AI_RLOGIC_SONIC_MIN_X 2496.0F
#define FTREMIX_AI_RLOGIC_SONIC_NO_JUMP_Y (-1496.0F)
#define FTREMIX_AI_RLOGIC_PEACH_MIN_X 2000.0F
#define FTREMIX_AI_RLOGIC_PEACH_UP_HIGH_Y (-2000.0F)
#define FTREMIX_AI_RLOGIC_PEACH_FLOAT_MIN_X 1000.0F

/* Banjo picks the nearest ledge like the others, but only recovers while facing the direction the asm expects. */
void ftRemixAiRecoveryLogicBanjo(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float reach;
    int is_left_ledge;

    if ((state->action_id == FTREMIX_AI_RLOGIC_ACTION_ATTACK_AIR_F) || (state->jump_command_active != 0))
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    is_left_ledge = fabsf(state->left_x - state->x) <= fabsf(state->right_x - state->x);

    if (state->lr != (is_left_ledge ? 1 : -1))
    {
        return;
    }
    if ((state->vel_y > 0.0F) || (fabsf(ledge_x - state->x) <= FTREMIX_AI_RLOGIC_BANJO_MIN_X))
    {
        return;
    }
    reach = ftRemixAiRecoveryLogicHasNoJumpsLeft(state) ? FTREMIX_AI_RLOGIC_BANJO_REACH_NO_JUMP
                                                        : FTREMIX_AI_RLOGIC_BANJO_REACH_WITH_JUMP;

    if (reach > (ledge_y - state->y))
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_FTILT, ledge_x, ledge_y);
        result->sets_stick_y = 1;
        result->stick_y = 0;
    }
}

/* DEVIATION: during Body Slam the asm reads f14 before it is computed (a stale register); the nearest ledge side is used. */
void ftRemixAiRecoveryLogicWario(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float y_diff;
    float reach;

    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    y_diff = ledge_y - state->y;

    if (state->action_id == FTREMIX_AI_RLOGIC_ACTION_WARIO_BODY_SLAM_AIR)
    {
        ftRemixAiRecoveryLogicHoldUpToward(result, x_diff);
        return;
    }
    if (fabsf(x_diff) <= FTREMIX_AI_RLOGIC_WARIO_MIN_X)
    {
        return;
    }
    reach = ftRemixAiRecoveryLogicHasNoJumpsLeft(state) ? FTREMIX_AI_RLOGIC_WARIO_REACH_NO_JUMP
                                                        : FTREMIX_AI_RLOGIC_WARIO_REACH_WITH_JUMP;

    if ((y_diff <= FTREMIX_AI_RLOGIC_WARIO_UP_HIGH_Y) || (reach > y_diff))
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

void ftRemixAiRecoveryLogicMarth(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_diff;
    float y_diff;

    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_diff = ledge_x - state->x;
    y_diff = ledge_y - state->y;

    if (state->action_id == FTREMIX_AI_RLOGIC_ACTION_MARTH_NSPA_1)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET, ledge_x, ledge_y);
        return;
    }
    if ((state->vel_y > 0.0F) || (fabsf(x_diff) <= FTREMIX_AI_RLOGIC_MARTH_MIN_X))
    {
        return;
    }
    if (fabsf(state->vel_x) < (state->air_speed_max_x - state->air_friction))
    {
        return;
    }
    if (y_diff < FTREMIX_AI_RLOGIC_MARTH_LOW_Y)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

void ftRemixAiRecoveryLogicCrash(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;

    if (state->action_id != FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F)
    {
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    if (fabsf(ledge_x - state->x) < FTREMIX_AI_RLOGIC_CRASH_MIN_X)
    {
        return;
    }
    ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
}

void ftRemixAiRecoveryLogicDedede(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;

    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    if (fabsf(ledge_x - state->x) >= FTREMIX_AI_RLOGIC_DEDEDE_MAX_X)
    {
        return;
    }
    if (((ledge_y - state->y) > FTREMIX_AI_RLOGIC_DEDEDE_UP_HIGH_Y) || (state->minions_full != 0))
    {
        return;
    }
    ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, ledge_x, ledge_y);
}

void ftRemixAiRecoveryLogicFalco(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;

    if (state->action_id == FTREMIX_AI_RLOGIC_ACTION_FALCO_PHANTASM_AIR)
    {
        result->sets_buttons = 1;
        result->buttons = state->buttons | FTREMIX_AI_RECOVERY_LOGIC_BUTTON_B;
        return;
    }
    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    if (fabsf(ledge_x - state->x) >= FTREMIX_AI_RLOGIC_FALCO_MAX_X)
    {
        return;
    }
    if (((state->y + state->cliff_catch_y) <= ledge_y) || (state->y > ledge_y))
    {
        return;
    }
    ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    result->roll_modulus = FTREMIX_AI_RLOGIC_FALCO_NSP_ROLL;
}

/* Sheik uses the ledge on the side she faces away from: facing right reads the left ledge and must be left of it. */
void ftRemixAiRecoveryLogicSheik(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float x_diff;

    if (state->lr < 0)
    {
        x_diff = state->right_x - state->x;
        if (x_diff >= 0.0F)
        {
            return;
        }
    }
    else
    {
        x_diff = state->left_x - state->x;
        if (x_diff <= 0.0F)
        {
            return;
        }
    }
    if ((state->word_adc != 0) || (fabsf(x_diff) <= FTREMIX_AI_RLOGIC_SHEIK_MIN_X))
    {
        return;
    }
    result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP;
}

void ftRemixAiRecoveryLogicMarina(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;

    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    if ((state->vel_y > 0.0F) || (fabsf(ledge_x - state->x) <= FTREMIX_AI_RLOGIC_MARINA_MIN_X))
    {
        return;
    }
    if ((ledge_y - state->y) <= FTREMIX_AI_RLOGIC_MARINA_UP_HIGH_Y)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

void ftRemixAiRecoveryLogicSonic(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float threshold;

    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);

    if ((state->vel_y > 0.0F) || (fabsf(ledge_x - state->x) <= FTREMIX_AI_RLOGIC_SONIC_MIN_X))
    {
        return;
    }
    threshold = ftRemixAiRecoveryLogicHasNoJumpsLeft(state) ? FTREMIX_AI_RLOGIC_SONIC_NO_JUMP_Y : 0.0F;

    if ((ledge_y - state->y) <= threshold)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

static int ftRemixAiRecoveryLogicPeachIsFloating(int action_id)
{
    return (action_id == FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F) || (action_id == FTREMIX_AI_RLOGIC_ACTION_PEACH_FLOAT) ||
           (action_id == FTREMIX_AI_RLOGIC_ACTION_FALL_AERIAL);
}

/* Peach holds C-up to keep floating while her jump brings her toward the ledge, and releases it once she is close. */
void ftRemixAiRecoveryLogicPeach(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    float ledge_x;
    float ledge_y;
    float x_distance;

    ftRemixAiRecoveryLogicNearestLedge(state, &ledge_x, &ledge_y);
    x_distance = fabsf(ledge_x - state->x);

    if (ftRemixAiRecoveryLogicPeachIsFloating(state->action_id))
    {
        result->sets_buttons = 1;
        result->buttons = (x_distance <= FTREMIX_AI_RLOGIC_PEACH_FLOAT_MIN_X)
                              ? (state->buttons & FTREMIX_AI_RECOVERY_LOGIC_BUTTONS_C_MASK)
                              : (state->buttons | FTREMIX_AI_RECOVERY_LOGIC_BUTTON_C_UP);
        return;
    }
    if (x_distance <= FTREMIX_AI_RLOGIC_PEACH_MIN_X)
    {
        return;
    }
    if ((ledge_y - state->y) <= FTREMIX_AI_RLOGIC_PEACH_UP_HIGH_Y)
    {
        ftRemixAiRecoveryLogicTargetLedge(result, FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, ledge_x, ledge_y);
    }
}

#endif
