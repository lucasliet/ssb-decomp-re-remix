#include <ft/fighter.h>
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixhooks.h"
#endif
#if defined(PORT) && defined(REGION_REMIX)
#include "if/ifremixannouncer.h"
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80149940
void ftCommonFuraSleepProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 breakout_wait;

    fp->breakout_wait--;

    breakout_wait = fp->breakout_wait;

    ftCommonCaptureTrappedUpdateBreakoutVars(fp);

    fp->breakout_wait += (fp->breakout_wait - breakout_wait) * 3;

    if (fp->breakout_wait <= 0)
    {
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

// 0x801499A4
void ftCommonFuraSleepSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 breakout_wait;

#if defined(PORT) && defined(REGION_REMIX)
    if (ifRemixAnnouncerIsActive())
    {
        ifRemixAnnouncerRequestSleep();
    }
#endif
    ftMainSetStatus(fighter_gobj, nFTCommonStatusFuraSleep, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    breakout_wait = FTCOMMON_FURASLEEP_BREAKOUT_WAIT_DEFAULT - fp->percent_damage;

    if (breakout_wait <= 0)
    {
        breakout_wait = 0;
    }
    breakout_wait += FTCOMMON_FURASLEEP_BREAKOUT_WAIT_MIN;

#ifdef REGION_REMIX
    breakout_wait = ftRemixFuraSleepBreakoutWait(breakout_wait);
#endif
    ftCommonCaptureTrappedInitBreakoutVars(fp, breakout_wait);
    ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterFuraSleep, 0);
}
