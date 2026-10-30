#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixailv10rules.h"

#define FTREMIX_AI_LV10_START_BUTTON 0x1000

int ftRemixAiIsLevelTen(int cpu_level)
{
    return cpu_level >= FTREMIX_AI_LV10_LEVEL;
}

int ftRemixAiCountsAsInvulnerableEnemy(int star_invincible, int respawn_tics, int cpu_level)
{
    if (star_invincible != 0)
    {
        return 1;
    }
    return ftRemixAiIsLevelTen(cpu_level) && (respawn_tics != 0);
}

int ftRemixAiCliffGetupInputFromRoll(int roll, int normal_input)
{
    if ((roll >> 1) == 0)
    {
        return FTREMIX_AI_LV10_ROUTINE_CLIFF_LET_GO;
    }
    return normal_input;
}

int ftRemixAiStartMashButtons(int buttons, int frame_count)
{
    if ((frame_count & FTREMIX_AI_LV10_MASH_FRAME_BIT) != 0)
    {
        return buttons | FTREMIX_AI_LV10_START_BUTTON;
    }
    return 0;
}

int ftRemixAiDashAttackCancelNeeded(int is_dash_attack, int is_idle_or_run)
{
    return (is_dash_attack != 0) && (is_idle_or_run == 0);
}

#endif
