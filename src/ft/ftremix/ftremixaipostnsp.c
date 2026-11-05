#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sys/utils.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostnsp.h"
#include "ft/ftremix/ftremixaipostrules.h"

void ftRemixAiPostProcessSwordNsp(FTStruct *fp)
{
    FTStruct *target_fp;
    s32 roll = 0;
    s32 input;

    if (ftRemixAiPostIsSwordNsp(fp->status_id) == FALSE)
    {
        return;
    }
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (fp->hitlag_tics == 0)
    {
        return;
    }
    if (ftRemixAiPostSwordNspNeedsRoll(fp->status_id, target_fp->status_id, target_fp->percent_damage))
    {
        roll = syUtilsRandIntRange(FTREMIX_AI_POST_SWORD_NSP_ROLL_RANGE);
    }
    input = ftRemixAiPostSwordNspInput(fp->status_id, target_fp->status_id, target_fp->percent_damage, roll);

    ftComputerSetCommandImmediate(fp, input);
    fp->computer.input_wait = FTREMIX_AI_POST_SWORD_NSP_WAIT;
}

#endif
