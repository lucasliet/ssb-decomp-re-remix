/**
 * gmremixgameend.c - VS match end of the Remix settings (GameEnd.asm update_screen_, TimedStock.asm show_full_results_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgameend.h"
#include "gm/gmremixtwelvecb.h"
#include "gm/gmremixtoggle.h"
#include <gr/grremix/grremixids.h>
#include <mn/mndef.h>
#include <sc/scdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/audio.h>
#include <sys/controller.h>

#define GMREMIX_GAMEEND_SALTY_SONG_FGM 0x566
#define GMREMIX_GAMEEND_SALTY_MASK_DEFAULT (A_BUTTON | B_BUTTON | Z_TRIG | R_TRIG | START_BUTTON)
#define GMREMIX_GAMEEND_SALTY_MASK_ALT (A_BUTTON | B_BUTTON | Z_TRIG | R_TRIG | L_JPAD)
#define GMREMIX_GAMEEND_RESULTS_MASK (R_TRIG | L_TRIG)
#define GMREMIX_GAMEEND_SALTY_TOGGLE_DEFAULT 1
#define GMREMIX_GAMEEND_SKIP_TOGGLE_ON 1

static sb32 sGMRemixGameEndIsSaltyRunback = FALSE;

/**
 * Whether any controller port holds one of the buttons (GameEnd.asm uses Joypad.check_buttons_all_ with "match any").
 *
 * @param mask Button mask of the controller.
 * @return TRUE when at least one held button of the mask is pressed.
 */
static sb32 gmRemixGameEndIsButtonHeld(u16 mask)
{
    s32 port;

    for (port = 0; port < ARRAY_COUNT(gSYControllerDevices); port++)
    {
        if ((gSYControllerDevices[port].button_hold & mask) != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Whether the salty runback button combination is held for the Salty Runback toggle (0 off, 1 default, 2 alt).
 *
 * @return TRUE when the toggle is on and its combination is held.
 */
static sb32 gmRemixGameEndIsSaltyCombinationHeld(void)
{
    s32 toggle = gmRemixToggle(salty_runback);
    u16 mask;

    if (toggle == 0)
    {
        return FALSE;
    }
    mask = (toggle == GMREMIX_GAMEEND_SALTY_TOGGLE_DEFAULT) ? GMREMIX_GAMEEND_SALTY_MASK_DEFAULT : GMREMIX_GAMEEND_SALTY_MASK_ALT;

    return gmRemixGameEndIsButtonHeld(mask);
}

/**
 * Starts a salty runback when its combination is held. Great Bay plays the "Song of Time" easter egg (FGM 0x566).
 *
 * @return TRUE when the match restarts.
 */
static sb32 gmRemixGameEndStartSaltyRunback(void)
{
    if (gmRemixGameEndIsSaltyCombinationHeld() == FALSE)
    {
        return FALSE;
    }
    if (gSCManagerSceneData.gkind == nGRRemixStage_GREAT_BAY)
    {
        syAudioPlayFGM(GMREMIX_GAMEEND_SALTY_SONG_FGM);
    }
    sGMRemixGameEndIsSaltyRunback = TRUE;

    return TRUE;
}

/**
 * Whether the results screen is skipped for the Skip Results Screen toggle (0 off, 1 on, 2 on reset).
 *
 * @return TRUE when the match goes to character select.
 */
static sb32 gmRemixGameEndCanSkipResults(void)
{
    s32 toggle = gmRemixToggle(skip_results_screen);

    if (toggle == 0)
    {
        return FALSE;
    }
    if (toggle == GMREMIX_GAMEEND_SKIP_TOGGLE_ON)
    {
        return TRUE;
    }
    return gSCManagerSceneData.is_reset != FALSE;
}

/**
 * Chooses the scene after a VS battle ends. Holding R or L always shows the results and cancels the salty runback, and the
 * 12 Character Battle never starts one (GameEnd.asm checks TwelveCharBattle.twelve_cb_flag).
 *
 * @return nSCKindVSBattle (salty runback), nSCKindPlayersVS (results skipped) or nSCKindVSResults.
 */
s32 gmRemixGameEndSelectScene(void)
{
    sGMRemixGameEndIsSaltyRunback = FALSE;

    if (gmRemixGameEndIsButtonHeld(GMREMIX_GAMEEND_RESULTS_MASK))
    {
        return nSCKindVSResults;
    }
    if ((gmRemixTwelveCBIsActive() == FALSE) && gmRemixGameEndStartSaltyRunback())
    {
        return nSCKindVSBattle;
    }
    if (gmRemixGameEndCanSkipResults())
    {
        return nSCKindPlayersVS;
    }
    return nSCKindVSResults;
}

/**
 * @return TRUE when the current match restarts a salty runback.
 */
sb32 gmRemixGameEndIsSaltyRunback(void)
{
    return sGMRemixGameEndIsSaltyRunback;
}

/**
 * @return TRUE when Toggles.entry_full_results is on.
 */
static sb32 gmRemixGameEndShowFullResults(void)
{
    return gmRemixToggle(full_results) != 0;
}

/**
 * @param kind sMNVSResultsKind.
 * @return The time layout for the stock kinds when full results are on, otherwise kind unchanged.
 * REMIX-TODO(results-mode-rows): the extra rows of GameEnd.asm add_row_to_results_ (VsRemixMenu.asm:1007: KOTH time,
 * Smasketball score and goals) are not ported, so those modes show only the vanilla rows.
 */
s32 gmRemixGameEndResultsDrawKind(s32 kind)
{
    if (gmRemixGameEndShowFullResults() == FALSE)
    {
        return kind;
    }
    if (kind == nMNVSResultsKindStockRoyal)
    {
        return nMNVSResultsKindTimeRoyal;
    }
    if (kind == nMNVSResultsKindStockTeam)
    {
        return nMNVSResultsKindTimeTeam;
    }
    return kind;
}

#endif
