#if defined(PORT) && defined(REGION_REMIX)

#include <math.h>
#include "ft/ftremix/ftremixairecoverylogicrules.h"

void ftRemixAiRecoveryLogicClearResult(FTRemixAiRecoveryLogicResult *result)
{
    result->command = FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE;
    result->sets_target = 0;
    result->target_x = 0.0F;
    result->target_y = 0.0F;
    result->sets_stick_x = 0;
    result->stick_x = 0;
    result->sets_stick_y = 0;
    result->stick_y = 0;
    result->sets_buttons = 0;
    result->buttons = 0;
    result->roll_modulus = 0;
}

void ftRemixAiRecoveryLogicNearestLedge(const FTRemixAiRecoveryLogicState *state, float *ledge_x, float *ledge_y)
{
    if (fabsf(state->left_x - state->x) <= fabsf(state->right_x - state->x))
    {
        *ledge_x = state->left_x;
        *ledge_y = state->left_y;
        return;
    }
    *ledge_x = state->right_x;
    *ledge_y = state->right_y;
}

void ftRemixAiRecoveryLogicTargetLedge(FTRemixAiRecoveryLogicResult *result, int command, float ledge_x, float ledge_y)
{
    result->sets_target = 1;
    result->target_x = ledge_x;
    result->target_y = ledge_y;
    result->command = command;
}

void ftRemixAiRecoveryLogicHoldUpToward(FTRemixAiRecoveryLogicResult *result, float x_diff)
{
    result->command = FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NULL;
    result->sets_stick_y = 1;
    result->stick_y = FTREMIX_AI_RECOVERY_LOGIC_STICK_MAX;
    result->sets_stick_x = 1;
    result->stick_x = (x_diff < 0.0F) ? -FTREMIX_AI_RECOVERY_LOGIC_STICK_MAX : FTREMIX_AI_RECOVERY_LOGIC_STICK_MAX;
}

int ftRemixAiRecoveryLogicHasNoJumpsLeft(const FTRemixAiRecoveryLogicState *state)
{
    return state->jumps_max == state->jumps_used;
}

void ftRemixAiRecoveryLogicRun(int kind, const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result)
{
    ftRemixAiRecoveryLogicClearResult(result);

    switch (kind)
    {
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO:
        ftRemixAiRecoveryLogicMario(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_PIKACHU:
        ftRemixAiRecoveryLogicPikachu(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF:
        ftRemixAiRecoveryLogicPuff(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI:
        ftRemixAiRecoveryLogicYoshi(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_CAPTAIN:
        ftRemixAiRecoveryLogicCaptain(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_LINK:
        ftRemixAiRecoveryLogicLink(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX:
        ftRemixAiRecoveryLogicFox(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO:
        ftRemixAiRecoveryLogicBanjo(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_WARIO:
        ftRemixAiRecoveryLogicWario(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_MARTH:
        ftRemixAiRecoveryLogicMarth(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_CRASH:
        ftRemixAiRecoveryLogicCrash(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE:
        ftRemixAiRecoveryLogicDedede(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_FALCO:
        ftRemixAiRecoveryLogicFalco(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_SHEIK:
        ftRemixAiRecoveryLogicSheik(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_MARINA:
        ftRemixAiRecoveryLogicMarina(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_SONIC:
        ftRemixAiRecoveryLogicSonic(state, result);
        break;
    case FTREMIX_AI_RECOVERY_LOGIC_KIND_PEACH:
        ftRemixAiRecoveryLogicPeach(state, result);
        break;
    default:
        break;
    }
}

#endif
