/**
 * sc1ptrainingremixexit.c - Hold to Exit of the Remix Training, see sc1ptrainingremixexit.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/controller.h>
#include "gm/gmremixgroupdisplay.h"
#include "gm/gmremixtoggle.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"
#include "sc/sc1pmode/sc1ptrainingleftoverrules.h"
#include "sc/sc1pmode/sc1ptrainingremixexit.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"

/* The asm group 0xF (Hold A text) is also the group of the battle pause legend, so the port uses a group of its own. */
#define SC1PTRAININGREMIXEXIT_GROUP 0x1C
#define SC1PTRAININGREMIXEXIT_DL_LINK 24

/* setup_ 1597-1598: text "(Hold --)" at (169, 166) scale 0.875 and the A glyph over the dashes at (202, 165) scale 0.9375. Slots 115 and 116. */
#define SC1PTRAININGREMIXEXIT_TEXT_SLOT 115
#define SC1PTRAININGREMIXEXIT_TEXT_X 169.0F
#define SC1PTRAININGREMIXEXIT_TEXT_Y 166.0F
#define SC1PTRAININGREMIXEXIT_TEXT_SCALE 0.875F
#define SC1PTRAININGREMIXEXIT_TEXT_COLOR 0xFFFFFFFF
#define SC1PTRAININGREMIXEXIT_GLYPH_X 202.0F
#define SC1PTRAININGREMIXEXIT_GLYPH_Y 165.0F
#define SC1PTRAININGREMIXEXIT_GLYPH_SCALE 0.9375F
#define SC1PTRAININGREMIXEXIT_GLYPH_TINT 0x50A8FFFF
#define SC1PTRAININGREMIXEXIT_GLYPH_PALETTE 0x303030FF

/* setup_ 1604-1611: the bar is a 2 pixel high rectangle at (73, 179). */
#define SC1PTRAININGREMIXEXIT_BAR_SLOT 30
#define SC1PTRAININGREMIXEXIT_BAR_X 73.0F
#define SC1PTRAININGREMIXEXIT_BAR_Y 179.0F
#define SC1PTRAININGREMIXEXIT_BAR_HEIGHT 2.0F
#define SC1PTRAININGREMIXEXIT_BAR_COLOR 0x0088FFFF

static s32 sSC1PTrainingRemixExitHeldFrames;
static s32 sSC1PTrainingRemixExitBarWidth;

void sc1PTrainingRemixExitStartScene(void)
{
    sSC1PTrainingRemixExitHeldFrames = 0;
    sSC1PTrainingRemixExitBarWidth = 0;
}

void sc1PTrainingRemixExitMakeLegend(void)
{
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_A, SC1PTRAININGREMIXEXIT_GROUP, SC1PTRAININGREMIXEXIT_DL_LINK,
                      SC1PTRAININGREMIXEXIT_GLYPH_X, SC1PTRAININGREMIXEXIT_GLYPH_Y, SC1PTRAININGREMIXEXIT_GLYPH_TINT,
                      SC1PTRAININGREMIXEXIT_GLYPH_PALETTE, SC1PTRAININGREMIXEXIT_GLYPH_SCALE);
    gmRemixGroupDisplaySet(SC1PTRAININGREMIXEXIT_GROUP, 1);
}

sb32 sc1PTrainingRemixExitIsRequested(void)
{
    SYController *controller = &gSYControllerDevices[gSCManagerSceneData.player];

    if (gmRemixToggle(hold_to_exit_training) == 0)
    {
        return ((controller->button_tap & A_BUTTON) != 0) ? TRUE : FALSE;
    }
    return (sc1PTrainingLeftoverRulesHoldExitStep(sSC1PTrainingRemixExitHeldFrames, ((controller->button_hold & A_BUTTON) != 0) ? 1 : 0,
                                                  &sSC1PTrainingRemixExitHeldFrames) != 0)
               ? TRUE
               : FALSE;
}

void sc1PTrainingRemixExitResetHold(void)
{
    sSC1PTrainingRemixExitHeldFrames = 0;
}

/** Joypad.check_buttons_all_ with HELD: any of the four controllers holds A. */
static sb32 sc1PTrainingRemixExitIsAnyAHeld(void)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        if (gSYControllerDevices[port].button_hold & A_BUTTON)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** run_ 2002-2012: the text and the bar belong to the vanilla pause menu with the cursor on Exit and the toggle on. */
static sb32 sc1PTrainingRemixExitIsShown(sb32 is_cursor_on_exit)
{
    if (gmRemixToggle(hold_to_exit_training) == 0)
    {
        return FALSE;
    }
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusPause)
    {
        return FALSE;
    }
    if (sc1PTrainingRemixMenuIsOpen() != FALSE)
    {
        return FALSE;
    }
    return is_cursor_on_exit;
}

/**
 * "(Hold --)" with the A glyph over the dashes: the glyph is a game sprite under the overlay text, so the dashes are left out and the
 * closing parenthesis sits where the full string would end.
 */
static void sc1PTrainingRemixExitPutText(void)
{
    f32 end_x = SC1PTRAININGREMIXEXIT_TEXT_X + remix_hud_text_measure("(Hold --)", SC1PTRAININGREMIXEXIT_TEXT_SCALE);

    remix_hud_text_put(SC1PTRAININGREMIXEXIT_TEXT_SLOT, "(Hold", SC1PTRAININGREMIXEXIT_TEXT_X, SC1PTRAININGREMIXEXIT_TEXT_Y,
                       SC1PTRAININGREMIXEXIT_TEXT_COLOR, SC1PTRAININGREMIXEXIT_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_put(SC1PTRAININGREMIXEXIT_TEXT_SLOT + 1, ")", end_x, SC1PTRAININGREMIXEXIT_TEXT_Y,
                       SC1PTRAININGREMIXEXIT_TEXT_COLOR, SC1PTRAININGREMIXEXIT_TEXT_SCALE, REMIX_HUD_ALIGN_RIGHT);
}

void sc1PTrainingRemixExitUpdate(sb32 is_cursor_on_exit)
{
    sb32 is_shown = sc1PTrainingRemixExitIsShown(is_cursor_on_exit);

    gmRemixGroupDisplaySet(SC1PTRAININGREMIXEXIT_GROUP, (is_shown != FALSE) ? 0 : 1);

    if (is_shown == FALSE)
    {
        sSC1PTrainingRemixExitBarWidth = 0;
        return;
    }
    sc1PTrainingRemixExitPutText();
    sSC1PTrainingRemixExitBarWidth =
        sc1PTrainingLeftoverRulesGetHoldBarWidth(sSC1PTrainingRemixExitBarWidth, (sc1PTrainingRemixExitIsAnyAHeld() != FALSE) ? 1 : 0);

    if (sSC1PTrainingRemixExitBarWidth > 0)
    {
        remix_hud_rect_put(SC1PTRAININGREMIXEXIT_BAR_SLOT, SC1PTRAININGREMIXEXIT_BAR_X, SC1PTRAININGREMIXEXIT_BAR_Y,
                           (f32)sSC1PTrainingRemixExitBarWidth, SC1PTRAININGREMIXEXIT_BAR_HEIGHT, SC1PTRAININGREMIXEXIT_BAR_COLOR);
    }
}

#endif
