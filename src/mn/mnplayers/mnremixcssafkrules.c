/**
 * mnremixcssafkrules.c - pure rules of the "afk" automation of the VS character select, see mnremixcssafkrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcssafkrules.h"

int mnRemixCssAfkRulesNextTimer(int timer, int is_ready, int total_tics)
{
    if ((is_ready == 0) || (total_tics < MNREMIX_CSS_AFK_OPEN_FRAMES))
    {
        return 0;
    }
    return timer + 1;
}

int mnRemixCssAfkRulesShouldStart(int timer, int is_button_held)
{
    return ((is_button_held == 0) && (timer >= MNREMIX_CSS_AFK_START_FRAMES)) ? 1 : 0;
}

#endif
