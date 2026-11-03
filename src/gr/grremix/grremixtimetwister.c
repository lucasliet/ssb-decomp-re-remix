/**
 * grremixtimetwister.c - Time Twister background fade. See grremixtimetwister.h.
 */

#ifdef PORT

#include "grremixtimetwister.h"
#include "grremixstagebg.h"
#include "gm/gmremixbgm.h"
#include "gm/gmremixbgmrules.h"
#include <lb/library.h>
#include <gr/ground.h>
#include <gm/gmcamera.h>
#include <mp/map.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/audio.h>
#include <sys/objman.h>
#include <port_log.h>
#include "remix/remix_training.h"

#define GRREMIX_TIMETWISTER_BEGIN_FADE_FRAMES 180
#define GRREMIX_TIMETWISTER_WARP_DISAPPEAR_FRAMES 270
#define GRREMIX_TIMETWISTER_ALPHA_MIDDLE 0x80
#define GRREMIX_TIMETWISTER_ALPHA_MAX 0xFC
#define GRREMIX_TIMETWISTER_ALPHA_STEP 4
#define GRREMIX_TIMETWISTER_RECT_X1 310
#define GRREMIX_TIMETWISTER_RECT_Y1 230
#define GRREMIX_TIMETWISTER_PROC_PRIORITY 3
#define GRREMIX_TIMETWISTER_DISPLAY_PRIORITY 6
#define GRREMIX_TIMETWISTER_FGM_WARP_APPEAR 0x059D
#define GRREMIX_TIMETWISTER_FGM_WARP_DIE 0x059E
#define GRREMIX_TIMETWISTER_FGM_WARPING 0x0094
#define GRREMIX_TIMETWISTER_START_BGM_FRAMES (GRREMIX_TIMETWISTER_WARP_DISAPPEAR_FRAMES + 30)
#define GRREMIX_TIMETWISTER_BGM_FADE_FRAMES 90
#define GRREMIX_TIMETWISTER_BGM_VOLUME 30720
#define GRREMIX_TIMETWISTER_BACKGROUNDS 4
/* asm time_twister_setup_: ground data fields of the image headers, in the background index order (N. Sanity Beach, Snow
 * Go, Future Frenzy, Time Twister). */
#define GRREMIX_TIMETWISTER_GLASS_COLOR_FIELD 0x4C
#define GRREMIX_TIMETWISTER_BGM_UNSET (-1)

typedef struct GRRemixTimeTwisterState
{
    s32 timer;
    s32 alpha;
    s32 alpha_delta;
    u32 rgb;
    u32 glass_color;
    s32 bg_index;
    u32 bg_bitmap;
    u32 bg_bitmaps[GRREMIX_TIMETWISTER_BACKGROUNDS];
    s32 bgm_change;
    void (*saved_camera_func)(GObj*);
} GRRemixTimeTwisterState;

static const u32 sGRRemixTimeTwisterBgFields[GRREMIX_TIMETWISTER_BACKGROUNDS] = { 0xCC, 0xD0, 0xD4, 0x48 };

/* Same values as the asm magnifying glass colors (Hazards.asm time_twister_glass_colors). */
static const u32 sGRRemixTimeTwisterColors[4] =
{
    0x7AABD900, /* N. Sanity Beach */
    0x4C82BD00, /* Snow Go */
    0x5E729800, /* Future Frenzy */
    0x00005400, /* Time Twister */
};

static GRRemixTimeTwisterState sGRRemixTimeTwister;

/** Clears the transition state at setup; the timer 0 means no transition is running. */
static void grRemixTimeTwisterResetState(void)
{
    sGRRemixTimeTwister.timer = 0;
    sGRRemixTimeTwister.alpha = 0;
    sGRRemixTimeTwister.alpha_delta = 0;
    sGRRemixTimeTwister.rgb = 0;
    sGRRemixTimeTwister.glass_color = 0;
    sGRRemixTimeTwister.bg_index = 0;
    sGRRemixTimeTwister.bg_bitmap = 0;
    sGRRemixTimeTwister.bgm_change = GRREMIX_TIMETWISTER_BGM_UNSET;
    sGRRemixTimeTwister.saved_camera_func = NULL;
}

/** Reads the four background bitmaps of the stage file (asm: _init_fade). */
static void grRemixTimeTwisterLoadBackgrounds(void)
{
    s32 index;

    for (index = 0; index < GRREMIX_TIMETWISTER_BACKGROUNDS; index++)
    {
        sGRRemixTimeTwister.bg_bitmaps[index] = grRemixStageBgBitmapToken(sGRRemixTimeTwisterBgFields[index]);
    }
}

/**
 * Returns the background index of the next stage when the warp ball is displayed, or -1 while it is not (asm: the walk
 * from the first object of group 2 to the warp ball and the current image of its material).
 */
static s32 grRemixTimeTwisterFindWarpBgIndex(void)
{
    s32 bg_index;

    if (!grRemixStageReadWarpBallBgIndex(&bg_index) || (bg_index < 0) || (bg_index >= GRREMIX_TIMETWISTER_BACKGROUNDS))
    {
        return -1;
    }
    return bg_index;
}

/** Zooms the camera out to the stage zoom bounds, like the Bonus pause (asm: Camera.zoom_to_max_). */
static void grRemixTimeTwisterZoomOut(void)
{
    Vec3f origin;
    Vec3f target;

    origin.x = gMPCollisionGroundData->zoom_start.x;
    origin.y = gMPCollisionGroundData->zoom_start.y;
    origin.z = gMPCollisionGroundData->zoom_start.z;
    target.x = gMPCollisionGroundData->zoom_end.x;
    target.y = gMPCollisionGroundData->zoom_end.y;
    target.z = gMPCollisionGroundData->zoom_end.z;
    gmCameraSetStatusMapZoom(&origin, &target);
}

/** The Time Twister swaps the song only once per match, and only on a stage that plays its own song (asm: _zoom_out). */
static void grRemixTimeTwisterDecideBgmChange(void)
{
    if (sGRRemixTimeTwister.bgm_change != GRREMIX_TIMETWISTER_BGM_UNSET)
    {
        return;
    }
    sGRRemixTimeTwister.bgm_change = gmRemixBgmRulesTimeTwisterChanges(gMPCollisionBGMDefault) ? TRUE : FALSE;
}

/**
 * Camera restore at the warp disappearing frame: the status resets, and the camera routine returns to the saved one when
 * it is still the map zoom (asm: _warp_disappear).
 */
static void grRemixTimeTwisterRestoreCamera(void)
{
    gGMCameraStruct.status_curr = 0;

    if (gGMCameraStruct.func_camera == gmCameraMapZoomFuncCamera)
    {
        gGMCameraStruct.func_camera = sGRRemixTimeTwister.saved_camera_func;
    }
}

/** Starts the next song of the background and restores the volume (asm: START_NEW_BGM). */
static void grRemixTimeTwisterStartNewBgm(void)
{
    s32 track;

    if (!sGRRemixTimeTwister.bgm_change)
    {
        return;
    }
    track = gmRemixBgmRulesTimeTwisterTrack(sGRRemixTimeTwister.bg_index);
    if (track != GMREMIX_BGM_NO_TRACK)
    {
        gmRemixBgmChange(track);
    }
    syAudioSetBGMVolume(0, GRREMIX_TIMETWISTER_BGM_VOLUME);
}

/**
 * Runs the cue of the current frame of the transition (asm: _bg_fade_begin, _warp_disappear and the new song).
 */
static void grRemixTimeTwisterPlayCue(s32 frame)
{
    if (frame == GRREMIX_TIMETWISTER_BEGIN_FADE_FRAMES)
    {
        syAudioPlayFGM(GRREMIX_TIMETWISTER_FGM_WARPING);
        if (sGRRemixTimeTwister.bgm_change)
        {
            gmRemixBgmFadeOut(GRREMIX_TIMETWISTER_BGM_FADE_FRAMES);
        }
    }
    else if (frame == GRREMIX_TIMETWISTER_WARP_DISAPPEAR_FRAMES)
    {
        syAudioPlayFGM(GRREMIX_TIMETWISTER_FGM_WARP_DIE);
        grRemixTimeTwisterRestoreCamera();
    }
    else if (frame == GRREMIX_TIMETWISTER_START_BGM_FRAMES)
    {
        grRemixTimeTwisterStartNewBgm();
    }
}

/** Starts the transition at the warp ball and selects the colors of the next stage background. */
static void grRemixTimeTwisterStartTransition(s32 bg_index)
{
    port_log("remix time twister: warp to background %d (song change %d)\n", (int)bg_index, (int)sGRRemixTimeTwister.bgm_change);
    sGRRemixTimeTwister.timer = 1;
    sGRRemixTimeTwister.bg_index = bg_index;
    sGRRemixTimeTwister.alpha = 0;
    sGRRemixTimeTwister.alpha_delta = GRREMIX_TIMETWISTER_ALPHA_STEP;
    sGRRemixTimeTwister.rgb = sGRRemixTimeTwisterColors[bg_index] & 0xFFFFFF00;
    sGRRemixTimeTwister.glass_color = sGRRemixTimeTwisterColors[bg_index];
    sGRRemixTimeTwister.bg_bitmap = sGRRemixTimeTwister.bg_bitmaps[bg_index];
    grRemixTimeTwisterDecideBgmChange();
    sGRRemixTimeTwister.saved_camera_func = gGMCameraStruct.func_camera;
    gGMCameraStruct.func_camera = gmCameraAnimFuncCamera;
    grRemixTimeTwisterZoomOut();
    syAudioPlayFGM(GRREMIX_TIMETWISTER_FGM_WARP_APPEAR);
}

/**
 * Moves the fade alpha by the current delta. At the middle alpha the magnifying glass color changes; at the max
 * alpha the background is swapped and the fade reverses; at zero the transition ends (asm: _bg_fade).
 */
static void grRemixTimeTwisterStepAlpha(void)
{
    sGRRemixTimeTwister.alpha += sGRRemixTimeTwister.alpha_delta;

    if (sGRRemixTimeTwister.alpha == 0)
    {
        sGRRemixTimeTwister.timer = 0;
    }
    else if (sGRRemixTimeTwister.alpha == GRREMIX_TIMETWISTER_ALPHA_MIDDLE)
    {
        *(u32 *) ((u8 *) gMPCollisionGroundData + GRREMIX_TIMETWISTER_GLASS_COLOR_FIELD) = sGRRemixTimeTwister.glass_color;
    }
    else if (sGRRemixTimeTwister.alpha == GRREMIX_TIMETWISTER_ALPHA_MAX)
    {
        sGRRemixTimeTwister.alpha_delta = -GRREMIX_TIMETWISTER_ALPHA_STEP;
        grRemixStageBgSetBitmap(sGRRemixTimeTwister.bg_bitmap);
    }
}

/**
 * Advances the transition by one frame. The fade alpha only moves from the frame the fade begins, as in the asm
 * (frames before BEGIN_FADE_FRAMES only count).
 */
static void grRemixTimeTwisterAdvanceTransition(void)
{
    s32 frame = sGRRemixTimeTwister.timer;

    sGRRemixTimeTwister.timer++;
    if (frame < GRREMIX_TIMETWISTER_BEGIN_FADE_FRAMES)
    {
        return;
    }

    grRemixTimeTwisterPlayCue(frame);
    grRemixTimeTwisterStepAlpha();
}

/**
 * Per-frame update of the fade object. Waits for the warp ball while no transition runs (asm: _finish on
 * timer 0), otherwise advances the transition.
 * @param gobj the fade object, unused
 */
static void grRemixTimeTwisterUpdate(GObj *gobj)
{
    s32 warp_bg_index;

    if (sGRRemixTimeTwister.timer != 0)
    {
        grRemixTimeTwisterAdvanceTransition();
        return;
    }

    warp_bg_index = grRemixTimeTwisterFindWarpBgIndex();
    if (warp_bg_index < 0)
    {
        return;
    }
    grRemixTimeTwisterStartTransition(warp_bg_index);
}

/**
 * Draws the fade rectangle over the screen at the current alpha (asm: Render.draw_rectangle at 10, 10, 300 x 220).
 * @param gobj the fade object, unused
 */
static void grRemixTimeTwisterDisplay(GObj *gobj)
{
    u32 rgb = sGRRemixTimeTwister.rgb;

    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, rgb >> 24, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, sGRRemixTimeTwister.alpha);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
    gDPFillRectangle(gSYTaskmanDLHeads[0]++, 10, 10, GRREMIX_TIMETWISTER_RECT_X1, GRREMIX_TIMETWISTER_RECT_Y1);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
}

/** The fade runs in every match; in Training only with the stage background (asm: time_twister_setup_ start). */
static sb32 grRemixTimeTwisterFadeIsWanted(void)
{
    if (gSCManagerSceneData.scene_curr != nSCKind1PTrainingMode)
    {
        return TRUE;
    }
    return (remix_training_get(REMIX_TRAINING_STAGE_BG) != 0) ? TRUE : FALSE;
}

/**
 * Creates the fade object with its update and display, and resets the transition state (asm: time_twister_setup_).
 */
void grRemixTimeTwisterSetup(void)
{
    GObj *gobj;

    if (!grRemixTimeTwisterFadeIsWanted())
    {
        return;
    }
    grRemixTimeTwisterResetState();
    grRemixTimeTwisterLoadBackgrounds();
    gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, grRemixTimeTwisterUpdate, nGCProcessKindFunc, GRREMIX_TIMETWISTER_PROC_PRIORITY);
    gcAddGObjDisplay(gobj, grRemixTimeTwisterDisplay, GRREMIX_TIMETWISTER_DISPLAY_PRIORITY, GOBJ_PRIORITY_DEFAULT, ~0);
}

#endif
