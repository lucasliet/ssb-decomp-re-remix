#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixairecoveryrules.h"

/** Level 10 and the improved AI repeat up-B, vanilla 1P/RTTF never does. */
int ftRemixAiRecoveryUpBRepeatAllowed(int cpu_level, int is_classic_mode, int improved_ai)
{
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

#endif
