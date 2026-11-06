/**
 * sc1pbonustimer.c - which timer a 1P game run shows, see sc1pbonustimer.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scdef.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include "gm/gmremixpractice.h"
#include "sc/sc1pmode/sc1pbonusstagerules.h"
#include "sc/sc1pmode/sc1pbonustimer.h"
#include "sc/sc1pmode/sc1premixbonus3.h"
#include "sc/sc1pmode/sc1premixhrc.h"

sb32 sc1PBonusTimerIsCountUpRun(void)
{
    if (gSCManagerSceneData.scene_curr != nSCKind1PGame)
    {
        return FALSE;
    }
    return (sc1PBonusStageRulesUsesCountUpTimer(sc1PRemixBonus3IsActive() != FALSE, sc1PRemixHrcIsActive() != FALSE, gmRemixPracticeIsActive() != FALSE) != 0) ? TRUE : FALSE;
}

#endif
