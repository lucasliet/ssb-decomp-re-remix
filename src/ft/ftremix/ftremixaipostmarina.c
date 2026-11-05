#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sys/utils.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostmarina.h"
#include "ft/ftremix/ftremixaipostrules.h"
#include "ft/ftremix/ftremixfields.h"

/* Marina's DSP charge level in passive_vars (N64 0x0ADC, MarinaDSP). */
#define FTREMIX_AI_POST_MARINA_N64_CHARGE 0x0ADC

/** Charge level of Marina's down special. */
static s32 ftRemixAiPostMarinaGetCharge(FTStruct *fp)
{
    return *ftRemixFighterWord(fp, FTREMIX_AI_POST_MARINA_N64_CHARGE);
}

/** _up of jet_snatch: the throw goes up, so the stick is held up with no sideways input. */
static void ftRemixAiPostMarinaThrowUp(FTStruct *fp)
{
    ftRemixAiPostSetNullCommand(fp);
    ftRemixAiPostSetStick(fp, 0, FTREMIX_AI_POST_STICK_MAX);
}

/** _back of jet_snatch: the throw goes away from the target. Without a target only the command is cleared. */
static void ftRemixAiPostMarinaThrowBack(FTStruct *fp)
{
    FTStruct *target_fp;

    ftRemixAiPostSetNullCommand(fp);
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    ftRemixAiPostSetStick(fp, ftRemixAiPostMarinaBackStickX(fp->coll_data.p_translate->x, target_fp->coll_data.p_translate->x), 0);
}

/** jet_snatch: a random choice between a forward throw (no change), a back throw and an up throw. */
static void ftRemixAiPostMarinaChooseSnatchThrow(FTStruct *fp)
{
    s32 choice = ftRemixAiPostMarinaSnatchChoice(syUtilsRandIntRange(FTREMIX_AI_POST_MARINA_SNATCH_ROLL_RANGE));

    if (choice == FTREMIX_AI_POST_MARINA_SNATCH_UP)
    {
        ftRemixAiPostMarinaThrowUp(fp);
        return;
    }
    if (choice == FTREMIX_AI_POST_MARINA_SNATCH_BACK)
    {
        ftRemixAiPostMarinaThrowBack(fp);
    }
}

/** cargo: when the carried fighter's breakout timer reaches 0x10 the CPU presses NSP to shake it. */
static void ftRemixAiPostMarinaShakeCargo(FTStruct *fp)
{
    FTStruct *target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (ftRemixAiPostMarinaCargoShakes(target_fp->breakout_wait))
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_POST_INPUT_NSP);
    }
}

/** dsp_check: with a charge and the target far away, a 1 in (500 / charge) roll starts the double DSP. */
static void ftRemixAiPostMarinaCheckChargedDsp(FTStruct *fp, s32 charge)
{
    FTStruct *target_fp;

    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        return;
    }
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (ftRemixAiPostMarinaDspInDistance(ftRemixAiPostDistanceToTarget(fp, target_fp)) == FALSE)
    {
        return;
    }
    if (syUtilsRandIntRange(ftRemixAiPostMarinaDspRollRange(charge)) == 0)
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_POST_ROUTINE_MARINA_DOUBLE_DSP);
    }
}

/** item_absorb_check: holding an item over ground, a 1 in 600 roll starts the DSP to absorb it. */
static void ftRemixAiPostMarinaCheckItemAbsorb(FTStruct *fp)
{
    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        return;
    }
    if (syUtilsRandIntRange(FTREMIX_AI_POST_MARINA_ITEM_ABSORB_ROLL_RANGE) == 0)
    {
        ftComputerSetCommandImmediate(fp, FTREMIX_AI_POST_INPUT_DSP);
    }
}

void ftRemixAiPostProcessMarina(FTStruct *fp)
{
    s32 charge;

    if (ftRemixAiPostIsMarinaPull(fp->status_id))
    {
        ftRemixAiPostMarinaChooseSnatchThrow(fp);
        return;
    }
    if (ftRemixAiPostIsMarinaCargo(fp->status_id))
    {
        ftRemixAiPostMarinaShakeCargo(fp);
        return;
    }
    charge = ftRemixAiPostMarinaGetCharge(fp);

    if (charge != 0)
    {
        ftRemixAiPostMarinaCheckChargedDsp(fp, charge);
        return;
    }
    if (fp->item_gobj != NULL)
    {
        ftRemixAiPostMarinaCheckItemAbsorb(fp);
    }
}

#endif
