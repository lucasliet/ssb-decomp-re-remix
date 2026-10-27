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

#define SC1PREMIX_BONUSEXIT_TIME_LIMIT 5
#define SC1PREMIX_BONUSEXIT_FIRST_STAGE 0

static void sc1PRemixModeAdvanceSeed(void)
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
    gSCManagerSceneData.spgame_stage = SC1PREMIX_BONUSEXIT_FIRST_STAGE;
    lbBackupWrite();
    return TRUE;
}

#endif
