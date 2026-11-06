/**
 * sc1premixmusic.c - BGM of the Remix 1P modes on the 1P game, see sc1premixmusic.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mp/map.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include <sys/utils.h>
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmusic.h"
#include "sc/sc1pmode/sc1premixmusicrules.h"

sb32 sc1PRemixMusicStartsAtOnce(void)
{
    return (sc1PRemixModePlaysMusicAtOnce(sc1PRemixModeGet(), gSCManagerSceneData.spgame_stage) != 0) ? TRUE : FALSE;
}

void sc1PRemixMusicApplyModeSong(void)
{
    s32 mode = sc1PRemixModeGet();
    s32 progress = gSCManagerSceneData.spgame_stage;
    s32 gkind = gSCManagerBattleState->gkind;
    s32 roll_count = sc1PRemixMusicRollCount(mode, progress, gkind);
    s32 roll = (roll_count != 0) ? syUtilsRandIntRange(roll_count) : 0;
    s32 bgm_id = sc1PRemixMusicPick(mode, progress, gkind, roll);

    if (bgm_id == SC1PREMIX_MUSIC_KEEP_STAGE)
    {
        return;
    }
    gMPCollisionGroundData->bgm_id = (u32)bgm_id;

    port_log("1p music: mode=%d stage=%d gkind=%d bgm=0x%X\n", (int)mode, (int)progress, (int)gkind, (unsigned int)bgm_id);
}

#endif
