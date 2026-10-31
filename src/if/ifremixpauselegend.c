/**
 * ifremixpauselegend.c - Pause legends of Smash Remix (Pause.asm show_legend_helper_, check_toggle_hud_press_,
 * check_legend_toggle_press_ and disable_hud_, Render groups 0xE and 0x10).
 *
 * The port has no Render groups, so the legends are drawn with remix_hud_text every frame the pause update runs. A label
 * stops being drawn once its caller stops putting it, so leaving the pause clears the legends without a destroy call.
 *
 * Not ported: the music legend of group 0xF (track title and the Next/Random rows), which belongs to the alternate pause
 * music. The button glyphs of the asm are image textures, so the port draws them as text.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/os.h>
#include <ssb_types.h>
#include "gm/gmremixtoggle.h"
#include "if/ifremixpauselegend.h"
#include "remix/remix_hud_text.h"

/* remix_hud_text slots 20 to 23: free in battle (the other Remix HUD texts use 24 to 29) */
#define IFREMIX_PAUSE_LEGEND_SLOT_R 20
#define IFREMIX_PAUSE_LEGEND_SLOT_CAMERA_ZOOM 21
#define IFREMIX_PAUSE_LEGEND_SLOT_CAMERA_PAN 22
#define IFREMIX_PAUSE_LEGEND_SLOT_CAMERA_FOV 23

/* Pause.asm Render.draw_string positions, converted to centre coordinates with an estimated glyph width */
#define IFREMIX_PAUSE_LEGEND_HEIGHT 8.0F
#define IFREMIX_PAUSE_LEGEND_COLOR 0xFFFFFFFF
#define IFREMIX_PAUSE_LEGEND_R_X 262.0F
#define IFREMIX_PAUSE_LEGEND_R_Y 229.0F
#define IFREMIX_PAUSE_LEGEND_ZOOM_X 72.0F
#define IFREMIX_PAUSE_LEGEND_ZOOM_Y 144.0F
#define IFREMIX_PAUSE_LEGEND_PAN_X 74.0F
#define IFREMIX_PAUSE_LEGEND_PAN_Y 158.0F
#define IFREMIX_PAUSE_LEGEND_FOV_X 80.0F
#define IFREMIX_PAUSE_LEGEND_FOV_Y 174.0F

/* Pause.asm group 0xE (R legend) and group 0x10 (camera legend) show state, both set in ifRemixPauseLegendReset */
static sb32 sIFRemixPauseLegendHudShown;
static sb32 sIFRemixPauseLegendCameraShown;

/**
 * Resets the pause legends for a new pause (Pause.asm: the R legend is created shown, the camera legend is created hidden).
 */
void ifRemixPauseLegendReset(void)
{
    sIFRemixPauseLegendHudShown = TRUE;
    sIFRemixPauseLegendCameraShown = FALSE;
}

/**
 * Checks whether the R legend is drawn: the Disable HUD toggle is OFF (disable_hud_ hides group 0xE for PAUSE and ALL)
 * and Dpad Left has not hidden it.
 *
 * @return TRUE when the R legend and, if shown, the camera legend are drawn.
 */
static sb32 ifRemixPauseLegendHudVisible(void)
{
    return (gmRemixToggle(disable_hud) == 0) && sIFRemixPauseLegendHudShown;
}

/**
 * Toggles the R legend with Dpad Left while the Disable HUD toggle is OFF (Pause.asm check_toggle_hud_press_). Hiding
 * it also hides the camera legend.
 *
 * @param button_tap Buttons pressed this frame.
 */
static void ifRemixPauseLegendToggleHudOnDpadLeft(u16 button_tap)
{
    if ((gmRemixToggle(disable_hud) != 0) || (button_tap != L_JPAD))
    {
        return;
    }
    sIFRemixPauseLegendHudShown = !sIFRemixPauseLegendHudShown;
    sIFRemixPauseLegendCameraShown = FALSE;
}

/**
 * Toggles the camera legend on a solo R press while the R legend is drawn (Pause.asm check_legend_toggle_press_).
 *
 * @param button_tap Buttons pressed this frame.
 * @param button_hold Buttons held this frame.
 */
static void ifRemixPauseLegendToggleOnRPress(u16 button_tap, u16 button_hold)
{
    if ((button_tap != R_TRIG) || (button_hold != R_TRIG))
    {
        return;
    }
    if (!ifRemixPauseLegendHudVisible())
    {
        return;
    }
    sIFRemixPauseLegendCameraShown = !sIFRemixPauseLegendCameraShown;
}

/**
 * Puts one camera legend row, the Pause.asm group 0x10 strings in the port text form.
 *
 * @param slot remix_hud_text slot of the row.
 * @param x Horizontal centre of the row.
 * @param y Vertical centre of the row.
 * @param text Row text.
 */
static void ifRemixPauseLegendPutCameraRow(s32 slot, f32 x, f32 y, const char *text)
{
    remix_hud_text_put(slot, text, x, y, IFREMIX_PAUSE_LEGEND_COLOR, IFREMIX_PAUSE_LEGEND_HEIGHT);
}

/**
 * Puts the camera legend rows (Pause.asm group 0x10: zoom on A/B, pan on the D-Pad, field of view on Z+A/B).
 */
static void ifRemixPauseLegendPutCamera(void)
{
    ifRemixPauseLegendPutCameraRow(IFREMIX_PAUSE_LEGEND_SLOT_CAMERA_ZOOM, IFREMIX_PAUSE_LEGEND_ZOOM_X,
                                   IFREMIX_PAUSE_LEGEND_ZOOM_Y, "A/B: Zoom Camera");
    ifRemixPauseLegendPutCameraRow(IFREMIX_PAUSE_LEGEND_SLOT_CAMERA_PAN, IFREMIX_PAUSE_LEGEND_PAN_X,
                                   IFREMIX_PAUSE_LEGEND_PAN_Y, "D-Pad: Pan Camera");
    ifRemixPauseLegendPutCameraRow(IFREMIX_PAUSE_LEGEND_SLOT_CAMERA_FOV, IFREMIX_PAUSE_LEGEND_FOV_X,
                                   IFREMIX_PAUSE_LEGEND_FOV_Y, "Z+A/B: Field of View");
}

/**
 * Puts the R legend label and, when it is shown, the camera legend rows. Labels that are not put stop being drawn on
 * their own, so hidden legends need no explicit clear.
 */
static void ifRemixPauseLegendDraw(void)
{
    if (!ifRemixPauseLegendHudVisible())
    {
        return;
    }
    remix_hud_text_put(IFREMIX_PAUSE_LEGEND_SLOT_R, "R: Legend", IFREMIX_PAUSE_LEGEND_R_X, IFREMIX_PAUSE_LEGEND_R_Y,
                       IFREMIX_PAUSE_LEGEND_COLOR, IFREMIX_PAUSE_LEGEND_HEIGHT);
    if (sIFRemixPauseLegendCameraShown)
    {
        ifRemixPauseLegendPutCamera();
    }
}

/**
 * Runs the pause legend controls of the frame, then draws the legends (Pause.asm extra_controls_ order: Dpad Left, then R).
 *
 * @param button_tap Buttons pressed this frame of the pausing player.
 * @param button_hold Buttons held this frame of the pausing player.
 */
void ifRemixPauseLegendUpdate(u16 button_tap, u16 button_hold)
{
    ifRemixPauseLegendToggleHudOnDpadLeft(button_tap);
    ifRemixPauseLegendToggleOnRPress(button_tap, button_hold);
    ifRemixPauseLegendDraw();
}

#endif
