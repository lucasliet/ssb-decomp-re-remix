#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaicpuinputrules.h"
#include "ft/ftremix/ftremixaiteachingrules.h"

int ftRemixAiJabGrabRulesApplies(int is_cpu, int level, int status_total_tics, int single_button_mode)
{
    if (is_cpu == 0)
    {
        return 0;
    }
    if (level < FTREMIX_AI_JAB_GRAB_LEVEL)
    {
        return 0;
    }
    if (status_total_tics < FTREMIX_AI_JAB_GRAB_MIN_STATUS_TICS)
    {
        return 0;
    }
    return ftRemixAiTechSingleButtonAllows(single_button_mode);
}

int ftRemixAiDashRulesUsesWalk(int is_giga_bowser, int is_cpu)
{
    return (is_giga_bowser != 0) && (is_cpu != 0);
}

#endif
