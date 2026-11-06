/**
 * sc1premixretry.c - the L retry of the Remix bonus runs, see sc1premixretry.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scdef.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <sys/controller.h>
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixretry.h"
#include "sc/sc1pmode/sc1premixretryrules.h"

/* reset_flag of the asm */
static sb32 sSC1PRemixRetryIsPending = FALSE;

/** Buttons held on any controller, ORed (Joypad.check_buttons_all_ reads every port). */
static u32 sc1PRemixRetryGetButtonHold(void)
{
    u32 button_hold = 0;
    s32 i;

    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        button_hold |= gSYControllerDevices[i].button_hold;
    }
    return button_hold;
}

/** Whether a Bonus stage runs that was entered from the Bonus 1 or Bonus 2 CSS (Practice_1P.asm _check_bonus_not_1p). */
static sb32 sc1PRemixRetryIsBonusPracticeScene(void)
{
    if (gSCManagerSceneData.scene_curr != nSCKind1PBonusStage)
    {
        return FALSE;
    }
    return ((gSCManagerSceneData.scene_prev == nSCKind1PBonus1Players) || (gSCManagerSceneData.scene_prev == nSCKind1PBonus2Players)) ? TRUE : FALSE;
}

sb32 sc1PRemixRetryIsModeAvailable(void)
{
    return (sc1PRemixRetryIsFakeScreenMode(sc1PRemixModeGet()) != 0) ? TRUE : FALSE;
}

sb32 sc1PRemixRetryTryByPause(u16 button_tap)
{
    sb32 is_1p_game_scene = (gSCManagerSceneData.scene_curr == nSCKind1PGame) ? TRUE : FALSE;

    if (sc1PRemixRetryIsPauseRetry(sc1PRemixModeGet(), is_1p_game_scene, button_tap) == 0)
    {
        return FALSE;
    }
    sSC1PRemixRetryIsPending = TRUE;
    return TRUE;
}

sb32 sc1PRemixRetryTryByGameSet(void)
{
    sb32 is_1p_game_scene = (gSCManagerSceneData.scene_curr == nSCKind1PGame) ? TRUE : FALSE;

    if (sc1PRemixRetryIsGameSetRetry(sc1PRemixModeGet(), is_1p_game_scene, sc1PRemixRetryIsBonusPracticeScene(), sc1PRemixRetryGetButtonHold()) == 0)
    {
        return FALSE;
    }
    sSC1PRemixRetryIsPending = TRUE;
    return TRUE;
}

sb32 sc1PRemixRetryConsume(void)
{
    sb32 is_retry = sSC1PRemixRetryIsPending;

    sSC1PRemixRetryIsPending = FALSE;
    return is_retry;
}

#endif
