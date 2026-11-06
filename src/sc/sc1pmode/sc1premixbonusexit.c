/**
 * sc1premixbonusexit.c - exit of the Bonus 2 CSS into the 1P game for the Remix 1P modes, see sc1premixbonusexit.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <lb/lbbackup.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include <sys/taskman.h>
#include <sys/utils.h>
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmoderules.h"
#include "sc/sc1pmode/sc1premixmultiman.h"
#include "sc/sc1pmode/sc1premixretry.h"

#define SC1PREMIX_BONUSEXIT_TIME_LIMIT 5

void sc1PRemixModeAdvanceSeed(void)
{
    u32 steps = sc1PRemixModeSeedAdvanceCount(dSYTaskmanFrameCount);

    while (steps-- != 0)
    {
        syUtilsRandUShort();
    }
}

sb32 sc1PRemixModeExitBonusCSS(s32 fkind, s32 costume)
{
    s32 mode = sc1PRemixModeGet();

    if (mode == SC1PREMIX_MODE_STANDARD)
    {
        return FALSE;
    }
    sc1PRemixModeAdvanceSeed();

    if (sc1PRemixModeLeavesBonusCSSFor1P(mode) == 0)
    {
        return FALSE;
    }
    gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
    gSCManagerSceneData.scene_curr = nSCKind1PGame;
    gSCManagerSceneData.fkind = fkind;
    gSCManagerSceneData.costume = costume;
    gSCManagerSceneData.spgame_time_limit = SC1PREMIX_BONUSEXIT_TIME_LIMIT;
    gSCManagerSceneData.spgame_stage = sc1PRemixMultimanFirstStage(mode);
    lbBackupWrite();
    return TRUE;
}

sb32 sc1PRemixModeLeaveRun(void)
{
    sb32 is_restart = sc1PRemixRetryConsume();

    gSCManagerSceneData.is_reset = FALSE;
    gSCManagerSceneData.scene_prev = nSCKind1PGame;
    gSCManagerSceneData.scene_curr = (is_restart != FALSE) ? nSCKind1PGame : nSCKind1PBonus2Players;

    return is_restart;
}

#endif
