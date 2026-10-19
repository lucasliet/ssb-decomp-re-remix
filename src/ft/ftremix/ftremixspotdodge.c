/*
 * ftremixspotdodge.c - Spot Dodging setting of Smash Remix (.toolchain/pkgs/smashremix-src/src/SpotDodge.asm).
 * ftCommonGuardPassCheckInterruptGuard calls ftRemixSpotDodgeCheckInterrupt first, then takes the pass input from
 * ftRemixSpotDodgePassInput. Setting values (Toggles.asm entry_spot_dodge): 0 Off, 1 On.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftmain.h>
#include <gm/gmremixtoggle.h>
#include <mp/mpcommon.h>
#include <mp/mpdef.h>
#include "ft/ftremix/ftremixhooks.h"

/* SpotDodge.asm check_input: a slam is stick y below -32 with fewer than 4 tics of tap_stick_y, and on a soft platform
 * the stick must pass -52 for a spot dodge (above -52 it is a platform drop) */
#define FTREMIX_SPOTDODGE_SLAM_STICK_Y -32
#define FTREMIX_SPOTDODGE_SLAM_TICS 4
#define FTREMIX_SPOTDODGE_SOFT_PLATFORM_STICK_Y -52

/* SpotDodge.asm moveset: WAIT 2, intangible, WAIT 16, intangible off, END */
static const u32 sFTRemixSpotDodgeMoveset[] =
{
    0x04000002,
    0x74000003,
    0x04000010,
    0x74000001,
    0x00000000,
};

/* SpotDodge.asm check_input: a slam with at most 3 tics since it began */
static sb32 ftRemixSpotDodgeIsSlam(FTStruct *fp)
{
    return (fp->input.pl.stick_range.y < FTREMIX_SPOTDODGE_SLAM_STICK_Y) && (fp->tap_stick_y < FTREMIX_SPOTDODGE_SLAM_TICS);
}

static sb32 ftRemixSpotDodgeOnSoftPlatform(FTStruct *fp)
{
    return (fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS) != 0;
}

/* SpotDodge.asm check_input: a slam that starts a spot dodge, off a soft platform or below -52 on one */
static sb32 ftRemixSpotDodgeIsSpotDodgeSlam(FTStruct *fp)
{
    if (ftRemixSpotDodgeIsSlam(fp) == FALSE)
    {
        return FALSE;
    }
    if (ftRemixSpotDodgeOnSoftPlatform(fp) == FALSE)
    {
        return TRUE;
    }
    return fp->input.pl.stick_range.y < FTREMIX_SPOTDODGE_SOFT_PLATFORM_STICK_Y;
}

/* SpotDodge.asm _plat_drop_check_modified: a slam on a soft platform above -52 is a platform drop, the vanilla pass */
static sb32 ftRemixSpotDodgeIsPlatformDrop(FTStruct *fp)
{
    if (ftRemixSpotDodgeIsSlam(fp) == FALSE)
    {
        return FALSE;
    }
    if (ftRemixSpotDodgeOnSoftPlatform(fp) == FALSE)
    {
        return FALSE;
    }
    return fp->input.pl.stick_range.y >= FTREMIX_SPOTDODGE_SOFT_PLATFORM_STICK_Y;
}

/* SpotDodge.asm _spot_dodge: DamageHigh2 with the spot dodge moveset and the dash attack collision routine */
static void ftRemixSpotDodgeStart(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTCommonStatusDamageHi2, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);

    fp->motion_scripts[0][0].p_script = (u32 *) sFTRemixSpotDodgeMoveset;
    fp->motion_scripts[1][0].p_script = (u32 *) sFTRemixSpotDodgeMoveset;
    fp->proc_interrupt = NULL;
    fp->proc_map = mpCommonSetFighterFallOnEdgeBreak;
}

/* SpotDodge.asm check_input: TRUE when a spot dodge started, the caller then skips its pass check */
sb32 ftRemixSpotDodgeCheckInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (gmRemixToggle(spot_dodge) == FALSE)
    {
        return FALSE;
    }
    if (ftRemixSpotDodgeIsSpotDodgeSlam(fp) == FALSE)
    {
        return FALSE;
    }
    if ((fp->input.pl.button_hold & fp->input.button_mask_z) == 0)
    {
        return FALSE;
    }
    ftRemixSpotDodgeStart(fighter_gobj);

    return TRUE;
}

/* SpotDodge.asm check_input: the pass input of ftCommonGuardPassCheckInterruptGuard, vanilla_pass when the setting is off */
sb32 ftRemixSpotDodgePassInput(FTStruct *fp, sb32 vanilla_pass)
{
    if (gmRemixToggle(spot_dodge) == FALSE)
    {
        return vanilla_pass;
    }
    return ftRemixSpotDodgeIsPlatformDrop(fp);
}

#endif
