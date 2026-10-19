/*
 * ftremixpause.c - Pause.asm settings of Smash Remix (.toolchain/pkgs/smashremix-src/src/Pause.asm).
 * Hold To Pause (toggle entry_hold_to_pause): ifCommonBattleGoUpdateInterface pauses only after START is held for longer than
 * the hold window. Camera Mode (toggle entry_camera_mode, SCENE): ifCommonBattlePauseUpdateInterface keeps the scene camera
 * rotation between pauses. Both entry points are no-ops while their setting is off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/os.h>
#include <ssb_types.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"

/* Pause.asm NUM_FRAMES: START must be held for more frames than this to pause */
#define FTREMIX_PAUSE_HOLD_FRAMES 30
/* Pause.asm input_table: one counter byte per port */
#define FTREMIX_PAUSE_PLAYERS 4
/* Camera.asm type.SCENE */
#define FTREMIX_PAUSE_CAMERA_SCENE 3

static s32 sFTRemixPauseHoldFrames[FTREMIX_PAUSE_PLAYERS];

/* Pause.asm hold_ (0x8011408C, ifCommonBattleGoUpdateInterface): TRUE when START pauses the match for this player */
sb32 ftRemixPauseStartRequested(s32 player, u16 button_tap, u16 button_hold)
{
    if (!gmRemixToggle(hold_to_pause))
    {
        return (button_tap & START_BUTTON) != 0;
    }
    if (!(button_hold & START_BUTTON))
    {
        sFTRemixPauseHoldFrames[player] = 0;
        return FALSE;
    }
    sFTRemixPauseHoldFrames[player]++;
    if (sFTRemixPauseHoldFrames[player] > FTREMIX_PAUSE_HOLD_FRAMES)
    {
        sFTRemixPauseHoldFrames[player] = 0;
        return TRUE;
    }
    return FALSE;
}

/* Pause.asm extra_controls_ _camera_controls (0x80114320, ifCommonBattlePauseUpdateInterface): TRUE when the scene camera
 * keeps its rotation between pauses. The caller only asks while the pause is the default one. */
sb32 ftRemixPauseKeepsSceneCamera(void)
{
    return gmRemixToggle(camera_mode) == FTREMIX_PAUSE_CAMERA_SCENE;
}

#endif
