/**
 * gmremixbgm.c - runtime BGM state of the Remix, see gmremixbgm.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/os.h>
#include <mp/map.h>
#include <sys/audio.h>
#include <sc/scmanager.h>
#include "gm/gmremixbgm.h"
#include "gm/gmremixbgmrules.h"
#include "gm/gmremixtoggle.h"
#include "gr/grremix/grremixrandommusic.h"
#include "gr/grremix/grremixstage.h"

#include <port_log.h>

extern void *func_800269C0_275C0(u16 id);

/* FGM.asm menu sound played by the pause song change */
#define GMREMIX_BGM_FGM_CHANGE 0x9E
/* Pause.asm dpad_song_cycle_timer: frames before the first change, and before each repeat */
#define GMREMIX_BGM_CYCLE_INITIAL_DELAY 2
#define GMREMIX_BGM_CYCLE_REPEAT_DELAY 30

static s32 sGMRemixBgmGkind;
static u32 sGMRemixBgmDefaultTrack;
static u32 sGMRemixBgmCurrentTrack;
static s32 sGMRemixBgmCycleTimer = GMREMIX_BGM_CYCLE_INITIAL_DELAY;

void gmRemixBgmSetStageTracks(s32 gkind, u32 default_track, u32 current_track)
{
    sGMRemixBgmGkind = gkind;
    sGMRemixBgmDefaultTrack = default_track;
    sGMRemixBgmCurrentTrack = current_track;
    sGMRemixBgmCycleTimer = GMREMIX_BGM_CYCLE_INITIAL_DELAY;
}

sb32 gmRemixBgmChange(u32 track)
{
    u32 previous_stage_track = gMPCollisionBGMDefault;

    sGMRemixBgmCurrentTrack = track;
    gMPCollisionBGMCurrent = track;
    gMPCollisionBGMDefault = track;

    if (previous_stage_track == track)
    {
        return FALSE;
    }
    gMPCollisionGroundData->bgm_id = track;
    port_log("remix bgm: track %u (stage default %u)\n", (unsigned)track, (unsigned)sGMRemixBgmDefaultTrack);
    syAudioPlayBGM(0, track);
    return TRUE;
}

/** Track a pause D-pad press selects: the next alternate for Right, a random one for Down. */
static u32 gmRemixBgmPickPauseTrack(u16 button_hold)
{
    s32 alternates[GMREMIX_BGM_ALT_SLOTS];

    if (button_hold & R_JPAD)
    {
        grRemixGetStageAlternateTracks(sGMRemixBgmGkind, alternates);
        return gmRemixBgmRulesNextAlternate(sGMRemixBgmCurrentTrack, sGMRemixBgmDefaultTrack, alternates);
    }
    return grRemixRandomMusicPickForced(sGMRemixBgmDefaultTrack);
}

void gmRemixBgmPauseUpdate(u16 button_hold)
{
    u32 track;

    if (!gmRemixToggle(play_music))
    {
        return;
    }
    if (!(button_hold & (R_JPAD | D_JPAD)))
    {
        sGMRemixBgmCycleTimer = GMREMIX_BGM_CYCLE_INITIAL_DELAY;
        return;
    }
    if (gmRemixBgmRulesIsProtected(gMPCollisionBGMCurrent))
    {
        return;
    }
    sGMRemixBgmCycleTimer--;
    if (sGMRemixBgmCycleTimer != 0)
    {
        return;
    }
    track = gmRemixBgmPickPauseTrack(button_hold);
    if (gmRemixBgmChange(track))
    {
        func_800269C0_275C0(GMREMIX_BGM_FGM_CHANGE);
        sGMRemixBgmCycleTimer = GMREMIX_BGM_CYCLE_REPEAT_DELAY;
        return;
    }
    sGMRemixBgmCycleTimer = GMREMIX_BGM_CYCLE_INITIAL_DELAY;
}

void gmRemixBgmFadeOut(s32 frames)
{
    if (gmRemixBgmRulesIsProtected(gMPCollisionBGMCurrent))
    {
        return;
    }
    syAudioSetBGMVolumeFade(0, 0, frames);
}

#endif
