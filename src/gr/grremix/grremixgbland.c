/**
 * grremixgbland.c - Game Boy Land music and startup beep (Hazards.asm gbland_setup, gb_music_). See grremixgbland.h.
 */

#ifdef PORT

#include "grremixgbland.h"
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sys/audio.h>
#include <mp/map.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <port_log.h>

/* Hazards.asm gbland_setup creates object 0x03F2 (nGCCommonKindGround) in group 1 (nGCCommonLinkIDGround). */
#define GRREMIX_GBLAND_OBJECT_KIND nGCCommonKindGround
#define GRREMIX_GBLAND_LINK_ID nGCCommonLinkIDGround
/* Hazards.asm gbland_setup registers gb_music_ with priority 4. */
#define GRREMIX_GBLAND_PROC_PRIORITY 4
/* Hazards.asm gb_music_: FGM 0x03D2 is the Game Boy startup sound. */
#define GRREMIX_GBLAND_FGM_BEEP 0x03D2
/* Frame counter values of gb_music_ (its timer is the static state below, Remix RAM 0x801313F0 + 0x60). */
#define GRREMIX_GBLAND_TIMER_BEEP 0x00B4
#define GRREMIX_GBLAND_TIMER_SONG_START 0x0188
#define GRREMIX_GBLAND_TIMER_LOOP 0x3C20
/* Current songs of the hammer and the star: a new stage song waits until they end (asm _play). */
#define GRREMIX_GBLAND_BGM_HAMMER 0x2D
#define GRREMIX_GBLAND_BGM_STAR 0x2E
/* MIDI ids of the Remix bank (build/remixasm/bass.log, MIDI_<NAME>_ID; docs/remix/audio.md). */
#define GRREMIX_GBLAND_MIDI_MUDA 0x3C
#define GRREMIX_GBLAND_MIDI_GB_MEDLEY 0xA1
#define GRREMIX_GBLAND_MIDI_BUBBLY 0xA2
#define GRREMIX_GBLAND_MIDI_ROAD_TO_CERULEAN 0xA3
#define GRREMIX_GBLAND_MIDI_LEVEL1_WARIO 0xA4
#define GRREMIX_GBLAND_MIDI_MABE 0xA5

/* Same vanilla FGM player declaration as grremixbbh.c. */
extern void *func_800269C0_275C0(u16 id);

/*
 * One timed music cue of gb_music_ _transitions. A beep cue plays the startup
 * sound and stops the music; a song cue starts bgm.
 */
typedef struct GRRemixGBLandCue
{
    u32 timer;
    sb32 is_beep;
    u32 bgm;
} GRRemixGBLandCue;

/* Hazards.asm gb_music_ state at 0x801313F0: the timer at 0x60 and the transition skip flag at 0x04. */
typedef struct GRRemixGBLandState
{
    u32 timer;
    sb32 transition_skip;
} GRRemixGBLandState;

static const GRRemixGBLandCue sGRRemixGBLandCues[] =
{
    { GRREMIX_GBLAND_TIMER_SONG_START, FALSE, GRREMIX_GBLAND_MIDI_MUDA },
    { 0x0CC8, TRUE, 0 },
    { 0x0D40, FALSE, GRREMIX_GBLAND_MIDI_BUBBLY },
    { 0x1880, TRUE, 0 },
    { 0x18F8, FALSE, GRREMIX_GBLAND_MIDI_ROAD_TO_CERULEAN },
    { 0x2439, TRUE, 0 },
    { 0x24B0, FALSE, GRREMIX_GBLAND_MIDI_LEVEL1_WARIO },
    { 0x2FF0, TRUE, 0 },
    { 0x3068, FALSE, GRREMIX_GBLAND_MIDI_MABE },
    { 0x3BA8, TRUE, 0 },
};

static GRRemixGBLandState sGRRemixGBLand;

/**
 * Returns the random music toggle. REMIX-TODO(toggles): Toggles.entry_random_music has no port accessor yet,
 * so the Remix default (CE: off) is returned.
 *
 * @return FALSE, the random music toggle is off.
 */
static sb32 grRemixGBLandRandomMusicIsOn(void)
{
    return FALSE;
}

/**
 * Makes bgm the stage song (asm _play). The stage default is always replaced, but the new song only starts
 * when neither the hammer nor the star is playing.
 *
 * @param bgm MIDI id of the new song.
 */
static void grRemixGBLandPlaySong(u32 bgm)
{
    u32 current_bgm = gMPCollisionBGMCurrent;

    gMPCollisionBGMDefault = bgm;
    if ((current_bgm == GRREMIX_GBLAND_BGM_HAMMER) || (current_bgm == GRREMIX_GBLAND_BGM_STAR))
    {
        return;
    }
    syAudioPlayBGM(0, bgm);
    gMPCollisionBGMCurrent = bgm;
}

/** Plays the Game Boy Medley once per stage load (asm gb_music_ Medley branch). */
static void grRemixGBLandPlayMedleyOnce(void)
{
    if (sGRRemixGBLand.transition_skip)
    {
        return;
    }
    syAudioPlayBGM(0, GRREMIX_GBLAND_MIDI_GB_MEDLEY);
    sGRRemixGBLand.transition_skip = TRUE;
}

/**
 * Plays the stage's default song once, then makes it the current song (asm gb_music_ random branch: BGM.play_
 * followed by _play with the same song).
 *
 * @param default_bgm MIDI id of the stage's default song.
 */
static void grRemixGBLandPlayRandomOnce(u32 default_bgm)
{
    if (sGRRemixGBLand.transition_skip)
    {
        return;
    }
    syAudioPlayBGM(0, default_bgm);
    sGRRemixGBLand.transition_skip = TRUE;
    grRemixGBLandPlaySong(default_bgm);
}

/** Plays the Game Boy startup sound and stops the music (asm _beep, then _mute). */
static void grRemixGBLandBeep(void)
{
    func_800269C0_275C0(GRREMIX_GBLAND_FGM_BEEP);
    syAudioStopBGMAll();
}

/**
 * Runs one transition cue of the table.
 *
 * @param cue Cue whose timer was reached.
 */
static void grRemixGBLandApplyCue(const GRRemixGBLandCue *cue)
{
    if (cue->is_beep)
    {
        grRemixGBLandBeep();
        return;
    }
    grRemixGBLandPlaySong(cue->bgm);
}

/**
 * Applies the transition cue of the timer, if there is one (asm gb_music_ _transitions). The loop timer
 * restarts the frame counter at the song start value.
 *
 * @param timer Frame counter before this frame's increment.
 */
static void grRemixGBLandCueAt(u32 timer)
{
    s32 i;

    if (timer == GRREMIX_GBLAND_TIMER_LOOP)
    {
        sGRRemixGBLand.timer = GRREMIX_GBLAND_TIMER_SONG_START;
        return;
    }
    for (i = 0; i < ARRAY_COUNT(sGRRemixGBLandCues); i++)
    {
        if (sGRRemixGBLandCues[i].timer == timer)
        {
            grRemixGBLandApplyCue(&sGRRemixGBLandCues[i]);
            return;
        }
    }
}

/**
 * Stage music from the song start timer on (asm gb_music_ _skip_beep onwards): the Medley when it is the
 * default song, otherwise the random or transition path.
 *
 * @param timer Frame counter before this frame's increment.
 */
static void grRemixGBLandStageMusic(u32 timer)
{
    u32 default_bgm = gMPCollisionBGMDefault;

    if (default_bgm == GRREMIX_GBLAND_MIDI_GB_MEDLEY)
    {
        grRemixGBLandPlayMedleyOnce();
        return;
    }
    if (grRemixGBLandRandomMusicIsOn())
    {
        grRemixGBLandPlayRandomOnce(default_bgm);
        return;
    }
    grRemixGBLandCueAt(timer);
}

/**
 * Per-frame process of gb_music_: stops the music on the first frame, beeps at the beep timer and then
 * runs the stage music.
 *
 * @param music_gobj Object created by grRemixGBLandSetup.
 */
static void grRemixGBLandMusicProcUpdate(GObj *music_gobj)
{
    u32 timer = sGRRemixGBLand.timer;

    sGRRemixGBLand.timer = timer + 1;
    if (timer == 0)
    {
        syAudioStopBGMAll();
        return;
    }
    if (timer == GRREMIX_GBLAND_TIMER_BEEP)
    {
        grRemixGBLandBeep();
        return;
    }
    if (timer < GRREMIX_GBLAND_TIMER_SONG_START)
    {
        return;
    }
    grRemixGBLandStageMusic(timer);
}

/**
 * Creates the Game Boy Land music object and clears its state (Hazards.asm gbland_setup), skipped in training mode.
 */
void grRemixGBLandSetup(void)
{
    GObj *gobj;

    if (gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode)
    {
        return;
    }
    gobj = gcMakeGObjSPAfter(GRREMIX_GBLAND_OBJECT_KIND, NULL, GRREMIX_GBLAND_LINK_ID, GOBJ_PRIORITY_DEFAULT);
    if (gobj == NULL)
    {
        port_log("[remix-hazard] gbland: music object not created, the GObj pool is full");
        return;
    }
    gcAddGObjProcess(gobj, grRemixGBLandMusicProcUpdate, nGCProcessKindFunc, GRREMIX_GBLAND_PROC_PRIORITY);
    sGRRemixGBLand.timer = 0;
    sGRRemixGBLand.transition_skip = FALSE;
}

#endif
