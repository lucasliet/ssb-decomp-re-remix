/**
 * sc1premixmultiman.c - Multi-Man and Cruel Multi-Man rules on the 1P game, see sc1premixmultiman.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/scene.h>
#include <sys/utils.h>
#include <lb/library.h>
#include <lb/lbbackup.h>
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixhighscores.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmoderules.h"
#include "sc/sc1pmode/sc1premixmultiman.h"
#include "gm/gmremixtoggle.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"

#define SC1PREMIX_MULTIMAN_POLYGON_KINDS 12
#define SC1PREMIX_MULTIMAN_STOCK_COUNT 0
#define SC1PREMIX_MULTIMAN_HUD_SLOT 28
#define SC1PREMIX_MULTIMAN_HUD_X 50.0F
#define SC1PREMIX_MULTIMAN_POLYGON_X 30.0F
#define SC1PREMIX_MULTIMAN_CROSS_X 39.0F
#define SC1PREMIX_MULTIMAN_ICON_TINT 0x848484FF
#define SC1PREMIX_MULTIMAN_ICON_PALETTE 0x303030FF
#define SC1PREMIX_MULTIMAN_GROUP 0x0B
#define SC1PREMIX_MULTIMAN_DL_LINK 23
#define SC1PREMIX_MULTIMAN_HUD_SCALE 0.9F
#define SC1PREMIX_MULTIMAN_HUD_COLOR 0xFFFFFFFF

static SC1PRemixMultimanKos sSC1PRemixMultimanKos;
static f32 sSC1PRemixMultimanCounterY;

s32 sc1PRemixMultimanFirstStage(s32 mode)
{
    if (mode == SC1PREMIX_MODE_BONUS3)
    {
        return nSC1PGameStageBonus3;
    }
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

void sc1PRemixMultimanSetupHud(void)
{
    f32 polygon_y;
    f32 cross_y;

    if (sc1PRemixMultimanIsActive() == FALSE)
    {
        return;
    }
    sc1PRemixMultimanHudY(gmRemixToggle(dragon_king_hud), &polygon_y, &cross_y, &sSC1PRemixMultimanCounterY);

    ifRemixSpriteMake(IFREMIX_SPRITE_MULTIMAN_POLYGON, SC1PREMIX_MULTIMAN_GROUP, SC1PREMIX_MULTIMAN_DL_LINK,
                      SC1PREMIX_MULTIMAN_POLYGON_X, polygon_y, SC1PREMIX_MULTIMAN_ICON_TINT, SC1PREMIX_MULTIMAN_ICON_PALETTE, 1.0F);
    ifRemixSpriteMake(IFREMIX_SPRITE_MULTIMAN_CROSS, SC1PREMIX_MULTIMAN_GROUP, SC1PREMIX_MULTIMAN_DL_LINK,
                      SC1PREMIX_MULTIMAN_CROSS_X, cross_y, SC1PREMIX_MULTIMAN_ICON_TINT, SC1PREMIX_MULTIMAN_ICON_PALETTE, 1.0F);
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

static u16* sc1PRemixMultimanRecordSlot(s32 fkind)
{
    if ((fkind < 0) || (fkind >= ARRAY_COUNT(gSCManagerBackupData.spgame_records)))
    {
        return NULL;
    }
    if (sc1PRemixModeIs(SC1PREMIX_MODE_CRUEL) != FALSE)
    {
        return &gSCManagerBackupData.spgame_records[fkind].cruel_kos;
    }
    return &gSCManagerBackupData.spgame_records[fkind].multiman_kos;
}

u32 sc1PRemixMultimanRecordGet(s32 fkind)
{
    u16 *record = sc1PRemixMultimanRecordSlot(fkind);

    return (record != NULL) ? (u32)*record : 0;
}

/* _ko_save: the run is kept when its KOs pass the record of the fighter, once the fighter loses its stock. */
static void sc1PRemixMultimanSaveRecord(void)
{
    u16 *record = sc1PRemixMultimanRecordSlot(gSCManagerSceneData.fkind);

    if ((record == NULL) || (sSC1PRemixMultimanKos.kos <= (s32)*record))
    {
        return;
    }
    if (sc1PRemixHighScoresCanSaveKoRecord() == FALSE)
    {
        return;
    }
    *record = (u16)sSC1PRemixMultimanKos.kos;
    lbBackupWrite();

    port_log("multiman: record saved, fkind=%d kos=%d\n", (int)gSCManagerSceneData.fkind, (int)*record);
}

void sc1PRemixMultimanOnPlayerKo(void)
{
    sc1PRemixMultimanKosSetPlayerKo(&sSC1PRemixMultimanKos);
    port_log("multiman: player KO, kos=%d\n", (int)sSC1PRemixMultimanKos.kos);
    sc1PRemixMultimanSaveRecord();
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
    remix_hud_text_putf(SC1PREMIX_MULTIMAN_HUD_SLOT, SC1PREMIX_MULTIMAN_HUD_X, sSC1PRemixMultimanCounterY,
                        SC1PREMIX_MULTIMAN_HUD_COLOR, SC1PREMIX_MULTIMAN_HUD_SCALE, REMIX_HUD_ALIGN_LEFT, "%d",
                        (int)sSC1PRemixMultimanKos.kos);
}

#endif
