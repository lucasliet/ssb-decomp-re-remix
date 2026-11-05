#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/mpcollision.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostedge.h"
#include "ft/ftremix/ftremixaipostrules.h"

/** Cancels the current command when the move it started would end off stage ("clear inputs to avoid SDing"). */
static void ftRemixAiPostCancelUnsafeLanding(FTStruct *fp)
{
    if (ftRemixAiPostIsLandingSafe(fp) == FALSE)
    {
        ftRemixAiPostSetNullCommand(fp);
    }
}

/**
 * usp_control (Wolf/AI/Attacks.asm): points the stick at the target, then sets only its Y by the target height. Over no
 * ground the stick goes up.
 */
static void ftRemixAiPostWolfSteerFlash(FTStruct *fp)
{
    f32 target_y_diff;

    ftComputerSetCommandImmediate(fp, FTREMIX_AI_POST_ROUTINE_POINT_STICK_TO_TARGET);

    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        fp->input.cp.stick_range.y = FTREMIX_AI_POST_STICK_MAX;
        return;
    }
    target_y_diff = fp->computer.target_pos.y - fp->coll_data.p_translate->y;
    fp->input.cp.stick_range.y = ftRemixAiPostWolfStickY(target_y_diff);
}

/**
 * ground_check of dkshared.asm: the probe sits 400 units along the horizontal speed. A fighter with no ground below or no
 * horizontal speed is safe.
 */
static sb32 ftRemixAiPostDkIsSpinSafe(FTStruct *fp)
{
    Vec3f probe;
    f32 probe_offset;

    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        return TRUE;
    }
    probe_offset = ftRemixAiPostDkProbeOffset(fp->physics.vel_air.x);

    if (probe_offset == 0.0F)
    {
        return TRUE;
    }
    probe = *fp->coll_data.p_translate;
    probe.x += probe_offset;

    return (func_ovl2_800F8FFC(&probe) != FALSE) ? TRUE : FALSE;
}

/** _cancel_velocity of dkshared.asm: no command, no vertical stick, the horizontal stick against the speed. */
static void ftRemixAiPostDkTurnBack(FTStruct *fp)
{
    ftRemixAiPostSetNullCommand(fp);
    ftRemixAiPostSetStick(fp, ftRemixAiPostDkHoldStickX(fp->physics.vel_air.x), 0);
}

void ftRemixAiPostProcessFalco(FTStruct *fp)
{
    if (ftRemixAiPostIsFoxFamilyUsp(fp->status_id))
    {
        ftRemixAiPostDiveDown(fp);
        return;
    }
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_INPUT_NSP))
    {
        ftRemixAiPostCancelUnsafeLanding(fp);
    }
}

void ftRemixAiPostProcessWario(FTStruct *fp)
{
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_ROUTINE_WARIO_NSP_TOWARDS) ||
        ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_ROUTINE_WARIO_NSP_TOWARDS_JUMP))
    {
        ftRemixAiPostCancelUnsafeLanding(fp);
    }
}

void ftRemixAiPostProcessWolf(FTStruct *fp)
{
    if (ftRemixAiPostIsWolfFlashStart(fp->status_id))
    {
        ftRemixAiPostWolfSteerFlash(fp);
        return;
    }
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_INPUT_NSP))
    {
        ftRemixAiPostCancelUnsafeLanding(fp);
    }
}

void ftRemixAiPostProcessDonkeyKong(FTStruct *fp)
{
    if (ftRemixAiIsLevelTen(fp->level) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostIsHandSlap(fp->status_id))
    {
        ftRemixAiPostSetNullCommand(fp);
        return;
    }
    if (ftRemixAiPostIsSpinningKong(fp->status_id) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostDkIsSpinSafe(fp) == FALSE)
    {
        ftRemixAiPostDkTurnBack(fp);
    }
}

#endif
