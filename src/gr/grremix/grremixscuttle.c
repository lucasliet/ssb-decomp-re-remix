/**
 * grremixscuttle.c - Scuttle Town background fade and minion setup. See grremixscuttle.h.
 * Ported from Hazards.asm scuttle_town_setup_ and scuttle_town_bg_fade_.
 */

#ifdef PORT

#include "grremixscuttle.h"
#include "grremixhazards.h"
#include "grremixscuttleminion.h"
#include <lb/library.h>
#include <gr/ground.h>
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <mp/map.h>
#include <port_log.h>

#define GRREMIX_SCUTTLE_MINIONS_ENABLED FALSE /* asm scuttle_town_setup_: `b _end` before the minion setup */
#define GRREMIX_SCUTTLE_HEADER_TO_GROUND_DATA 0x14 /* asm scuttle_town_setup_: header = 0x80131300 pointer - 0x14 */
#define GRREMIX_SCUTTLE_DAY_HEADER_FIELD 0x48 /* asm: day background image header, in the ground data */
#define GRREMIX_SCUTTLE_NIGHT_HEADER_FIELD 0xC8 /* asm: night background image header, in the ground data */
#define GRREMIX_SCUTTLE_IMAGE_BG_ADDRESS_FIELD 0x34 /* asm: background address, in the image header */
#define GRREMIX_SCUTTLE_GLASS_COLOR_FIELD 0x4C /* asm: magnifying glass color, in the ground data */
#define GRREMIX_SCUTTLE_PALETTE_NIGHT 0x3F800000 /* asm: palette index 1.0F, the first palette is shown (to night) */
#define GRREMIX_SCUTTLE_PALETTE_DAY 0x41100000 /* asm: palette index 9.0F, the second to last palette is shown (to day) */
#define GRREMIX_SCUTTLE_ALPHA_STEP 4
#define GRREMIX_SCUTTLE_ALPHA_MIDDLE 0x80
#define GRREMIX_SCUTTLE_ALPHA_MAX 0xFC
#define GRREMIX_SCUTTLE_RECT_X1 310
#define GRREMIX_SCUTTLE_RECT_Y1 230
#define GRREMIX_SCUTTLE_PROC_PRIORITY 3
#define GRREMIX_SCUTTLE_DISPLAY_PRIORITY 6

#define GRREMIX_SCUTTLE_NIGHT 0
#define GRREMIX_SCUTTLE_DAY 1
#define GRREMIX_SCUTTLE_TRANSITIONS 2

typedef struct GRRemixScuttleTransitionColors
{
    u32 rgb;
    u32 glass_color;
} GRRemixScuttleTransitionColors;

typedef struct GRRemixScuttleFadeState
{
    sb32 in_transition;
    s32 alpha;
    s32 alpha_delta;
    u32 rgb;
    u32 glass_color;
    void *bg_address;
    void *bg_addresses[GRREMIX_SCUTTLE_TRANSITIONS];
} GRRemixScuttleFadeState;

/* Hazards.asm COLOR_NIGHT, COLOR_NIGHT_GLASS, COLOR_DAY, COLOR_DAY_GLASS, in the asm order of the night and day slots. */
static const GRRemixScuttleTransitionColors sGRRemixScuttleColors[GRREMIX_SCUTTLE_TRANSITIONS] =
{
    { 0x00105000, 0x00103800 },
    { 0x94CEFF00, 0x9ACEFF00 },
};

static GRRemixScuttleFadeState sGRRemixScuttleFade;

/**
 * Returns the raw stage palette index read by the asm (RAM 0x800466F8 group, top joint, part image palette). The
 * port cannot walk that object tree yet.
 * REMIX-TODO(palette): the asm walks the group 0x2 head (RAM 0x800466F8) to the top joint, first joint (+0x10), part
 * image struct (+0x80) and reads the current palette index (+0x88). Until that walk is ported it returns 0, which
 * matches neither palette, so the background transition never starts.
 *
 * @return the raw palette index, 0 while the port cannot read it.
 */
static u32 grRemixScuttleStagePaletteIndex(void)
{
    return 0;
}

/**
 * Returns whether the training stage background is shown, the only training case where the asm runs the fade
 * (asm: Training.entry_bg + 0x4 is non-zero).
 * REMIX-TODO(toggles): the Training menu background value has no port accessor yet, so the normal training
 * background (value 0, no fade) is assumed.
 *
 * @return FALSE, the normal training background is assumed.
 */
static sb32 grRemixScuttleTrainingShowsStageBg(void)
{
    return FALSE;
}

/**
 * Returns whether the fade is created on this stage load. Every match runs it; training runs it only with the stage
 * background (asm: _check_hazard is reached without the fade otherwise).
 *
 * @return TRUE when the background fade object is created.
 */
static sb32 grRemixScuttleFadeIsWanted(void)
{
    if (gSCManagerSceneData.scene_curr != nSCKind1PTrainingMode)
    {
        return TRUE;
    }
    return grRemixScuttleTrainingShowsStageBg();
}

/**
 * Returns the background address of one transition, read from the stage file (asm: header field -> image header
 * -> background address at +0x34). The file pointers are relocation tokens, so they go through PORT_RESOLVE.
 *
 * @param image_header_field Ground data offset of the background image header pointer.
 * @return the background address the transition shows.
 */
static void *grRemixScuttleBgAddress(u32 image_header_field)
{
    u8 *ground = (u8 *) gMPCollisionGroundData;
    u8 *image_header = (u8 *) PORT_RESOLVE(*(u32 *) (ground + image_header_field));

    return (void *) PORT_RESOLVE(*(u32 *) (image_header + GRREMIX_SCUTTLE_IMAGE_BG_ADDRESS_FIELD));
}

/**
 * Reads the night and day background addresses of the stage file and selects the fade colors (asm: _init_fade).
 * The asm saves the addresses in the fade object at 0x68 (night) and 0x6C (day).
 */
static void grRemixScuttleFadeLoadBackgrounds(void)
{
    sGRRemixScuttleFade.bg_addresses[GRREMIX_SCUTTLE_NIGHT] =
        grRemixScuttleBgAddress(GRREMIX_SCUTTLE_NIGHT_HEADER_FIELD);
    sGRRemixScuttleFade.bg_addresses[GRREMIX_SCUTTLE_DAY] =
        grRemixScuttleBgAddress(GRREMIX_SCUTTLE_DAY_HEADER_FIELD);
}

/**
 * Clears the fade state at setup. The fade starts transparent, as the asm draw_rectangle with color 0 and alpha 0.
 */
static void grRemixScuttleFadeResetState(void)
{
    sGRRemixScuttleFade.in_transition = FALSE;
    sGRRemixScuttleFade.alpha = 0;
    sGRRemixScuttleFade.alpha_delta = 0;
    sGRRemixScuttleFade.rgb = 0;
    sGRRemixScuttleFade.glass_color = 0;
    sGRRemixScuttleFade.bg_address = NULL;
}

/**
 * Returns the transition index for the stage palette, or -1 while the palette is not changing (asm: _check the
 * palette index at the start of the transition).
 *
 * @return GRREMIX_SCUTTLE_NIGHT, GRREMIX_SCUTTLE_DAY or -1.
 */
static s32 grRemixScuttleFadeFindTransition(void)
{
    u32 palette = grRemixScuttleStagePaletteIndex();

    if (palette == GRREMIX_SCUTTLE_PALETTE_NIGHT)
    {
        return GRREMIX_SCUTTLE_NIGHT;
    }
    if (palette == GRREMIX_SCUTTLE_PALETTE_DAY)
    {
        return GRREMIX_SCUTTLE_DAY;
    }
    return -1;
}

/**
 * Starts the transition to the given background and selects its colors (asm: _start_transition).
 *
 * @param transition GRREMIX_SCUTTLE_NIGHT or GRREMIX_SCUTTLE_DAY.
 */
static void grRemixScuttleFadeStart(s32 transition)
{
    const GRRemixScuttleTransitionColors *colors = &sGRRemixScuttleColors[transition];

    sGRRemixScuttleFade.in_transition = TRUE;
    sGRRemixScuttleFade.alpha_delta = GRREMIX_SCUTTLE_ALPHA_STEP;
    sGRRemixScuttleFade.rgb = colors->rgb & 0xFFFFFF00;
    sGRRemixScuttleFade.glass_color = colors->glass_color;
    sGRRemixScuttleFade.bg_address = sGRRemixScuttleFade.bg_addresses[transition];
}

/**
 * Writes the magnifying glass color of the current transition into the ground data (asm: _in_transition at the
 * middle alpha, which stores the glass color at stage header + 0x4C).
 */
static void grRemixScuttleFadeWriteGlassColor(void)
{
    u8 *ground = (u8 *) gMPCollisionGroundData;

    *(u32 *) (ground + GRREMIX_SCUTTLE_GLASS_COLOR_FIELD) = sGRRemixScuttleFade.glass_color;
}

/**
 * Switches the stage background at the max alpha (asm: _check_max, which stores the background address into the
 * background image object).
 * REMIX-TODO(bg image object): the asm writes the background address to the bg image object position struct (RAM
 * 0x80046800 -> +0x74, field +0x44). Until that object is ported the background stays the same.
 */
static void grRemixScuttleFadeSwitchBackground(void)
{
}

/**
 * Moves the fade alpha by the current delta and applies the middle and max alpha changes (asm: _in_transition). The
 * transition ends at alpha 0.
 */
static void grRemixScuttleFadeStepAlpha(void)
{
    sGRRemixScuttleFade.alpha += sGRRemixScuttleFade.alpha_delta;

    if (sGRRemixScuttleFade.alpha == 0)
    {
        sGRRemixScuttleFade.in_transition = FALSE;
        return;
    }
    if (sGRRemixScuttleFade.alpha == GRREMIX_SCUTTLE_ALPHA_MIDDLE)
    {
        grRemixScuttleFadeWriteGlassColor();
        return;
    }
    if (sGRRemixScuttleFade.alpha == GRREMIX_SCUTTLE_ALPHA_MAX)
    {
        sGRRemixScuttleFade.alpha_delta = -GRREMIX_SCUTTLE_ALPHA_STEP;
        grRemixScuttleFadeSwitchBackground();
    }
}

/**
 * Per-frame update of the fade object. Waits for the stage palette change while no transition runs, then steps the
 * transition (asm: scuttle_town_bg_fade_).
 *
 * @param gobj the fade object, unused
 */
static void grRemixScuttleFadeUpdate(GObj *gobj)
{
    s32 transition;

    if (!sGRRemixScuttleFade.in_transition)
    {
        transition = grRemixScuttleFadeFindTransition();
        if (transition < 0)
        {
            return;
        }
        grRemixScuttleFadeStart(transition);
    }
    grRemixScuttleFadeStepAlpha();
}

/**
 * Draws the fade rectangle over the screen at the current alpha (asm: Render.draw_rectangle at 10, 10, 300 x 220).
 *
 * @param gobj the fade object, unused
 */
static void grRemixScuttleFadeDisplay(GObj *gobj)
{
    u32 rgb = sGRRemixScuttleFade.rgb;

    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, rgb >> 24, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF,
        sGRRemixScuttleFade.alpha);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
    gDPFillRectangle(gSYTaskmanDLHeads[0]++, 10, 10, GRREMIX_SCUTTLE_RECT_X1, GRREMIX_SCUTTLE_RECT_Y1);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
}

/**
 * Creates the background fade object with its update and display, and reads the stage backgrounds (asm:
 * scuttle_town_setup_, _init_fade).
 */
static void grRemixScuttleFadeSetup(void)
{
    GObj *gobj;

    grRemixScuttleFadeResetState();
    grRemixScuttleFadeLoadBackgrounds();
    gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    if (gobj == NULL)
    {
        port_log("[remix-hazard] scuttle: fade object not created, the GObj pool is full");
        return;
    }
    gcAddGObjProcess(gobj, grRemixScuttleFadeUpdate, nGCProcessKindFunc, GRREMIX_SCUTTLE_PROC_PRIORITY);
    gcAddGObjDisplay(gobj, grRemixScuttleFadeDisplay, GRREMIX_SCUTTLE_DISPLAY_PRIORITY, GOBJ_PRIORITY_DEFAULT, ~0);
}

/**
 * Creates the Scuttle Minion factory on the stage header, when hazards are on (asm: scuttle_town_setup_ checks
 * Toggles.entry_hazard_mode). Remix 2.0.1 disables this branch with a `b _end` (Hazards.asm line 7338, "remove when
 * minions stop crashing"), so the stage ships without minions; GRREMIX_SCUTTLE_MINIONS_ENABLED mirrors that. The
 * minion port (itremixscuttleminion.c) is kept for when the Remix author enables it.
 */
static void grRemixScuttleMinionSetup(void)
{
    void *header;

    if (!GRREMIX_SCUTTLE_MINIONS_ENABLED || !grRemixHazardsAreOn())
    {
        return;
    }
    header = (u8 *) gMPCollisionGroundData - GRREMIX_SCUTTLE_HEADER_TO_GROUND_DATA;
    grRemixScuttleMinionFactoryMake(header);
}

/**
 * Scuttle Town setup (asm: scuttle_town_setup_): the background fade, then the minion factory.
 */
void grRemixScuttleTownSetup(void)
{
    if (grRemixScuttleFadeIsWanted())
    {
        grRemixScuttleFadeSetup();
    }
    grRemixScuttleMinionSetup();
}

#endif
