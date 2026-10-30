#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixairollrules.h"

/** Only the exact level 10 gets the advanced routines. */
int ftRemixAiRollIsLevelTen(int level)
{
    return level == FTREMIX_AI_ROLL_LEVEL_TEN;
}

/** The proc test is a level 10 only extra: the status test is vanilla and applies at every level. */
int ftRemixAiRollCanJumpWhileFalling(int level, int proc_is_aerial_jump, int status_is_fall_special)
{
    if (status_is_fall_special)
    {
        return 1;
    }
    return ftRemixAiRollIsLevelTen(level) && proc_is_aerial_jump;
}

/** Idle level 10 CPUs shield drop, every other CPU does the normal platform drop. */
int ftRemixAiRollShieldDrops(int level, int is_idle)
{
    return ftRemixAiRollIsLevelTen(level) && is_idle;
}

#endif
