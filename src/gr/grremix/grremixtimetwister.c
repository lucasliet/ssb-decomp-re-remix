/**
 * grremixtimetwister.c - Time Twister background fade. See grremixtimetwister.h.
 */

#ifdef PORT

#include "grremixtimetwister.h"
#include <lb/library.h>
#include <gr/ground.h>
#include <sys/audio.h>
#include <sys/objman.h>

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

typedef struct GRRemixTimeTwisterState
{
    s32 timer;
    s32 alpha;
    s32 alpha_delta;
    u32 rgb;
    u32 glass_color;
    s32 bg_index;
} GRRemixTimeTwisterState;

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
}

/**
 * Returns the background index of the next stage when the warp ball is displayed, or -1 while it is not.
 * The asm walks the first object of group 2 (the stage model): first joint, four siblings along, is the warp ball and
 * the current texture of its material is the next background.
 * REMIX-TODO(T1.5b): the walk is not enabled because the transition also needs the background image swap (asm writes
 * the bg address into the wallpaper object at group 0x80046800 + 0x74 + 0x44), which has no port equivalent yet;
 * starting the fade without it would only flash the screen. Until then no transition starts.
 */
static s32 grRemixTimeTwisterFindWarpBgIndex(void)
{
    return -1;
}

/**
 * Plays the warp sound of the current frame of the transition (asm: _bg_fade_begin, _warp_disappear).
 * REMIX-TODO(camera): the asm also switches the camera routine (Camera.zoom_to_max_, gGMCameraStruct routine
 * field at 0xC), and REMIX-TODO(bgm): the asm fades out the stage song and starts the next track at frame 300.
 */
static void grRemixTimeTwisterPlayCue(s32 frame)
{
    if (frame == GRREMIX_TIMETWISTER_BEGIN_FADE_FRAMES)
    {
        syAudioPlayFGM(GRREMIX_TIMETWISTER_FGM_WARPING);
    }
    else if (frame == GRREMIX_TIMETWISTER_WARP_DISAPPEAR_FRAMES)
    {
        syAudioPlayFGM(GRREMIX_TIMETWISTER_FGM_WARP_DIE);
    }
}

/** Starts the transition at the warp ball and selects the colors of the next stage background. */
static void grRemixTimeTwisterStartTransition(s32 bg_index)
{
    sGRRemixTimeTwister.timer = 1;
    sGRRemixTimeTwister.bg_index = bg_index;
    sGRRemixTimeTwister.alpha = 0;
    sGRRemixTimeTwister.alpha_delta = GRREMIX_TIMETWISTER_ALPHA_STEP;
    sGRRemixTimeTwister.rgb = sGRRemixTimeTwisterColors[bg_index] & 0xFFFFFF00;
    sGRRemixTimeTwister.glass_color = sGRRemixTimeTwisterColors[bg_index];
    syAudioPlayFGM(GRREMIX_TIMETWISTER_FGM_WARP_APPEAR);
}

/**
 * Moves the fade alpha by the current delta. At the middle alpha the magnifying glass color changes; at the max
 * alpha the fade reverses and at zero the transition ends (asm: _bg_fade).
 * REMIX-TODO(stage header): the asm writes the glass color to stage header + 0x4C and the next background to the
 * bg image object (RAM 0x80046800 + 0x44) at the max alpha; both need the stage file references.
 */
static void grRemixTimeTwisterStepAlpha(void)
{
    sGRRemixTimeTwister.alpha += sGRRemixTimeTwister.alpha_delta;

    if (sGRRemixTimeTwister.alpha == 0)
    {
        sGRRemixTimeTwister.timer = 0;
    }
    else if (sGRRemixTimeTwister.alpha == GRREMIX_TIMETWISTER_ALPHA_MAX)
    {
        sGRRemixTimeTwister.alpha_delta = -GRREMIX_TIMETWISTER_ALPHA_STEP;
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

/**
 * Creates the fade object with its update and display, and resets the transition state (asm: time_twister_setup_).
 */
void grRemixTimeTwisterSetup(void)
{
    GObj *gobj;

    grRemixTimeTwisterResetState();
    gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, grRemixTimeTwisterUpdate, nGCProcessKindFunc, GRREMIX_TIMETWISTER_PROC_PRIORITY);
    gcAddGObjDisplay(gobj, grRemixTimeTwisterDisplay, GRREMIX_TIMETWISTER_DISPLAY_PRIORITY, GOBJ_PRIORITY_DEFAULT, ~0);
}

#endif
