/**
 * ftremixcrashshared.c - Crash and N Crash hooks into the common statuses (Crash.asm dair_bounce_ and crash_eat_sfx).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/crash/ftremixcrashshared.h"
#include "remix/remix_roster_data.h"
#include <ft/fighter.h>
#include <ft/ftmain.h>

extern void *func_800269C0_275C0(u16 id);

#define FTREMIX_CRASH_DAIR_BOUNCE_VEL_Y 50.0F
#define FTREMIX_CRASH_DAIR_RESTART_FRAME 35.0F
#define FTREMIX_CRASH_EAT_FGM 0x05BF

sb32 ftRemixCrashAttackAirLwProcHit(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if ((fp->fkind != REMIX_ID_CRASH) && (fp->fkind != REMIX_ID_NCRASH))
    {
        return FALSE;
    }
    fp->is_fastfall = FALSE;
    fp->physics.vel_air.y = FTREMIX_CRASH_DAIR_BOUNCE_VEL_Y;

    ftMainSetStatus(fighter_gobj, nFTCommonStatusAttackAirLw, FTREMIX_CRASH_DAIR_RESTART_FRAME, 1.0F, FTSTATUS_PRESERVE_NONE);
    return TRUE;
}

void ftRemixCrashEatSfx(FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_CRASH)
    {
        return;
    }
    func_800269C0_275C0(FTREMIX_CRASH_EAT_FGM);
}

#endif
