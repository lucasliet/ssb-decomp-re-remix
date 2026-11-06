#if defined(PORT) && defined(REGION_REMIX)

#include <macros.h>
#include <ft/fighter.h>
#include <gm/gmremixmenumusic.h>
#include <gm/gmsound.h>
#include <gr/grremix/grremixrandommusic.h>
#include <mn/mncommon/mncongra.h>
#include <sc/scene.h>
#include <sys/audio.h>
#include <sys/controller.h>
#include <sys/taskman.h>
#include <sys/utils.h>
#include <mn/mncommon/mnremixgallery.h>
#include <mn/mncommon/mnremixgalleryaudio.h>
#include <mn/mncommon/mnremixgalleryrules.h>
#include "remix/remix_roster_data.h"

/* Gallery.asm check_sticks_: x >= 40 is right, x < -39 is left, and only player 1 counts */
#define MN_REMIX_GALLERY_STICK_THRESHOLD 40

/* Gallery.asm calls 0x80020B38 (BGM volume) with 0x7000 every frame and after a song starts */
#define MN_REMIX_GALLERY_BGM_VOLUME 0x7000

/* FGM.asm menu.CONFIRM, menu.SELECT_STAGE and menu.SCROLL */
#define MN_REMIX_GALLERY_FGM_CONFIRM 158
#define MN_REMIX_GALLERY_FGM_SELECT_STAGE 159
#define MN_REMIX_GALLERY_FGM_SCROLL 164

static sb32 sMNRemixGalleryIsActive;
static s32 sMNRemixGalleryPage;
static s32 sMNRemixGalleryLastPage;
static s32 sMNRemixGallerySongNumber;
static s32 sMNRemixGalleryIdleMode;
static s32 sMNRemixGalleryIdleIndex;
static s32 sMNRemixGalleryIdleFrames;
static s32 sMNRemixGalleryDrumKit;
static s32 sMNRemixGalleryPreviousScene;
static s32 sMNRemixGalleryIdleOrder[MN_REMIX_GALLERY_ENTRY_COUNT];

static void mnRemixGalleryPlaySong(s32 bgm_id)
{
    syAudioPlayBGM(0, bgm_id);
    syAudioSetBGMVolume(0, MN_REMIX_GALLERY_BGM_VOLUME);
}

/* Gallery.asm initial_ _idle_mode_1: next picture of the shuffled order with a random song of its list. */
static void mnRemixGalleryShowNextShuffledPage(void)
{
    sMNRemixGalleryIdleIndex = mnRemixGalleryRulesNextIdleIndex(sMNRemixGalleryIdleIndex);
    sMNRemixGalleryPage = sMNRemixGalleryIdleOrder[sMNRemixGalleryIdleIndex];
    sMNRemixGallerySongNumber = syUtilsRandIntRange(mnRemixGalleryAudioGetSongCount(sMNRemixGalleryPage)) + 1;

    mnRemixGalleryPlaySong(mnRemixGalleryAudioGetSong(sMNRemixGalleryPage, sMNRemixGallerySongNumber));
}

/* Gallery.asm initial_ _idle_mode_2: a random song of the Random Music list with the picture that matches it. */
static void mnRemixGalleryShowRandomSongPage(void)
{
    s32 bgm_id = (s32)grRemixRandomMusicPickForced(0);

    sMNRemixGalleryPage = mnRemixGalleryAudioGetPageForBgm(bgm_id);
    sMNRemixGallerySongNumber = 1;

    mnRemixGalleryPlaySong(bgm_id);
}

/* Gallery.asm initial_ _check_bgm: the Gallery song plays while no song of the picture was picked. */
static void mnRemixGalleryKeepGalleryBgm(void)
{
    if (sMNRemixGallerySongNumber != 0)
    {
        return;
    }
    if (syAudioGetBGMPlaying(0) == MN_REMIX_GALLERY_BGM_GALLERY)
    {
        return;
    }
    mnRemixGalleryPlaySong(MN_REMIX_GALLERY_BGM_GALLERY);
}

/* Starts the fade to the next picture; the song fades out with it when a song of the picture plays (Gallery.asm _begin_fade). */
static s32 mnRemixGalleryBeginFade(s32 fade_length)
{
    if (sMNRemixGallerySongNumber != 0)
    {
        syAudioSetBGMVolumeFade(0, 0, fade_length);
    }
    return fade_length;
}

static s32 mnRemixGalleryRandomRange(s32 range)
{
    return syUtilsRandIntRange(range);
}

static sb32 mnRemixGalleryHoldsButtons(u32 buttons)
{
    s32 player;

    for (player = 0; player < ARRAY_COUNT(gSYControllerDevices); player++)
    {
        if ((mnCongraCheckPlayerControllerConnected(player) != FALSE) && (gSYControllerDevices[player].button_hold & buttons))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Stick direction of player 1: 1 = right, -1 = left, 0 = inside the dead zone. */
static s32 mnRemixGalleryReadStickDirection(void)
{
    s32 stick_x = gSYControllerDevices[0].stick_range.x;

    if (stick_x >= MN_REMIX_GALLERY_STICK_THRESHOLD)
    {
        return 1;
    }
    if (stick_x <= -MN_REMIX_GALLERY_STICK_THRESHOLD)
    {
        return -1;
    }
    return 0;
}

/* Gallery.asm _cycle_image: turns the idle mode off, moves the page and starts the fade. */
static s32 mnRemixGalleryChangePage(s32 step, s32 fade_length)
{
    sMNRemixGalleryIdleMode = MN_REMIX_GALLERY_IDLE_OFF;
    sMNRemixGalleryPage = mnRemixGalleryRulesStepIndex(sMNRemixGalleryPage, step);

    return mnRemixGalleryBeginFade(fade_length);
}

/* Gallery.asm _check_cycle: a pushed stick fades normally, a held D-pad fades quickly with a scroll sound. */
static s32 mnRemixGalleryReadPageChange(void)
{
    s32 stick_direction = mnRemixGalleryReadStickDirection();
    s32 step = 0;

    if (stick_direction != 0)
    {
        return mnRemixGalleryChangePage(stick_direction, MN_REMIX_GALLERY_FADE_NORMAL);
    }
    if (mnRemixGalleryHoldsButtons(L_JPAD) != FALSE)
    {
        step = -1;
    }
    else if (mnRemixGalleryHoldsButtons(R_JPAD) != FALSE)
    {
        step = 1;
    }
    if (step == 0)
    {
        return 0;
    }
    syAudioPlayFGM(MN_REMIX_GALLERY_FGM_SCROLL);

    return mnRemixGalleryChangePage(step, MN_REMIX_GALLERY_FADE_QUICK);
}

/* Gallery.asm _check_start_press: START builds a new shuffled order and cycles the idle mode. */
static s32 mnRemixGalleryCycleIdleMode(void)
{
    s32 fgm_id = (sMNRemixGalleryIdleMode == MN_REMIX_GALLERY_IDLE_SHUFFLE) ? MN_REMIX_GALLERY_FGM_SELECT_STAGE : MN_REMIX_GALLERY_FGM_CONFIRM;

    mnRemixGalleryRulesBuildIdleOrder(sMNRemixGalleryIdleOrder, mnRemixGalleryRandomRange);

    sMNRemixGalleryIdleMode = mnRemixGalleryRulesNextIdleMode(sMNRemixGalleryIdleMode);
    sMNRemixGalleryIdleIndex = -1;

    syAudioPlayFGM(fgm_id);

    return mnRemixGalleryBeginFade(MN_REMIX_GALLERY_FADE_IDLE);
}

/* Gallery.asm main_: counts the idle timer down and tells when it ran out. */
static sb32 mnRemixGalleryTickIdleTimer(void)
{
    if (sMNRemixGalleryIdleMode == MN_REMIX_GALLERY_IDLE_OFF)
    {
        return FALSE;
    }
    sMNRemixGalleryIdleFrames--;

    return (sMNRemixGalleryIdleFrames <= 0) ? TRUE : FALSE;
}

/* Gallery.asm _check_l_press, _check_r_press and _check_z_press: L and R drum, Z picks the next sound. */
static void mnRemixGalleryPlayDrumKit(void)
{
    if (mnCongraGetPlayerTapButtons(L_TRIG) != FALSE)
    {
        syAudioPlayFGM(mnRemixGalleryAudioGetDrumFgm(sMNRemixGalleryDrumKit, FALSE));
    }
    else if (mnCongraGetPlayerTapButtons(R_TRIG) != FALSE)
    {
        syAudioPlayFGM(mnRemixGalleryAudioGetDrumFgm(sMNRemixGalleryDrumKit, TRUE));
    }
    if (mnCongraGetPlayerTapButtons(Z_TRIG) != FALSE)
    {
        sMNRemixGalleryDrumKit = mnRemixGalleryAudioNextDrumKit(sMNRemixGalleryDrumKit);
    }
}

/* Gallery.asm _a_press_normal: plays the next song of the picture. */
static void mnRemixGalleryPlayNextSong(void)
{
    sMNRemixGallerySongNumber = mnRemixGalleryAudioNextSongNumber(sMNRemixGalleryPage, sMNRemixGallerySongNumber);

    mnRemixGalleryPlaySong(mnRemixGalleryAudioGetSong(sMNRemixGalleryPage, sMNRemixGallerySongNumber));
    syAudioPlayFGM(MN_REMIX_GALLERY_FGM_CONFIRM);
}

/* Reloads the Congratulations scene on the new picture. */
static void mnRemixGalleryReloadScene(void)
{
    gSCManagerSceneData.scene_prev = nSCKindCongra;
    gSCManagerSceneData.scene_curr = nSCKindCongra;
    syTaskmanSetLoadScene();
}

/* Gallery.asm _leave_gallery: remembers the picture, stops the song and goes back with the menu music. */
static void mnRemixGalleryLeave(void)
{
    sMNRemixGalleryIsActive = FALSE;
    sMNRemixGalleryLastPage = sMNRemixGalleryPage;
    sMNRemixGalleryDrumKit = 0;

    syAudioStopBGMAll();
    gmRemixMenuMusicPlay(nSYAudioBGMModeSelect, FALSE);

    gSCManagerSceneData.scene_prev = nSCKindCongra;
    gSCManagerSceneData.scene_curr = sMNRemixGalleryPreviousScene;
    syTaskmanSetLoadScene();
}

void mnRemixGalleryActivateAtPage(s32 previous_scene, s32 page)
{
    sMNRemixGalleryIsActive = TRUE;
    sMNRemixGalleryPage = mnRemixGalleryRulesStepIndex(page, 0);
    sMNRemixGallerySongNumber = 0;
    sMNRemixGalleryIdleMode = MN_REMIX_GALLERY_IDLE_OFF;
    sMNRemixGalleryPreviousScene = previous_scene;
}

void mnRemixGalleryActivate(s32 previous_scene)
{
    mnRemixGalleryActivateAtPage(previous_scene, 0);
}

s32 mnRemixGalleryGetLastPage(void)
{
    return sMNRemixGalleryLastPage;
}

sb32 mnRemixGalleryIsActive(void)
{
    return sMNRemixGalleryIsActive;
}

void mnRemixGalleryPrepareScene(void)
{
    if (sMNRemixGalleryIsActive == FALSE)
    {
        return;
    }
    if (sMNRemixGalleryIdleMode == MN_REMIX_GALLERY_IDLE_OFF)
    {
        mnRemixGalleryKeepGalleryBgm();
        return;
    }
    if (sMNRemixGalleryIdleMode == MN_REMIX_GALLERY_IDLE_SHUFFLE)
    {
        mnRemixGalleryShowNextShuffledPage();
    }
    else
    {
        mnRemixGalleryShowRandomSongPage();
    }
    sMNRemixGalleryIdleFrames = MN_REMIX_GALLERY_IDLE_FRAMES;
}

s32 mnRemixGalleryGetFighterKind(void)
{
    return REMIX_ID_TO_PORT_FKIND(mnRemixGalleryRulesGetCharacterId(sMNRemixGalleryPage));
}

s32 mnRemixGalleryUpdateInput(void)
{
    syAudioSetBGMVolume(0, MN_REMIX_GALLERY_BGM_VOLUME);

    if (mnRemixGalleryTickIdleTimer() != FALSE)
    {
        return mnRemixGalleryBeginFade(MN_REMIX_GALLERY_FADE_IDLE);
    }
    if (mnCongraGetPlayerTapButtons(START_BUTTON) != FALSE)
    {
        return mnRemixGalleryCycleIdleMode();
    }
    if (mnCongraGetPlayerTapButtons(B_BUTTON) != FALSE)
    {
        mnRemixGalleryLeave();
        return 0;
    }
    mnRemixGalleryPlayDrumKit();

    if (mnCongraGetPlayerTapButtons(A_BUTTON) != FALSE)
    {
        if (sMNRemixGalleryIdleMode != MN_REMIX_GALLERY_IDLE_OFF)
        {
            syAudioPlayFGM(MN_REMIX_GALLERY_FGM_CONFIRM);
            return mnRemixGalleryBeginFade(MN_REMIX_GALLERY_FADE_IDLE);
        }
        mnRemixGalleryPlayNextSong();
    }
    return mnRemixGalleryReadPageChange();
}

void mnRemixGalleryOnFadeEnd(void)
{
    if (sMNRemixGallerySongNumber != 0)
    {
        syAudioStopBGMAll();
        sMNRemixGallerySongNumber = 0;
    }
    mnRemixGalleryReloadScene();
}

#endif
