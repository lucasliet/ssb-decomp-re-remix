/**
 * gmremixbgmswap.c - Final Destination intro and Hurry Up song swaps of the Remix, see gmremixbgmswap.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/ftparam.h>
#include <mp/map.h>
#include <sc/scdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/audio.h>
#include <sys/taskman.h>
#include "gm/gmremixbgmrules.h"
#include "gm/gmremixbgmstagerules.h"
#include "gm/gmremixbgmswap.h"
#include "gm/gmremixbgmswaprules.h"
#include "sc/sc1pmode/sc1premixmode.h"

/* Update count when the last song started (FD.asm music_fix_.timer_start), and the song that was playing then. */
static u32 sGMRemixBgmSwapTimerStart = 0;
static s32 sGMRemixBgmSwapPlayingTrack = -1;

void gmRemixBgmSwapNoteStageSongPlayed(void)
{
    sGMRemixBgmSwapTimerStart = dSYTaskmanUpdateCount;
    sGMRemixBgmSwapPlayingTrack = syAudioGetBGMPlaying(0);
}

/**
 * Restarts the count when the playing song is not the one the count started with (an item song began or ended).
 */
static void gmRemixBgmSwapFollowPlayingTrack(void)
{
    s32 playing_track = syAudioGetBGMPlaying(0);

    if (playing_track != sGMRemixBgmSwapPlayingTrack)
    {
        sGMRemixBgmSwapPlayingTrack = playing_track;
        sGMRemixBgmSwapTimerStart = dSYTaskmanUpdateCount;
    }
}

/**
 * Whether the match runs on a stage whose song starts with the Final Destination intro.
 *
 * @return TRUE on the VS Final Destination stages and on the 1P Final Stage of the standard and Remix 1P runs.
 */
static sb32 gmRemixBgmSwapIsFinalDestinationMatch(void)
{
    if (gmRemixBgmSwapRulesIsFinalDestination(gSCManagerBattleState->gkind) != 0)
    {
        return TRUE;
    }
    if (gSCManagerSceneData.scene_curr != nSCKind1PGame)
    {
        return FALSE;
    }
    return (gmRemixBgmSwapRulesIs1PFinalStage(sc1PRemixModeGet(), gSCManagerSceneData.spgame_stage) != 0) ? TRUE : FALSE;
}

void gmRemixBgmSwapFdIntroUpdate(void)
{
    if (gmRemixBgmSwapIsFinalDestinationMatch() == FALSE)
    {
        return;
    }
    if (dSYTaskmanUpdateCount == 0)
    {
        sGMRemixBgmSwapTimerStart = 0;
    }
    gmRemixBgmSwapFollowPlayingTrack();

    if (gmRemixBgmSwapRulesFdIntroEnds(dSYTaskmanUpdateCount, sGMRemixBgmSwapTimerStart, gMPCollisionBGMDefault) == 0)
    {
        return;
    }
    gMPCollisionBGMCurrent = gMPCollisionBGMDefault = GMREMIX_BGM_STAGE_FD_MAIN;
    syAudioPlayBGM(0, GMREMIX_BGM_STAGE_FD_MAIN);
    gmRemixBgmSwapNoteStageSongPlayed();
}

void gmRemixBgmSwapHurryUpdate(void)
{
    s32 hurry_track = gmRemixBgmSwapRulesHurryTrack(gMPCollisionBGMCurrent, gMPCollisionBGMDefault);

    if (hurry_track == GMREMIX_BGM_NO_TRACK)
    {
        return;
    }
    gMPCollisionBGMDefault = hurry_track;
    ftParamTryUpdateItemMusic();
}

#endif
