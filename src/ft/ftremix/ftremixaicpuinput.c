#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>
#include "ft/ftremix/ftremixaicpuinput.h"
#include "ft/ftremix/ftremixaicpuinputrules.h"
#include "remix/remix_roster_data.h"

sb32 ftRemixAiAutoJabGrab(FTStruct *fp)
{
    sb32 is_cpu = (fp->pkind != nFTPlayerKindMan) ? TRUE : FALSE;

    return ftRemixAiJabGrabRulesApplies(is_cpu, fp->level, (s32)fp->status_total_tics, gmRemixToggle(single_button_mode));
}

s32 ftRemixAiDashStatus(FTStruct *fp, s32 status_id)
{
    sb32 is_giga_bowser = (fp->fkind == REMIX_ID_GBOWSER) ? TRUE : FALSE;
    sb32 is_cpu = (fp->pkind != nFTPlayerKindMan) ? TRUE : FALSE;

    if (ftRemixAiDashRulesUsesWalk(is_giga_bowser, is_cpu))
    {
        return nFTCommonStatusWalkFast;
    }
    return status_id;
}

#endif
