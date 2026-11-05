#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixairecoveryrules.h"

#define FTREMIX_AI_RECOVERY_BANJO_FLIGHT 0xEF
#define FTREMIX_AI_RECOVERY_BANJO_FLAP_FRAME 21
#define FTREMIX_AI_RECOVERY_NO_FLOOR_LINE (-1)

/** Remix 1P and level 10 repeat up-B, then vanilla 1P/RTTF never does, then the improved AI decides. */
int ftRemixAiRecoveryUpBRepeatAllowed(int cpu_level, int is_classic_mode, int improved_ai, int is_remix_1p)
{
    if (is_remix_1p)
    {
        return 1;
    }
    if (cpu_level >= FTREMIX_AI_RECOVERY_CPU_LEVEL_MAX)
    {
        return 1;
    }
    if (is_classic_mode)
    {
        return 0;
    }
    return improved_ai != 0;
}

/**
 * The asm compares with `<=` on the left bound and `<=` on the right bound (target >= right - padding), so a target
 * exactly on the padded edge is moved to that edge.
 */
float ftRemixAiRecoveryClampTargetX(float target_x, float left_blastzone, float right_blastzone)
{
    float left_limit = left_blastzone + FTREMIX_AI_RECOVERY_BLASTZONE_PADDING;
    float right_limit = right_blastzone - FTREMIX_AI_RECOVERY_BLASTZONE_PADDING;

    if (target_x <= left_limit)
    {
        return left_limit;
    }
    if (right_limit <= target_x)
    {
        return right_limit;
    }
    return target_x;
}

/** Wolf USP actions 0xE3, 0xE4, 0xE6 and 0xE8 (fix_remix_recovery, _wolf). */
int ftRemixAiRecoveryIsWolfUspAction(int action)
{
    return (action == 0xE3) || (action == 0xE4) || (action == 0xE6) || (action == 0xE8);
}

/** A negative charge_action means the fighter has no NSP charge recovery, so it never matches. */
int ftRemixAiRecoveryIsNspChargeAction(int action, int charge_action)
{
    return (charge_action >= 0) && (action == charge_action);
}

/**
 * Banjo Flight (0xEF) at frame 21 (fix_remix_recovery, _banjo_flight). The asm comment says frame 22, but the compare is
 * with 21, which is the value that runs.
 */
int ftRemixAiRecoveryIsBanjoFlapFrame(int action, int frame)
{
    return (action == FTREMIX_AI_RECOVERY_BANJO_FLIGHT) && (frame == FTREMIX_AI_RECOVERY_BANJO_FLAP_FRAME);
}

/** No ground below (clipping -1) makes Banjo press B by command and skip the custom recovery (_banjo_flight). */
int ftRemixAiRecoveryBanjoNeedsPressB(int floor_line_id)
{
    return floor_line_id == FTREMIX_AI_RECOVERY_NO_FLOOR_LINE;
}

/** The vanilla Pikachu branch is kept in classic mode and when the improved AI is off (fix_remix_recovery, _pikachu). */
int ftRemixAiRecoveryKeepsPikachuVanilla(int is_classic_mode, int improved_ai)
{
    return (is_classic_mode != 0) || (improved_ai == 0);
}

#endif
