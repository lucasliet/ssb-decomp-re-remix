/**
 * ftremixdpadmenu.c - Smash Remix css/DpadControl.asm css_cursor_and_dpad_controls_ (RAM 0x80004000), the hook that runs after every
 * controller read for every port: Stick Swap on the menus and on the Training screen, the stick Stickless clears in the unpaused
 * matches and on the title, the D-pad as a stick on the paused Training menu (Stickless) and the D-pad cursor of the character
 * select screens (Stickless, or the Dpad CSS Cursor Control toggle). The rules are in ftremixdpadmenurules.c; the fighter side of
 * the option is in ftremixdpad.c.
 *
 * The asm hook runs at the end of the controller read, after the held, pressed, repeat and release buttons and the stick are
 * computed, and the Single Button Mode filter follows it (the port filters later, see ftremixdpad.c).
 *
 * Training: the D-pad shortcuts of the unpaused Training match (reset DL, freeze DU, frame advance DR, model display DD, read by
 * sc1PTrainingRemixPollDpad from gSYControllerDevices) see Stick Swap because this hook swaps the Training screen even while the
 * match runs, and the DR turbo of DR_stick_training_turbo_timer (a repeat every 5th frame of a stick pushed right) is added to
 * the repeat buttons. ftRemixDpadControlController then must not swap a second time (ftRemixDpadMenuHasSwappedController). Every
 * other unpaused match is left to ftremixdpad.c, which swaps the fighter sample, because nothing else reads the D-pad there.
 * The paused Training menus (the vanilla one and the Z menu) read the stick of gSYControllerDevices, so the D-pad of Stickless
 * moves them through the stick values this hook writes.
 *
 * Differences: the pause state of the title is not read (the asm reads the match info byte; the port treats the title as
 * unpaused); the 1P game scene stands for the "1p battle" part of Global.screen.TITLE_AND_1P (the same mapping as the 1P test of
 * the Dream Land wind); and the screen 0x77 (Remix modes) never occurs because the port runs those modes on the VS scene.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <gm/gmdef.h>
#include <gm/gmremixpanel.h>
#include <gm/gmremixtoggle.h>
#include <sc/scdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "ft/ftremix/ftremixdpadcontrolrules.h"
#include "ft/ftremix/ftremixdpadmenu.h"
#include "ft/ftremix/ftremixdpadmenurules.h"

static s32 sFTRemixDpadMenuCursorTimers[GMCOMMON_PLAYERS_MAX];
static s32 sFTRemixDpadMenuTurboFrames[GMCOMMON_PLAYERS_MAX];
static sb32 sFTRemixDpadMenuTokenOverFighter[GMCOMMON_PLAYERS_MAX];

/** TRUE on the scenes that run a match (VS, 1P, Bonus and Training). */
static sb32 ftRemixDpadMenuIsMatchScene(u8 scene)
{
    return ((scene == nSCKindVSBattle) || (scene == nSCKind1PGame) || (scene == nSCKind1PBonusStage) || (scene == nSCKind1PTrainingMode)) ? TRUE : FALSE;
}

/** TRUE on the VS, 1P, Training, Bonus 1 and Bonus 2 character select screens (screen ids 0x10 to 0x14). */
static sb32 ftRemixDpadMenuIsCssScene(u8 scene)
{
    return ((scene >= nSCKindPlayersVS) && (scene <= nSCKind1PBonus2Players)) ? TRUE : FALSE;
}

/** TRUE on the Training scene. */
static sb32 ftRemixDpadMenuIsTrainingScene(u8 scene)
{
    return (scene == nSCKind1PTrainingMode) ? TRUE : FALSE;
}

static sb32 ftRemixDpadMenuIsPaused(void)
{
    return ((gSCManagerBattleState != NULL) && (gSCManagerBattleState->game_status == nSCBattleGameStatusPause)) ? TRUE : FALSE;
}

void ftRemixDpadMenuSetTokenOverFighter(s32 port, sb32 is_token_over_fighter)
{
    if ((port < 0) || (port >= GMCOMMON_PLAYERS_MAX))
    {
        return;
    }
    sFTRemixDpadMenuTokenOverFighter[port] = is_token_over_fighter;
}

static void ftRemixDpadMenuReadSample(const SYController *device, FTRemixDpadMenuSample *sample)
{
    sample->hold = device->button_hold;
    sample->tap = device->button_tap;
    sample->update = device->button_update;
    sample->stick_x = device->stick_range.x;
    sample->stick_y = device->stick_range.y;
}

static void ftRemixDpadMenuWriteSample(const FTRemixDpadMenuSample *sample, SYController *device)
{
    device->button_hold = (u16)sample->hold;
    device->button_tap = (u16)sample->tap;
    device->button_update = (u16)sample->update;
    device->stick_range.x = (s8)sample->stick_x;
    device->stick_range.y = (s8)sample->stick_y;
}

sb32 ftRemixDpadMenuHasSwappedController(void)
{
    return ftRemixDpadMenuIsTrainingScene(gSCManagerSceneData.scene_curr);
}

/** Reads the facts about the current screen that the rules need; the toggle is read only on a character select screen with the option off. */
static void ftRemixDpadMenuReadScreen(s32 mode, FTRemixDpadMenuScreen *screen)
{
    u8 scene = gSCManagerSceneData.scene_curr;

    screen->is_match_scene = ftRemixDpadMenuIsMatchScene(scene);
    screen->is_title_scene = (scene == nSCKindTitle) ? TRUE : FALSE;
    screen->is_paused = (screen->is_match_scene != FALSE) ? ftRemixDpadMenuIsPaused() : FALSE;
    screen->is_training = ftRemixDpadMenuIsTrainingScene(scene);
    screen->is_css_scene = ftRemixDpadMenuIsCssScene(scene);
    screen->is_cursor_toggle_on = ((mode == FTREMIX_DPAD_CONTROL_OFF) && (screen->is_css_scene != FALSE)) ? (gmRemixToggle(dpad_css_control) != 0) : FALSE;
}

void ftRemixDpadMenuTransform(s32 port, SYController *device, u16 prev_hold)
{
    FTRemixDpadMenuSample sample;
    FTRemixDpadMenuScreen screen;
    s32 mode;

    if ((port < 0) || (port >= GMCOMMON_PLAYERS_MAX))
    {
        return;
    }
    mode = gmRemixPanel(DPAD_CONTROL, port);

    ftRemixDpadMenuReadSample(device, &sample);
    ftRemixDpadMenuReadScreen(mode, &screen);

    switch (ftRemixDpadMenuPickAction(mode, &screen))
    {
    case nFTRemixDpadMenuActionSwap:
        ftRemixDpadMenuSwap(&sample, prev_hold);
        break;

    case nFTRemixDpadMenuActionSwapTraining:
        ftRemixDpadMenuSwapTraining(&sample, prev_hold, &sFTRemixDpadMenuTurboFrames[port]);
        break;

    case nFTRemixDpadMenuActionClearStick:
        ftRemixDpadMenuClearStick(&sample);
        break;

    case nFTRemixDpadMenuActionTrainingPause:
        ftRemixDpadMenuDpadAsStick(&sample);
        break;

    case nFTRemixDpadMenuActionCursor:
        ftRemixDpadMenuCursor(&sample, prev_hold, (mode != FTREMIX_DPAD_CONTROL_OFF) ? TRUE : FALSE, &sFTRemixDpadMenuCursorTimers[port],
                              sFTRemixDpadMenuTokenOverFighter[port]);
        break;

    default:
        return;
    }
    ftRemixDpadMenuWriteSample(&sample, device);
}

#endif
