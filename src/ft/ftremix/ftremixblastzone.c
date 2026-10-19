/*
 * ftremixblastzone.c - BlastZone Warp (BETA) setting of Smash Remix (Toggles.asm entry_blastzone_warp, Blastzone.asm).
 *
 * The Remix hooks replace the KO call of ftCommonDeadCheckInterruptCommon with a warp to the opposite map bound. LEFT/RIGHT
 * is bit 1 of the toggle value and TOP/BOTTOM is bit 2. A warped player keeps its state: grabs and ledges are released, a
 * grounded player becomes aerial, and the hitbox interpolation is reset. The bottom warp also handles the actions that
 * would get stuck at the bottom bound. Every entry point returns FALSE while its bit is off, so vanilla KO runs.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftmain.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <mp/map.h>
#include <mp/mpdef.h>
#include <sys/obj.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_BLASTZONE_WARP_LEFT_RIGHT 1             /* Toggles.asm entry_blastzone_warp: LEFT/RIGHT */
#define FTREMIX_BLASTZONE_WARP_TOP_BOTTOM 2             /* Toggles.asm entry_blastzone_warp: TOP/BOTTOM */
#define FTREMIX_BLASTZONE_STUCK_FRAMES 250              /* Blastzone.asm loop_screen_bottom_: sltiu at, at, 250 */

#define FTREMIX_BLASTZONE_ACTION_FALL 0x1A              /* Action.Fall */
#define FTREMIX_BLASTZONE_ACTION_CLIFF_CATCH 0x54       /* Action.CliffCatch */
#define FTREMIX_BLASTZONE_ACTION_CLIFF_ESCAPE_SLOW2 0x63 /* Action.CliffEscapeSlow2 */
#define FTREMIX_BLASTZONE_ACTION_SHIELD_BREAK_FALL 0x9F /* Action.ShieldBreakFall */
#define FTREMIX_BLASTZONE_ACTION_GROUND_POUND_DROP 0xE3 /* Action.YOSHI.GroundPoundDrop, Bowser.Action.BowserBombDrop */
#define FTREMIX_BLASTZONE_ACTION_BOWSER_FORWARD_THROW2 0xE6 /* Bowser.Action.BowserForwardThrow2 */
#define FTREMIX_BLASTZONE_ACTION_KIRBY_FORWARD_THROW_FALL 0xE5 /* Action.KIRBY.ForwardThrowFall */

#define FTREMIX_BLASTZONE_STUCK_NONE 0
#define FTREMIX_BLASTZONE_STUCK_FRAME_CHECK 1
#define FTREMIX_BLASTZONE_STUCK_GRAB_RELEASE 2

/** Returns TRUE when the LEFT/RIGHT bit of the BlastZone Warp setting is on. */
static sb32 ftRemixBlastZoneWarpsLeftRight(void)
{
    return (gmRemixToggle(blastzone_warp) & FTREMIX_BLASTZONE_WARP_LEFT_RIGHT) != 0;
}

/** Returns TRUE when the TOP/BOTTOM bit of the BlastZone Warp setting is on. */
static sb32 ftRemixBlastZoneWarpsTopBottom(void)
{
    return (gmRemixToggle(blastzone_warp) & FTREMIX_BLASTZONE_WARP_TOP_BOTTOM) != 0;
}

/** Makes a player hanging from a ledge (CliffCatch to CliffEscapeSlow2) fall, as Blastzone.asm grab_release_check_ does. */
static void ftRemixBlastZoneLedgeRelease(FTStruct *fp)
{
    s32 action = fp->status_id;

    if ((action < FTREMIX_BLASTZONE_ACTION_CLIFF_CATCH) || (action >= FTREMIX_BLASTZONE_ACTION_CLIFF_ESCAPE_SLOW2))
    {
        return;
    }
    ftCommonFallSetStatus(fp->fighter_gobj);
}

/**
 * Releases the grab of the player, or the capture of the player that holds it, and then the ledge
 * (Blastzone.asm grab_release_check_).
 */
static void ftRemixBlastZoneGrabReleaseCheck(FTStruct *fp)
{
    if (fp->catch_gobj != NULL)
    {
        ftCommonCatchCaptureSetStatusRelease(fp->fighter_gobj);
        return;
    }
    if (fp->capture_gobj != NULL)
    {
        ftCommonCatchCaptureSetStatusRelease(fp->capture_gobj);
        return;
    }
    ftRemixBlastZoneLedgeRelease(fp);
}

/** Runs the player collision routine (FTStruct.proc_map) to make a grounded player aerial (Blastzone.asm set_aerial_from_grounded). */
static void ftRemixBlastZoneSetAerialFromGrounded(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->proc_map != NULL)
    {
        fp->proc_map(fighter_gobj);
    }
}

/** Grab release, then the grounded to aerial step, shared by the warps (Blastzone.asm loop_screen_* hooks). */
static void ftRemixBlastZoneReleaseAndAerialize(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixBlastZoneGrabReleaseCheck(fp);
    if (fp->ga == nMPKineticsGround)
    {
        ftRemixBlastZoneSetAerialFromGrounded(fighter_gobj);
    }
}

/**
 * Keeps the active hitboxes active, so they do not interpolate across the screen during the warp
 * (Blastzone.asm reset_hitbox_interpolation).
 */
static void ftRemixBlastZoneResetHitboxInterpolation(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 i;

    for (i = 0; i < ARRAY_COUNT(fp->attack_colls); i++)
    {
        if (fp->attack_colls[i].attack_state != 0)
        {
            fp->attack_colls[i].attack_state = 1;
        }
    }
}

/** The release and reset that every warp applies after it moves the player. */
static void ftRemixBlastZoneSettle(GObj *fighter_gobj)
{
    ftRemixBlastZoneReleaseAndAerialize(fighter_gobj);
    ftRemixBlastZoneResetHitboxInterpolation(fighter_gobj);
}

/** Writes the warped x to the player position and to the root DObj translate. */
static void ftRemixBlastZoneSetX(GObj *fighter_gobj, Vec3f *pos, f32 x)
{
    pos->x = x;
    DObjGetStruct(fighter_gobj)->translate.vec.f.x = x;
}

/** Writes the warped y to the player position and to the root DObj translate. */
static void ftRemixBlastZoneSetY(GObj *fighter_gobj, Vec3f *pos, f32 y)
{
    pos->y = y;
    DObjGetStruct(fighter_gobj)->translate.vec.f.y = y;
}

/**
 * Classifies the bottom action that would get the player stuck: Yoshi and Bowser ground pound drops, Bowser forward
 * throws, Kirby forward throw falls and shield break falls (Blastzone.asm loop_screen_bottom_).
 */
static s32 ftRemixBlastZoneStuckKind(s32 fkind, s32 action)
{
    sb32 is_bowser = (fkind == REMIX_ID_BOWSER) || (fkind == REMIX_ID_GBOWSER);

    if ((action == FTREMIX_BLASTZONE_ACTION_GROUND_POUND_DROP) && ((fkind == REMIX_ID_YOSHI) || is_bowser))
    {
        return FTREMIX_BLASTZONE_STUCK_FRAME_CHECK;
    }
    if ((action == FTREMIX_BLASTZONE_ACTION_BOWSER_FORWARD_THROW2) && is_bowser)
    {
        return FTREMIX_BLASTZONE_STUCK_GRAB_RELEASE;
    }
    if (action == FTREMIX_BLASTZONE_ACTION_SHIELD_BREAK_FALL)
    {
        return FTREMIX_BLASTZONE_STUCK_FRAME_CHECK;
    }
    if ((action == FTREMIX_BLASTZONE_ACTION_KIRBY_FORWARD_THROW_FALL) && (fkind == REMIX_ID_KIRBY))
    {
        return FTREMIX_BLASTZONE_STUCK_GRAB_RELEASE;
    }
    return FTREMIX_BLASTZONE_STUCK_NONE;
}

/** After the stuck action has lasted FTREMIX_BLASTZONE_STUCK_FRAMES, the player falls (Blastzone.asm _check_frame). */
static void ftRemixBlastZoneFallWhenStuck(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->status_total_tics < FTREMIX_BLASTZONE_STUCK_FRAMES)
    {
        return;
    }
    ftMainSetStatus(fighter_gobj, FTREMIX_BLASTZONE_ACTION_FALL, 0.0F, 1.0F, 0);
}

/** Applies the release that the bottom warp needs for the player's current action. */
static void ftRemixBlastZoneBottomRelease(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 stuck_kind = ftRemixBlastZoneStuckKind(fp->fkind, fp->status_id);

    if (stuck_kind == FTREMIX_BLASTZONE_STUCK_FRAME_CHECK)
    {
        ftRemixBlastZoneFallWhenStuck(fighter_gobj);
        return;
    }
    if (stuck_kind == FTREMIX_BLASTZONE_STUCK_GRAB_RELEASE)
    {
        ftCommonCatchCaptureSetStatusRelease(fighter_gobj);
        return;
    }
    ftRemixBlastZoneReleaseAndAerialize(fighter_gobj);
}

/**
 * Warps a player that left the right side of the map to the left bound, when LEFT/RIGHT is on. Returns TRUE when the warp
 * replaces the KO, which the caller must then skip.
 */
sb32 ftRemixBlastZoneWarpFromRight(GObj *fighter_gobj, Vec3f *pos)
{
    if (!ftRemixBlastZoneWarpsLeftRight())
    {
        return FALSE;
    }
    ftRemixBlastZoneSetX(fighter_gobj, pos, (f32) gMPCollisionGroundData->map_bound_left);
    ftRemixBlastZoneSettle(fighter_gobj);

    return TRUE;
}

/** Warps a player that left the left side of the map to the right bound, when LEFT/RIGHT is on. See ftRemixBlastZoneWarpFromRight. */
sb32 ftRemixBlastZoneWarpFromLeft(GObj *fighter_gobj, Vec3f *pos)
{
    if (!ftRemixBlastZoneWarpsLeftRight())
    {
        return FALSE;
    }
    ftRemixBlastZoneSetX(fighter_gobj, pos, (f32) gMPCollisionGroundData->map_bound_right);
    ftRemixBlastZoneSettle(fighter_gobj);

    return TRUE;
}

/** Warps a player that left the top of the map to the bottom bound, when TOP/BOTTOM is on. See ftRemixBlastZoneWarpFromRight. */
sb32 ftRemixBlastZoneWarpFromTop(GObj *fighter_gobj, Vec3f *pos)
{
    if (!ftRemixBlastZoneWarpsTopBottom())
    {
        return FALSE;
    }
    ftRemixBlastZoneSetY(fighter_gobj, pos, (f32) gMPCollisionGroundData->map_bound_bottom);
    ftRemixBlastZoneSettle(fighter_gobj);

    return TRUE;
}

/**
 * Warps a player that left the bottom of the map to the top bound, when TOP/BOTTOM is on. The stuck actions are released
 * first (Blastzone.asm loop_screen_bottom_). See ftRemixBlastZoneWarpFromRight.
 */
sb32 ftRemixBlastZoneWarpFromBottom(GObj *fighter_gobj, Vec3f *pos)
{
    if (!ftRemixBlastZoneWarpsTopBottom())
    {
        return FALSE;
    }
    ftRemixBlastZoneSetY(fighter_gobj, pos, (f32) gMPCollisionGroundData->map_bound_top);
    ftRemixBlastZoneBottomRelease(fighter_gobj);
    ftRemixBlastZoneResetHitboxInterpolation(fighter_gobj);

    return TRUE;
}

#endif
