/**
 * gmremixpractice.c - Remix Practice_1P panel option (Practice_1P.asm): the start stage of a 1P Game run
 * (_1p_practice), the practice state of the run (practice_active) and the L retry (hold_l_to_retry_,
 * hold_l_to_retry_stage_clear_, the pause menu L of SinglePlayerModes.asm _reset).
 *
 * A retry replays the stage without a continue, a level drop or the stage preview: the retry triggers set a flag and end
 * the scene, and sc1PManagerUpdateScene consumes it (gmRemixPracticeConsumeRetry).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scdef.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <sys/controller.h>
#include "gm/gmremixpractice.h"
#include "gm/gmremixpracticerules.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "remix/remix_player_panel.h"

#define GMREMIX_PRACTICE_OFF 0

/* practice_active and reset_flag of the asm. */
static sb32 sGMRemixPracticeIsActive = FALSE;
static sb32 sGMRemixPracticeIsRetry = FALSE;
static sb32 sGMRemixPracticeIsIntroSkipped = FALSE;

/* Practice value 1..11 (Stage 1..10, Final Stage) to nSC1PGameStage. The asm skips the bonus stages (Practice_1P.asm 41). */
static const s32 sGMRemixPracticeStages[] = {
    nSC1PGameStageLink,
    nSC1PGameStageYoshi,
    nSC1PGameStageFox,
    nSC1PGameStageMario,
    nSC1PGameStagePikachu,
    nSC1PGameStageDonkey,
    nSC1PGameStageKirby,
    nSC1PGameStageSamus,
    nSC1PGameStageMMario,
    nSC1PGameStageZako,
    nSC1PGameStageBoss,
};

/**
 * Stage of a 1P Game run. The option is read straight from the registry because the start runs before the battle state
 * holds the players, so the human-only guard of gmRemixPanel cannot be applied here.
 *
 * @param player Port index of the human fighter (0..3).
 * @param stage nSC1PGameStage the run starts on without the option.
 * @return The practice stage, or `stage` when the option is off.
 */
s32 gmRemixPracticeStartStage(s32 player, s32 stage)
{
    s32 practice_value = remix_panel_get(REMIX_PANEL_PRACTICE_1P, player);

    if (gmRemixPracticeIsRunActive(sc1PRemixModeGet(), practice_value) == 0)
    {
        return stage;
    }
    return sGMRemixPracticeStages[practice_value - 1];
}

/**
 * Decides whether the 1P run that starts is a practice run (Practice_1P.asm _check_mode) and clears the retry state. Called
 * at the start of sc1PManagerUpdateScene.
 *
 * @param player Port index of the human fighter (0..3).
 */
void gmRemixPracticeBegin(s32 player)
{
    sGMRemixPracticeIsActive = (gmRemixPracticeIsRunActive(sc1PRemixModeGet(), remix_panel_get(REMIX_PANEL_PRACTICE_1P, player)) != 0) ? TRUE : FALSE;
    sGMRemixPracticeIsRetry = FALSE;
    sGMRemixPracticeIsIntroSkipped = FALSE;
}

/**
 * TRUE while the 1P run is a practice run (practice_active).
 */
sb32 gmRemixPracticeIsActive(void)
{
    return sGMRemixPracticeIsActive;
}

/**
 * Whether one of the controllers holds L and nothing else (Joypad.check_buttons_all_ checks each port by itself).
 */
static sb32 gmRemixPracticeIsRetryHeldOnAnyController(void)
{
    s32 i;

    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        if (gmRemixPracticeIsRetryHeld(gSYControllerDevices[i].button_hold) != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * Starts a retry when the run is a practice run and one controller holds L alone (hold_l_to_retry_ and
 * hold_l_to_retry_stage_clear_). The caller ends its scene.
 *
 * @return TRUE when the retry started.
 */
sb32 gmRemixPracticeTryRetryByHold(void)
{
    if ((sGMRemixPracticeIsActive == FALSE) || (gmRemixPracticeIsRetryHeldOnAnyController() == FALSE))
    {
        return FALSE;
    }
    sGMRemixPracticeIsRetry = TRUE;
    return TRUE;
}

/**
 * The L retry while GAME SET shows (hold_l_to_retry_): in the 1P Game battle and in a Bonus stage of the 1P run, whose previous
 * scene is not a Bonus CSS. The caller ends its scene.
 *
 * @return TRUE when the retry started.
 */
sb32 gmRemixPracticeTryRetryByGameSet(void)
{
    sb32 is_1p_game_scene = (gSCManagerSceneData.scene_curr == nSCKind1PGame) ? TRUE : FALSE;
    sb32 is_bonus_stage_scene = (gSCManagerSceneData.scene_curr == nSCKind1PBonusStage) ? TRUE : FALSE;
    sb32 is_from_bonus_css = ((gSCManagerSceneData.scene_prev == nSCKind1PBonus1Players) || (gSCManagerSceneData.scene_prev == nSCKind1PBonus2Players)) ? TRUE : FALSE;

    if (gmRemixPracticeIsGameSetRetryScene(is_1p_game_scene, is_bonus_stage_scene, is_from_bonus_css) == 0)
    {
        return FALSE;
    }
    return gmRemixPracticeTryRetryByHold();
}

/**
 * Starts a retry from the pause menu (SinglePlayerModes.asm _reset, `_practice`) when L is pressed in a practice run of the
 * 1P Game scene. The caller ends its scene.
 *
 * @param button_tap Buttons pressed this frame by the pausing player.
 * @return TRUE when the retry started.
 */
sb32 gmRemixPracticeTryRetryByPause(u16 button_tap)
{
    sb32 is_1p_game_scene = (gSCManagerSceneData.scene_curr == nSCKind1PGame) ? TRUE : FALSE;

    if (gmRemixPracticeIsPauseRetry(sGMRemixPracticeIsActive, is_1p_game_scene, button_tap) == 0)
    {
        return FALSE;
    }
    sGMRemixPracticeIsRetry = TRUE;
    return TRUE;
}

/**
 * TRUE once per retry (reset_flag): the manager replays the stage and skips the stage preview of the replay.
 */
sb32 gmRemixPracticeConsumeRetry(void)
{
    if (sGMRemixPracticeIsRetry == FALSE)
    {
        return FALSE;
    }
    sGMRemixPracticeIsRetry = FALSE;
    sGMRemixPracticeIsIntroSkipped = TRUE;
    return TRUE;
}

/**
 * TRUE once after a retry: the stage preview is skipped (SinglePlayerModes.asm _preview_skip and _preview_song_skip).
 */
sb32 gmRemixPracticeConsumeIntroSkip(void)
{
    sb32 is_skipped = sGMRemixPracticeIsIntroSkipped;

    sGMRemixPracticeIsIntroSkipped = FALSE;
    return is_skipped;
}

#endif
