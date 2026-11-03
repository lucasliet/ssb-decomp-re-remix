/*
 * mnremixvsstatsview.c - Press A prompt and button handling of the VS match stats on the Results screen (VsStats.asm
 * check_menu_toggle_). The page layout is in mnremixvsstatspages.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <macros.h>
#include <sys/controller.h>
#include "mn/mnvsmode/mnremixvsstatsrules.h"
#include "mn/mnvsmode/mnremixvsstatspages.h"
#include "mn/mnvsmode/mnremixvsstatsview.h"
#include "mn/mnvsmode/mnremixvsstatsscene.h"
#include "gm/gmremixgroupdisplay.h"
#include "remix/remix_hud_text.h"

extern void *func_800269C0_275C0(u16 id);

/* FGM.asm menu.TOGGLE (163) and menu.SCROLL (164), played by check_menu_toggle_ */
#define MNVSSTATS_VIEW_FGM_MENU_TOGGLE 163
#define MNVSSTATS_VIEW_FGM_MENU_SCROLL 164

#define MNVSSTATS_VIEW_PROMPT_SLOT 0
#define MNVSSTATS_VIEW_HINT_BACK_SLOT 1
#define MNVSSTATS_VIEW_HINT_NEXT_SLOT 2
#define MNVSSTATS_VIEW_PROMPT_X 160.0F
#define MNVSSTATS_VIEW_PROMPT_Y 218.0F
#define MNVSSTATS_VIEW_HINT_Y 15.9375F
#define MNVSSTATS_VIEW_HINT_BACK_X 34.75F
#define MNVSSTATS_VIEW_HINT_NEXT_X 87.0F
#define MNVSSTATS_VIEW_TEXT_SCALE 0.8125F
#define MNVSSTATS_VIEW_WHITE 0xFFFFFFFF
/* Display group (gGCCommonLinks) of the normal results rows, which VsStats.asm check_menu_toggle_ hides while a page shows. */
#define MNVSSTATS_VIEW_GROUP_RESULTS_ROWS 0x16

static s32 sMNRemixVSStatsViewPage;
static sb32 sMNRemixVSStatsViewVisible;
static sb32 sMNRemixVSStatsViewPromptShown;
static s32 sMNRemixVSStatsViewPressBuffer;

/** TRUE when any controller tapped the button this frame (asm Joypad.check_buttons_all_ with Joypad.PRESSED). */
static sb32 mnRemixVSStatsViewIsPressed(u16 button)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gSYControllerDevices); i++)
    {
        if (gSYControllerDevices[i].button_tap & button)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** Starts the short lock after a show or hide, during which presses are ignored (STATS_PRESS_BUFFER_PERIOD). */
static void mnRemixVSStatsViewStartPressBuffer(void)
{
    sMNRemixVSStatsViewPressBuffer = MNREMIX_VSSTATS_RULES_PRESS_BUFFER_PERIOD;
}

/**
 * Hides or shows the normal results rows and the Press A prompt, and the opposite for the stats page background and its
 * B and R buttons (Render.toggle_group_display_).
 */
static void mnRemixVSStatsViewSetResultsHidden(u32 hide)
{
    gmRemixGroupDisplaySet(MNREMIX_VSSTATS_GROUP_PROMPT, hide);
    gmRemixGroupDisplaySet(MNVSSTATS_VIEW_GROUP_RESULTS_ROWS, hide);
    gmRemixGroupDisplaySet(MNREMIX_VSSTATS_GROUP_PAGES, (hide == 0) ? 1 : 0);
}

/** A shows the page (asm check_menu_toggle_ _check_status). */
static void mnRemixVSStatsViewCheckShow(void)
{
    if (sMNRemixVSStatsViewPressBuffer != 0 || !mnRemixVSStatsViewIsPressed(A_BUTTON))
    {
        return;
    }
    mnRemixVSStatsViewStartPressBuffer();
    func_800269C0_275C0(MNVSSTATS_VIEW_FGM_MENU_TOGGLE);
    sMNRemixVSStatsViewVisible = TRUE;
    mnRemixVSStatsViewSetResultsHidden(1);
}

/** B hides the page and R moves to the next one (asm check_menu_toggle_ _match_status_up and _check_r). */
static void mnRemixVSStatsViewCheckHide(void)
{
    if (sMNRemixVSStatsViewPressBuffer != 0)
    {
        return;
    }
    if (mnRemixVSStatsViewIsPressed(B_BUTTON))
    {
        mnRemixVSStatsViewStartPressBuffer();
        func_800269C0_275C0(MNVSSTATS_VIEW_FGM_MENU_TOGGLE);
        sMNRemixVSStatsViewVisible = FALSE;
        mnRemixVSStatsViewSetResultsHidden(0);
        return;
    }
    if (mnRemixVSStatsViewIsPressed(R_TRIG))
    {
        sMNRemixVSStatsViewPage = mnRemixVSStatsRulesNextPage(sMNRemixVSStatsViewPage);
        func_800269C0_275C0(MNVSSTATS_VIEW_FGM_MENU_SCROLL);
    }
}

/** The Press A prompt above the results (asm press_a text, revealed by check_menu_toggle_). */
static void mnRemixVSStatsViewDrawPrompt(void)
{
    remix_hud_text_put(MNVSSTATS_VIEW_PROMPT_SLOT, "Press     for Match Stats", MNVSSTATS_VIEW_PROMPT_X,
                       MNVSSTATS_VIEW_PROMPT_Y, MNVSSTATS_VIEW_WHITE, MNVSSTATS_VIEW_TEXT_SCALE, REMIX_HUD_ALIGN_CENTER);
}

/** The B and R hints in the upper corners of the stats page (asm press_b and press_r). */
static void mnRemixVSStatsViewDrawHints(void)
{
    remix_hud_text_put(MNVSSTATS_VIEW_HINT_BACK_SLOT, ": Back", MNVSSTATS_VIEW_HINT_BACK_X, MNVSSTATS_VIEW_HINT_Y,
                       MNVSSTATS_VIEW_WHITE, MNVSSTATS_VIEW_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_put(MNVSSTATS_VIEW_HINT_NEXT_SLOT, ": Next Page", MNVSSTATS_VIEW_HINT_NEXT_X, MNVSSTATS_VIEW_HINT_Y,
                       MNVSSTATS_VIEW_WHITE, MNVSSTATS_VIEW_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
}

void mnRemixVSStatsViewSetup(void)
{
    sMNRemixVSStatsViewPage = 0;
    sMNRemixVSStatsViewVisible = FALSE;
    sMNRemixVSStatsViewPromptShown = FALSE;
    sMNRemixVSStatsViewPressBuffer = 0;
    mnRemixVSStatsSceneMake();
}

void mnRemixVSStatsViewRun(s32 results_tics, s32 allow_exit_wait)
{
    sMNRemixVSStatsViewPressBuffer = mnRemixVSStatsRulesTickBuffer(sMNRemixVSStatsViewPressBuffer);

    if (!mnRemixVSStatsRulesIsPromptActive(results_tics, allow_exit_wait))
    {
        return;
    }
    if (!sMNRemixVSStatsViewPromptShown)
    {
        sMNRemixVSStatsViewPromptShown = TRUE;
        gmRemixGroupDisplaySet(MNREMIX_VSSTATS_GROUP_PROMPT, 0);
    }
    if (sMNRemixVSStatsViewVisible)
    {
        mnRemixVSStatsViewCheckHide();
    }
    else
    {
        mnRemixVSStatsViewCheckShow();
    }

    if (sMNRemixVSStatsViewVisible)
    {
        mnRemixVSStatsPagesDraw(sMNRemixVSStatsViewPage);
        mnRemixVSStatsViewDrawHints();
        return;
    }
    mnRemixVSStatsViewDrawPrompt();
}

#endif
