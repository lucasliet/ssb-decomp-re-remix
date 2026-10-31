#include <ft/fighter.h>
#ifdef REGION_REMIX
#include "mn/mnvsmode/mnremixvsstats.h"
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80144660
void ftCommonPassiveSetStatus(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

#ifdef REGION_REMIX
    /* WallTeching.asm track_successful_tech (0x80144670): every passive tech counts as a successful tech */
    mnRemixVSStatsCountTracker(fp->player, nMNRemixVSStatsTrackerTechSuccess);
#endif
    if (fp->ga == nMPKineticsAir)
    {
        mpCommonSetFighterGround(fp);
    }
    ftMainSetStatus(fighter_gobj, nFTCommonStatusPassive, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftParamVelDamageTransferGround(fp);
}

// 0x801446BC
sb32 ftCommonPassiveCheckInterruptDamage(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->tics_since_last_z < FTCOMMON_PASSIVE_BUFFER_TICS_MAX)
    {
        ftCommonPassiveSetStatus(fighter_gobj);

        return TRUE;
    }
    else return FALSE;
}
