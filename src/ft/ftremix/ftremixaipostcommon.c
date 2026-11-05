#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/mpcollision.h>
#include <math.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostrules.h"

#define FTREMIX_AI_POST_NO_FLOOR_LINE (-1)

sb32 ftRemixAiPostCommandIs(FTStruct *fp, s32 input_kind)
{
    return fp->computer.p_command == ftComputerGetInputScript(input_kind);
}

FTStruct *ftRemixAiPostGetTarget(FTStruct *fp)
{
    if (fp->computer.target_gobj == NULL)
    {
        return NULL;
    }
    return ftGetStruct(fp->computer.target_gobj);
}

sb32 ftRemixAiPostHasFloorBelow(FTStruct *fp)
{
    return fp->coll_data.floor_line_id != FTREMIX_AI_POST_NO_FLOOR_LINE;
}

void ftRemixAiPostSetStick(FTStruct *fp, s32 stick_x, s32 stick_y)
{
    fp->input.cp.stick_range.x = stick_x;
    fp->input.cp.stick_range.y = stick_y;
}

void ftRemixAiPostSetNullCommand(FTStruct *fp)
{
    ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_NULL);
}

f32 ftRemixAiPostDistanceToTarget(FTStruct *fp, FTStruct *target_fp)
{
    f32 dx = target_fp->coll_data.p_translate->x - fp->coll_data.p_translate->x;
    f32 dy = target_fp->coll_data.p_translate->y - fp->coll_data.p_translate->y;

    return sqrtf((dx * dx) + (dy * dy));
}

void ftRemixAiPostDiveDown(FTStruct *fp)
{
    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        return;
    }
    ftRemixAiPostSetNullCommand(fp);
    ftRemixAiPostSetStick(fp, 0, FTREMIX_AI_POST_STICK_MIN);
}

sb32 ftRemixAiPostIsLandingSafe(FTStruct *fp)
{
    Vec3f probe;

    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        return TRUE;
    }
    probe = *fp->coll_data.p_translate;
    probe.x += ftRemixAiPostLandingProbeOffset(probe.x, fp->computer.target_pos.x);

    return (func_ovl2_800F8FFC(&probe) != FALSE) ? TRUE : FALSE;
}

#endif
