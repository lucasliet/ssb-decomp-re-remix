/*
 * ftremixaipostsheik.c - Sheik entry of the Remix AI per-fighter post process (AI.asm cpu_post_process,
 * Sheik/AI/Attacks.asm). Pure decisions live in ftremixaipostsheikrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <math.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostsheik.h"
#include "ft/ftremix/ftremixaipostsheikrules.h"

/**
 * Distance between the fighter and the target, computed as the asm does (sqrt of the squared x and y differences).
 */
static f32 ftRemixAiSheikDistanceTo(FTStruct *fp, FTStruct *target_fp)
{
    f32 dx = target_fp->coll_data.p_translate->x - fp->coll_data.p_translate->x;
    f32 dy = target_fp->coll_data.p_translate->y - fp->coll_data.p_translate->y;

    return sqrtf((dx * dx) + (dy * dy));
}

/**
 * teleport_down (Sheik/AI/Attacks.asm): during a USP with ground below, the CPU sets the NULL command and steers the stick
 * down.
 */
static void ftRemixAiSheikTeleportDown(FTStruct *fp)
{
    if (ftRemixAiSheikIsTeleportAction(fp->status_id) == FALSE)
    {
        return;
    }
    if (ftRemixAiSheikTeleportHasGroundBelow(fp->coll_data.floor_line_id) == FALSE)
    {
        return;
    }
    ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_NULL);
    fp->input.cp.stick_range.y = FTREMIX_AI_SHEIK_TELEPORT_STICK_Y;
    fp->input.cp.stick_range.x = 0;
}

/**
 * dsp_control (Sheik/AI/Attacks.asm): during DSP the CPU presses B only when its target is within range, otherwise it
 * sets the NULL command. No target means no change.
 */
static void ftRemixAiSheikDspControl(FTStruct *fp)
{
    FTStruct *target_fp;

    if (fp->computer.target_gobj == NULL)
    {
        return;
    }
    target_fp = ftGetStruct(fp->computer.target_gobj);
    if (ftRemixAiSheikDspIsInRange(ftRemixAiSheikDistanceTo(fp, target_fp)) != FALSE)
    {
        fp->input.cp.button_inputs |= B_BUTTON;
        return;
    }
    ftComputerSetCommandImmediate(fp, FTREMIX_AI_LV10_ROUTINE_NULL);
}

void ftRemixAiPostProcessSheik(FTStruct *fp)
{
    if (ftRemixAiSheikIsDspAction(fp->status_id) != FALSE)
    {
        ftRemixAiSheikDspControl(fp);
        return;
    }
    ftRemixAiSheikTeleportDown(fp);
}

#endif
