#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostrules.h"
#include "ft/ftremix/ftremixaipostusp.h"

/** Level 10 gate of the shared routines (slti 10, or the level minus 10 for Captain; the level never exceeds 10). */
static sb32 ftRemixAiPostIsLevelTenCpu(FTStruct *fp)
{
    return ftRemixAiIsLevelTen(fp->level);
}

/** Sends the CPU stick toward the target (AI.ROUTINE.POINT_STICK_TO_TARGET). */
static void ftRemixAiPostPointStickToTarget(FTStruct *fp)
{
    ftComputerSetCommandImmediate(fp, FTREMIX_AI_POST_ROUTINE_POINT_STICK_TO_TARGET);
}

/**
 * Dive control of Ganondorf and Captain Falcon (_point_to_target). Above ground, a fighter that does not face the target
 * holds the stick toward it for a turnaround, otherwise the stick points at it. Over no ground nothing changes, so the
 * recovery is not disturbed.
 */
static void ftRemixAiPostSteerDiveToTarget(FTStruct *fp)
{
    f32 target_x_diff;
    s32 stick_x;

    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        return;
    }
    target_x_diff = fp->computer.target_pos.x - fp->coll_data.p_translate->x;
    stick_x = ftRemixAiPostDiveTurnStickX(target_x_diff, fp->lr);

    if (stick_x == 0)
    {
        ftRemixAiPostPointStickToTarget(fp);
        return;
    }
    ftRemixAiPostSetStick(fp, stick_x, 0);
    ftRemixAiPostSetNullCommand(fp);
}

void ftRemixAiPostProcessBowser(FTStruct *fp)
{
    if (ftRemixAiPostIsWhirlingFortress(fp->status_id))
    {
        ftRemixAiPostPointStickToTarget(fp);
    }
}

void ftRemixAiPostProcessDragonKing(FTStruct *fp)
{
    if (ftRemixAiPostIsDragonKingUsp(fp->status_id) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostHasFloorBelow(fp))
    {
        ftRemixAiPostPointStickToTarget(fp);
    }
}

void ftRemixAiPostProcessGanondorf(FTStruct *fp)
{
    if (ftRemixAiPostIsDiveGrab(fp->status_id))
    {
        ftRemixAiPostSteerDiveToTarget(fp);
    }
}

void ftRemixAiPostProcessCaptain(FTStruct *fp)
{
    if (ftRemixAiPostIsLevelTenCpu(fp) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostIsDiveGrab(fp->status_id))
    {
        ftRemixAiPostSteerDiveToTarget(fp);
    }
}

void ftRemixAiPostProcessFox(FTStruct *fp)
{
    if (ftRemixAiPostIsLevelTenCpu(fp) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostIsFoxFamilyUsp(fp->status_id))
    {
        ftRemixAiPostDiveDown(fp);
    }
}

void ftRemixAiPostProcessPikachu(FTStruct *fp)
{
    if (ftRemixAiPostIsLevelTenCpu(fp) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostIsQuickAttack(fp->status_id))
    {
        ftRemixAiPostDiveDown(fp);
    }
}

#endif
