/**
 * mnremixcssafk.c - "afk" automation of the VS character select, see mnremixcssafk.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/controller.h>
#include "mn/mnplayers/mnremixcssafk.h"
#include "mn/mnplayers/mnremixcssafkrules.h"
#include "sc/sc1pmode/sc1premixenemyafk.h"
#include "sc/scsubsys/scsubsys.h"

/* The timer lives as long as the session, like afk_timer in Remix RAM. */
static s32 sMNRemixCssAfkTimer;

static sb32 mnRemixCssAfkIsButtonHeld(void)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        if (gSYControllerDevices[port].button_hold & MNREMIX_CSS_AFK_BUTTON_MASK)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void mnRemixCssAfkUpdate(sb32 is_ready, s32 total_tics)
{
    sMNRemixCssAfkTimer = mnRemixCssAfkRulesNextTimer(sMNRemixCssAfkTimer, (is_ready != FALSE) ? 1 : 0, total_tics);
}

sb32 mnRemixCssAfkTapStart(void)
{
    if ((sc1PRemixAfkIsActive() != FALSE) && (mnRemixCssAfkRulesShouldStart(sMNRemixCssAfkTimer, (mnRemixCssAfkIsButtonHeld() != FALSE) ? 1 : 0) != 0))
    {
        sMNRemixCssAfkTimer = 0;
        return TRUE;
    }
    return scSubsysControllerGetPlayerTapButtons(START_BUTTON);
}

sb32 mnRemixCssAfkPlaysDeniedSound(void)
{
    return (sc1PRemixAfkIsActive() == FALSE) ? TRUE : FALSE;
}

#endif
