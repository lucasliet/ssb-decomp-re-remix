/*
 * ftremixwalltech.c - Wall Teching setting of Smash Remix (.toolchain/pkgs/smashremix-src/src/WallTeching.asm).
 * ftCommonWallDamageSetStatus calls ftRemixWallTechTryStart in place of its wall damage status. The tech is the
 * Passive status with the ftAnimEndSetFall update and the cliff, floor and ceiling map routine.
 * Setting values (Toggles.asm entry_wall_teching): 0 Off, 1 On.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftanimend.h>
#include <ft/ftcommon.h>
#include <ft/ftmain.h>
#include <gm/gmremixtoggle.h>
#include <mp/mpcommon.h>
#include "ft/ftremix/ftremixhooks.h"
#include "mn/mnvsmode/mnremixvsstats.h"

/* WallTeching.asm 0x80141C04 hook: the update, physics and map routines of the tech, the interrupt routine is removed */
static void ftRemixWallTechSetRoutines(FTStruct *fp)
{
    fp->proc_update = ftAnimEndSetFall;
    fp->proc_physics = NULL;
    fp->proc_map = mpCommonProcFighterCliffFloorCeil;
    fp->proc_interrupt = NULL;
}

/* WallTeching.asm: the wall damage becomes a wall tech when Z was pressed within the tech buffer. Returns FALSE when the
 * setting is off or the tech was missed, and the caller then starts the wall damage status as vanilla does. */
sb32 ftRemixWallTechTryStart(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (gmRemixToggle(wall_teching) == FALSE)
    {
        return FALSE;
    }
    if (fp->tics_since_last_z >= FTCOMMON_PASSIVE_BUFFER_TICS_MAX)
    {
        mnRemixVSStatsCountTracker(fp->player, nMNRemixVSStatsTrackerTechMiss);
        return FALSE;
    }
    fp->physics.vel_damage_air.x = 0.0F;
    fp->physics.vel_damage_air.y = 0.0F;

    ftMainSetStatus(fighter_gobj, nFTCommonStatusPassive, 0.0F, 2.0F, FTSTATUS_PRESERVE_NONE);
    ftRemixWallTechSetRoutines(fp);

    mnRemixVSStatsCountTracker(fp->player, nMNRemixVSStatsTrackerTechSuccess);
    return TRUE;
}

#endif
