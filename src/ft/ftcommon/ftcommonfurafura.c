#include <ft/fighter.h>
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixhooks.h"
#include "if/ifremixannouncer.h"
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80149810
void ftCommonFuraFuraProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 breakout_wait;

    fp->shield_health = (fp->fkind == nFTKindYoshi) ? 30 : 30;

    fp->breakout_wait--;

    breakout_wait = fp->breakout_wait;

    ftCommonCaptureTrappedUpdateBreakoutVars(fp);

    fp->breakout_wait += (fp->breakout_wait - breakout_wait) * 3;

    if (fp->breakout_wait <= 0)
    {
        ftCommonWaitSetStatus(fighter_gobj);
    }
}

// 0x801498A4
void ftCommonFuraFuraSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 breakout_wait;

#ifdef REGION_REMIX
    if (ifRemixAnnouncerIsActive())
    {
        ifRemixAnnouncerPlayCall(nIFRemixAnnouncerCallStun);
    }
#endif
    ftMainSetStatus(fighter_gobj, nFTCommonStatusFuraFura, 0.0F, 1.0F, (FTSTATUS_PRESERVE_TEXTUREPART | FTSTATUS_PRESERVE_MODELPART));

    fp->shield_health = (fp->fkind == nFTKindYoshi) ? 30 : 30;

    breakout_wait = FTCOMMON_FURAFURA_BREAKOUT_WAIT_DEFAULT - fp->percent_damage;

    if (breakout_wait <= 0)
    {
        breakout_wait = 0;
    }
    breakout_wait += FTCOMMON_FURAFURA_BREAKOUT_WAIT_MIN;

#ifdef REGION_REMIX
    breakout_wait = ftRemixFuraFuraBreakoutWait(breakout_wait);
#endif
    ftCommonCaptureTrappedInitBreakoutVars(fp, breakout_wait);
    ftParamCheckSetFighterColAnimID(fighter_gobj, nGMColAnimFighterFuraFura, 0);
}
