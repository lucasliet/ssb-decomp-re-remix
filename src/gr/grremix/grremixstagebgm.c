/**
 * grremixstagebgm.c - stage song of a scene that loads a stage (BGM.asm apply_alt_or_random_music_ and alternate_music_).
 *
 * Pure decisions are in gm/gmremixbgmstagerules.c; this file reads the Remix tables, the controllers and the toggles.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixstage.h"
#include "grremixrandommusic.h"
#include "gm/gmremixbgm.h"
#include "gm/gmremixbgmrules.h"
#include "gm/gmremixbgmstagerules.h"
#include "gm/gmremixgameend.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <sc/scene.h>
#include <sys/controller.h>
#include <sys/utils.h>
#include <macros.h>

#define GRREMIX_ALT_MUSIC_NONE 0xFFFF
#define GRREMIX_ALT_MUSIC_ENTRY_SIZE 8
#define GRREMIX_ALT_MUSIC_SLOT_SIZE 2

/* Song the last stage pick chose before Random Music (BGM.asm current_track), kept for a Salty Runback. */
static s32 sGRRemixStageBgmPreviousPick = GMREMIX_BGM_NO_TRACK;

u32 grRemixGetStageDefaultTrack(s32 gkind, u32 default_bgm)
{
    s32 override_track = grRemixGetDefaultMusicTrack(gkind);

    return (override_track != 0) ? (u32)(override_track - 1) : default_bgm;
}

void grRemixGetStageAlternateTracks(s32 gkind, s32 *out_tracks)
{
    s32 slot;

    for (slot = 0; slot < GMREMIX_BGM_ALT_SLOTS; slot++)
    {
        u32 table_addr = REMIX_SYM_Stages__alternate_music_table + (gkind * GRREMIX_ALT_MUSIC_ENTRY_SIZE) +
                         (slot * GRREMIX_ALT_MUSIC_SLOT_SIZE);
        u32 alternate = remix_ram_image_read_half(table_addr);

        out_tracks[slot] = (alternate == GRREMIX_ALT_MUSIC_NONE) ? GMREMIX_BGM_NO_TRACK : (s32)alternate;
    }
}

/**
 * Whether the scene that loads the stage plays the stage song, so the Remix picks apply: the battle scenes, the VS demo and
 * How to Play. The opening movies load a stage too but never play its song.
 *
 * @return TRUE when the pick applies to the current scene.
 */
static sb32 grRemixStageBgmAppliesToScene(void)
{
    switch (gSCManagerSceneData.scene_curr)
    {
    case nSCKindVSBattle:
    case nSCKind1PGame:
    case nSCKind1PBonusStage:
    case nSCKind1PTrainingMode:
    case nSCKindExplain:
    case nSCKindAutoDemo:
        return TRUE;

    default:
        return FALSE;
    }
}

/**
 * Buttons held on any of the four controllers (Joypad.check_buttons_all_).
 *
 * @return The held buttons, ORed.
 */
static u16 grRemixStageBgmReadHeldButtons(void)
{
    u16 held_buttons = 0;
    s32 port;

    for (port = 0; port < ARRAY_COUNT(gSYControllerDevices); port++)
    {
        held_buttons |= gSYControllerDevices[port].button_hold;
    }
    return held_buttons;
}

/**
 * Song a Salty Runback keeps from the previous match (Salty Runback Preserves Song), GMREMIX_BGM_NO_TRACK otherwise.
 *
 * @param salty_runback TRUE when the match is a salty runback.
 */
static s32 grRemixStageBgmGetPreservedTrack(sb32 salty_runback)
{
    if ((gmRemixToggle(preserve_salty_song) == 0) || (salty_runback == FALSE))
    {
        return GMREMIX_BGM_NO_TRACK;
    }
    return sGRRemixStageBgmPreviousPick;
}

u32 grRemixGetStageBGM(s32 gkind, u32 file_bgm)
{
    sb32 salty_runback = gmRemixGameEndIsSaltyRunback();
    s32 alternates[GMREMIX_BGM_ALT_SLOTS];
    s32 forced_slot = gmRemixBgmStageRulesCButtonSlot(grRemixStageBgmReadHeldButtons());
    s32 preserved_track = grRemixStageBgmGetPreservedTrack(salty_runback);
    u32 default_track;
    s32 roll = 0;
    GMRemixBgmStagePick pick;

    if (grRemixStageBgmAppliesToScene() == FALSE)
    {
        return file_bgm;
    }
    if (gSCManagerSceneData.scene_curr == nSCKindAutoDemo)
    {
        file_bgm = gmRemixBgmStageRulesDemoTrack(file_bgm);
    }
    default_track = grRemixGetStageDefaultTrack(gkind, file_bgm);
    grRemixGetStageAlternateTracks(gkind, alternates);

    if (gmRemixBgmStageRulesNeedsRoll(forced_slot, preserved_track))
    {
        roll = syUtilsRandIntRange(GMREMIX_BGM_STAGE_ROLL_RANGE);
    }
    pick = gmRemixBgmStageRulesPick(default_track, alternates, forced_slot, preserved_track, roll);
    sGRRemixStageBgmPreviousPick = pick.track;

    if (pick.random_disabled == 0)
    {
        pick.track = grRemixRandomMusicPick(pick.track, salty_runback);
    }
    gmRemixBgmSetStageTracks(gkind, default_track, pick.track);
    return pick.track;
}

#endif
