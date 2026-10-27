/**
 * sc1premixmultiman.c - Multi-Man and Cruel Multi-Man rules on the 1P game, see sc1premixmultiman.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include <sys/utils.h>
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmoderules.h"
#include "sc/sc1pmode/sc1premixmultiman.h"
#include "remix/remix_hud_text.h"

#define SC1PREMIX_MULTIMAN_POLYGON_KINDS 12
#define SC1PREMIX_MULTIMAN_STOCK_COUNT 0
#define SC1PREMIX_MULTIMAN_HUD_SLOT 28
#define SC1PREMIX_MULTIMAN_HUD_X 48.0F
#define SC1PREMIX_MULTIMAN_HUD_Y 22.0F
#define SC1PREMIX_MULTIMAN_HUD_HEIGHT 16.0F
#define SC1PREMIX_MULTIMAN_HUD_COLOR 0xFFFFFFFF

static SC1PRemixMultimanKos sSC1PRemixMultimanKos;

s32 sc1PRemixMultimanFirstStage(s32 mode)
{
    return (sc1PRemixModeIsMultiman(mode) != 0) ? nSC1PGameStageZako : 0;
}

sb32 sc1PRemixMultimanIsActive(void)
{
    if (sc1PRemixModeIsMultiman(sc1PRemixModeGet()) == 0)
    {
        return FALSE;
    }
    return (gSCManagerSceneData.spgame_stage == nSC1PGameStageZako) ? TRUE : FALSE;
}

void sc1PRemixMultimanSetupBattle(void)
{
    if (sc1PRemixMultimanIsActive() == FALSE)
    {
        return;
    }
    sc1PRemixMultimanKosReset(&sSC1PRemixMultimanKos);

    gSCManagerBattleState->is_team_attack = FALSE;
    gSCManagerBattleState->item_appearance_rate = 0;
    gSCManagerBattleState->time_limit = SCBATTLE_TIMELIMIT_INFINITE;
}

void sc1PRemixMultimanSetupPlayer(s32 player)
{
    s32 handicap;

    if (sc1PRemixMultimanIsActive() == FALSE)
    {
        return;
    }
    handicap = sc1PRemixModeHumanHandicap(sc1PRemixModeGet());

    if (handicap != 0)
    {
        gSCManager1PGameBattleState.players[player].handicap = handicap;
    }
    gSCManager1PGameBattleState.players[player].stock_count = SC1PREMIX_MULTIMAN_STOCK_COUNT;
}

void sc1PRemixMultimanSetupEnemy(s32 player)
{
    if (sc1PRemixMultimanIsActive() == FALSE)
    {
        return;
    }
    gSCManagerBattleState->players[player].handicap = sc1PRemixModeEnemyHandicap(sc1PRemixModeGet());
}

s32 sc1PRemixMultimanPolygonKind(void)
{
    return nFTKindNStart + syUtilsRandIntRange(SC1PREMIX_MULTIMAN_POLYGON_KINDS);
}

void sc1PRemixMultimanOnEnemyKo(void)
{
    sc1PRemixMultimanKosAddEnemy(&sSC1PRemixMultimanKos);
    port_log("multiman: enemy KO, kos=%d player_ko=%d\n", (int)sSC1PRemixMultimanKos.kos, (int)sSC1PRemixMultimanKos.is_player_ko);
}

void sc1PRemixMultimanOnPlayerKo(void)
{
    sc1PRemixMultimanKosSetPlayerKo(&sSC1PRemixMultimanKos);
    port_log("multiman: player KO, kos=%d\n", (int)sSC1PRemixMultimanKos.kos);
}

s32 sc1PRemixMultimanGetKos(void)
{
    return sSC1PRemixMultimanKos.kos;
}

void sc1PRemixMultimanLeaveRun(void)
{
    sb32 is_restart = sc1PRemixModeLeaveRun();

    port_log("multiman: run ended, kos=%d restart=%d\n", (int)sSC1PRemixMultimanKos.kos, (int)is_restart);
}

void sc1PRemixMultimanDrawHud(void)
{
    if (sc1PRemixMultimanIsActive() == FALSE)
    {
        return;
    }
    remix_hud_text_putf(SC1PREMIX_MULTIMAN_HUD_SLOT, SC1PREMIX_MULTIMAN_HUD_X, SC1PREMIX_MULTIMAN_HUD_Y,
                        SC1PREMIX_MULTIMAN_HUD_COLOR, SC1PREMIX_MULTIMAN_HUD_HEIGHT, "KOs %d",
                        (int)sSC1PRemixMultimanKos.kos);
}

#endif
