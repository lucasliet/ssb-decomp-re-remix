/**
 * sc1premix1p.c - random path of Remix 1P, see sc1premix1p.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <port_log.h>
#include <sys/utils.h>
#include <sc/scene.h>
#include "ft/ftremix/ftremixhooks.h"
#include "gr/grremix/grremixids.h"
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1premix1p.h"
#include "sc/sc1pmode/sc1premix1prules.h"
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmodename.h"

#define SC1PREMIX_1P_STAGE_ID_COUNT 256
#define SC1PREMIX_1P_DUO_SLOT 4
#define SC1PREMIX_1P_TEAM_SLOT 1
#define SC1PREMIX_1P_GIANT_SLOT 6
#define SC1PREMIX_1P_TINY_TEAM_SLOT 9
#define SC1PREMIX_1P_BOSS_SLOT 10
#define SC1PREMIX_1P_KIRBY_RANDOM_COUNT 7
#define SC1PREMIX_1P_KIRBY_MAGIC_HAT_POWER 0x1C
#define SC1PREMIX_1P_KIRBY_MAGIC_HAT 0x25

/* Literals of the boss branches of remix_1p_randomization and the name textures of SinglePlayer.asm (each one + 0x10 for the file header). */
#define SC1PREMIX_1P_NAME_TEXTURE_HEADER 0x10
#define SC1PREMIX_1P_NAME_TEXTURE_PEACH 0xC558
#define SC1PREMIX_1P_NAME_TEXTURE_PIANO 0x5A08
#define SC1PREMIX_1P_NAME_TEXTURE_METAL_MARIO_BROS 0xADA8
#define SC1PREMIX_1P_NAME_TEXTURE_SSONIC 0x6918
#define SC1PREMIX_1P_ICON_PIANO 0x15240
#define SC1PREMIX_1P_ICON_METAL_MARIO_BROS (0x17128 + 0x10)
#define SC1PREMIX_1P_ICON_SSONIC 0x16000
#define SC1PREMIX_1P_CALL_PIANO 0x3C6
#define SC1PREMIX_1P_CALL_METAL_MARIO_BROS 0x3C6
#define SC1PREMIX_1P_CALL_SSONIC 0x3FE
#define SC1PREMIX_1P_SCALE_BOSS 0x7070
#define SC1PREMIX_1P_SCALE_SSONIC 0x6F80

static SC1PGameStage sSC1PRemix1PStages[SC1PREMIX_1P_STAGE_COUNT];
static SC1PGameComputer sSC1PRemix1PComputers[SC1PREMIX_1P_STAGE_COUNT];
static SC1PRemix1PSlotCard sSC1PRemix1PCards[SC1PREMIX_1P_PATH_COUNT];
static s32 sSC1PRemix1PKirbyPowers[SC1PREMIX_1P_KIRBY_RANDOM_COUNT + 1];
static s32 sSC1PRemix1PKirbyHats[SC1PREMIX_1P_KIRBY_RANDOM_COUNT + 1];
static sb32 sSC1PRemix1PIsReady;
static sb32 sSC1PRemix1PIsDuoPartnerFirst;
static sb32 sSC1PRemix1PIsMetalBros;

static int sc1PRemix1PRandom(int range)
{
    return (int)syUtilsRandIntRange(range);
}

static void sc1PRemix1PLoadTables(void)
{
    s32 i;

    for (i = 0; i < SC1PREMIX_1P_STAGE_COUNT; i++)
    {
        sc1PRemix1PDataReadStage(i, &sSC1PRemix1PStages[i]);
        sc1PRemix1PDataReadComputer(i, &sSC1PRemix1PComputers[i]);
    }
    for (i = 0; i < SC1PREMIX_1P_PATH_COUNT; i++)
    {
        sc1PRemix1PDataReadSlotCard(i, &sSC1PRemix1PCards[i]);
    }
    sSC1PRemix1PIsDuoPartnerFirst = FALSE;
    sSC1PRemix1PIsMetalBros = FALSE;
}

static void sc1PRemix1PPickKirbys(void)
{
    int picks[SC1PREMIX_1P_KIRBY_RANDOM_COUNT];
    s32 i;

    sc1PRemix1PPickDistinct(sc1PRemix1PRandom, SC1PREMIX_1P_KIRBY_POOL_COUNT, picks, SC1PREMIX_1P_KIRBY_RANDOM_COUNT);

    for (i = 0; i < SC1PREMIX_1P_KIRBY_RANDOM_COUNT; i++)
    {
        sc1PRemix1PDataReadKirby(picks[i], &sSC1PRemix1PKirbyPowers[i], &sSC1PRemix1PKirbyHats[i]);
    }
    sSC1PRemix1PKirbyPowers[SC1PREMIX_1P_KIRBY_RANDOM_COUNT] = SC1PREMIX_1P_KIRBY_MAGIC_HAT_POWER;
    sSC1PRemix1PKirbyHats[SC1PREMIX_1P_KIRBY_RANDOM_COUNT] = SC1PREMIX_1P_KIRBY_MAGIC_HAT;
}

static void sc1PRemix1PSetSlot(s32 slot, s32 gkind, s32 fkind)
{
    sSC1PRemix1PStages[slot].gkind = gkind;
    sSC1PRemix1PStages[slot].fkind[0] = fkind;
    sSC1PRemix1PCards[slot].title_fkind = fkind;
}

static void sc1PRemix1PSetCard(s32 slot, const SC1PRemix1PFighter *fighter)
{
    SC1PRemix1PSlotCard *card = &sSC1PRemix1PCards[slot];

    card->progress_icon = fighter->progress_icon;
    card->name_texture = fighter->name_texture;

    if (slot != SC1PREMIX_1P_TINY_TEAM_SLOT)
    {
        card->announcer_call = fighter->announcer_call;
    }
    if (slot != SC1PREMIX_1P_TEAM_SLOT)
    {
        card->model_scale = fighter->model_scale;
    }
    if ((fighter->fkind == REMIX_ID_PEACH) && ((slot == SC1PREMIX_1P_TEAM_SLOT) || (slot == SC1PREMIX_1P_GIANT_SLOT)))
    {
        card->name_texture = SC1PREMIX_1P_NAME_TEXTURE_PEACH + SC1PREMIX_1P_NAME_TEXTURE_HEADER;
    }
}

static void sc1PRemix1PPickDuo(int *is_fighter_used, int *is_stage_used)
{
    SC1PRemix1PDuo duo;
    SC1PRemix1PSlotCard *card = &sSC1PRemix1PCards[SC1PREMIX_1P_DUO_SLOT];
    s32 stage;
    s32 partner_index;

    sc1PRemix1PDataReadDuo(sc1PRemix1PRandom(SC1PREMIX_1P_DUO_COUNT), &duo);

    if ((duo.main_pool_index >= 0) && (duo.main_pool_index < SC1PREMIX_1P_POOL_COUNT))
    {
        is_fighter_used[duo.main_pool_index] = 1;
    }
    if (duo.main_fkind == REMIX_ID_BANJO)
    {
        partner_index = sc1PRemix1PDataFindFighter(REMIX_ID_CONKER);

        if (partner_index >= 0)
        {
            is_fighter_used[partner_index] = 1;
        }
    }
    sSC1PRemix1PIsDuoPartnerFirst = (duo.loads_first != 0) ? TRUE : FALSE;
    card->progress_icon = duo.progress_icon;
    card->name_texture = duo.name_texture;
    card->announcer_call = duo.announcer_call;

    stage = sc1PRemix1PPickStage(sc1PRemix1PRandom, duo.main_stages, is_stage_used, SC1PREMIX_1P_STAGE_ID_COUNT);

    if (stage < 0)
    {
        stage = duo.main_stages[0];
    }
    sc1PRemix1PSetSlot(SC1PREMIX_1P_DUO_SLOT, stage, duo.main_fkind);
    sSC1PRemix1PStages[SC1PREMIX_1P_DUO_SLOT].fkind[1] = duo.partner_fkind;
}

static sb32 sc1PRemix1PFillSlot(s32 slot, int *is_fighter_used, int *is_stage_used)
{
    SC1PRemix1PFighter fighter;
    int options[SC1PREMIX_1P_STAGE_OPTIONS];
    s32 stage;
    s32 pool_index;
    s32 i;

    while ((pool_index = sc1PRemix1PPickFighter(sc1PRemix1PRandom, is_fighter_used, SC1PREMIX_1P_POOL_COUNT)) >= 0)
    {
        sc1PRemix1PDataReadFighter(pool_index, &fighter);

        for (i = 0; i < SC1PREMIX_1P_STAGE_OPTIONS; i++)
        {
            options[i] = fighter.stages[i];
        }
        stage = sc1PRemix1PPickStage(sc1PRemix1PRandom, options, is_stage_used, SC1PREMIX_1P_STAGE_ID_COUNT);

        if (stage < 0)
        {
            continue;
        }
        sc1PRemix1PSetCard(slot, &fighter);
        sc1PRemix1PSetSlot(slot, stage, fighter.fkind);

        if (slot == SC1PREMIX_1P_TINY_TEAM_SLOT)
        {
            sSC1PRemix1PStages[slot].fkind[1] = fighter.fkind;
            sSC1PRemix1PStages[slot].opponent_behavior = fighter.fkind;
        }
        return TRUE;
    }
    return FALSE;
}

static void sc1PRemix1PSetBoss(s32 fkind, s32 gkind, s32 name_texture, s32 icon, s32 call, s32 scale)
{
    SC1PRemix1PSlotCard *card = &sSC1PRemix1PCards[SC1PREMIX_1P_BOSS_SLOT];

    sc1PRemix1PSetSlot(SC1PREMIX_1P_BOSS_SLOT, gkind, fkind);

    card->progress_icon = icon;
    card->name_texture = name_texture + SC1PREMIX_1P_NAME_TEXTURE_HEADER;
    card->announcer_call = call;
    card->model_scale = scale;
}

static void sc1PRemix1PPickBossSlot(void)
{
    s32 boss = sc1PRemix1PPickBoss(sc1PRemix1PRandom);
    s32 sonic_stage;

    switch (boss)
    {
    case SC1PREMIX_1P_BOSS_MAD_PIANO:
        sc1PRemix1PSetBoss(REMIX_ID_PIANO, nGRRemixStage_BIG_BOOS_HAUNT, SC1PREMIX_1P_NAME_TEXTURE_PIANO, SC1PREMIX_1P_ICON_PIANO,
                           SC1PREMIX_1P_CALL_PIANO, SC1PREMIX_1P_SCALE_BOSS);
        break;

    case SC1PREMIX_1P_BOSS_METAL_BROS:
        sc1PRemix1PSetBoss(REMIX_ID_METAL, nGRRemixStage_META_CRYSTAL_DL, SC1PREMIX_1P_NAME_TEXTURE_METAL_MARIO_BROS, SC1PREMIX_1P_ICON_METAL_MARIO_BROS,
                           SC1PREMIX_1P_CALL_METAL_MARIO_BROS, SC1PREMIX_1P_SCALE_BOSS);
        sSC1PRemix1PStages[SC1PREMIX_1P_BOSS_SLOT].fkind[1] = REMIX_ID_MLUIGI;
        sSC1PRemix1PStages[SC1PREMIX_1P_BOSS_SLOT].opponent_count = 2;
        sSC1PRemix1PIsMetalBros = TRUE;
        break;

    default:
        sonic_stage = (sc1PRemix1PPickSonicStage(sc1PRemix1PRandom) == SC1PREMIX_1P_SONIC_STAGE_CASINO) ? nGRRemixStage_CASINO : nGRRemixStage_GHZ;
        sc1PRemix1PSetBoss(REMIX_ID_SSONIC, sonic_stage, SC1PREMIX_1P_NAME_TEXTURE_SSONIC, SC1PREMIX_1P_ICON_SSONIC,
                           SC1PREMIX_1P_CALL_SSONIC, SC1PREMIX_1P_SCALE_SSONIC);
        break;
    }
}

static void sc1PRemix1PBuildPath(void)
{
    int is_fighter_used[SC1PREMIX_1P_POOL_COUNT] = { 0 };
    int is_stage_used[SC1PREMIX_1P_STAGE_ID_COUNT] = { 0 };
    s32 pick;

    sc1PRemix1PPickKirbys();
    sc1PRemix1PPickDuo(is_fighter_used, is_stage_used);

    for (pick = 0; pick < SC1PREMIX_1P_RANDOM_SLOTS; pick++)
    {
        if (sc1PRemix1PFillSlot(sc1PRemix1PDataRandomSlot(pick), is_fighter_used, is_stage_used) == FALSE)
        {
            port_log("remix1p: no fighter left for random slot %d\n", (int)pick);
        }
    }
    sc1PRemix1PPickBossSlot();
}

static void sc1PRemix1PClearSizes(void)
{
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        ftRemixSizeForceMatchState(port, FTREMIX_SIZE_STATE_UNFORCED);
    }
}

static void sc1PRemix1PLogPath(void)
{
    s32 i;

    for (i = 0; i < SC1PREMIX_1P_PATH_COUNT; i++)
    {
        port_log("remix1p: stage %d gkind=0x%02X fkind=0x%02X,0x%02X count=%d\n", (int)i, (int)sSC1PRemix1PStages[i].gkind,
                 (int)sSC1PRemix1PStages[i].fkind[0], (int)sSC1PRemix1PStages[i].fkind[1], (int)sSC1PRemix1PStages[i].opponent_count);
    }
}

sb32 sc1PRemix1PIsActive(void)
{
    return ((sc1PRemixModeIs(SC1PREMIX_MODE_REMIX_1P) != FALSE) && (sSC1PRemix1PIsReady != FALSE)) ? TRUE : FALSE;
}

void sc1PRemix1PRandomize(void)
{
    sSC1PRemix1PIsReady = FALSE;
    sc1PRemix1PClearSizes();

    if (sc1PRemixModeIs(SC1PREMIX_MODE_REMIX_1P) == FALSE)
    {
        return;
    }
    sc1PRemix1PLoadTables();
    sSC1PRemix1PStages[SC1PREMIX_1P_BOSS_SLOT].opponent_count = 1;
    sc1PRemixModeAdvanceSeed();
    sc1PRemix1PBuildPath();
    sSC1PRemix1PIsReady = TRUE;
    sc1PRemix1PLogPath();
}

SC1PGameStage* sc1PRemix1PGetStage(s32 stage_index)
{
    if ((sc1PRemix1PIsActive() == FALSE) || (stage_index < 0) || (stage_index >= SC1PREMIX_1P_STAGE_COUNT))
    {
        return NULL;
    }
    return &sSC1PRemix1PStages[stage_index];
}

void sc1PRemix1PSetupEnemy(s32 player)
{
    if (sc1PRemix1PIsActive() == FALSE)
    {
        return;
    }
    if (gSCManagerSceneData.spgame_stage == nSC1PGameStageDonkey)
    {
        ftRemixSizeForceMatchState(player, FTREMIX_SIZE_STATE_GIANT);
        return;
    }
    if (gSCManagerSceneData.spgame_stage == nSC1PGameStageSamus)
    {
        ftRemixSizeForceMatchState(player, FTREMIX_SIZE_STATE_TINY);
        return;
    }
    sc1PRemix1PClearSizes();
}

SC1PGameComputer* sc1PRemix1PGetComputer(s32 stage_index)
{
    if ((sc1PRemix1PIsActive() == FALSE) || (stage_index < 0) || (stage_index >= SC1PREMIX_1P_STAGE_COUNT))
    {
        return NULL;
    }
    return &sSC1PRemix1PComputers[stage_index];
}

const SC1PRemix1PSlotCard* sc1PRemix1PGetCard(s32 stage_index)
{
    if ((sc1PRemix1PIsActive() == FALSE) || (stage_index < 0) || (stage_index >= SC1PREMIX_1P_PATH_COUNT))
    {
        return NULL;
    }
    return &sSC1PRemix1PCards[stage_index];
}

void sc1PRemix1PGetKirby(s32 slot, s32 *power, s32 *hat)
{
    slot = ((slot >= 0) && (slot <= SC1PREMIX_1P_KIRBY_RANDOM_COUNT)) ? slot : 0;

    *power = sSC1PRemix1PKirbyPowers[slot];
    *hat = sSC1PRemix1PKirbyHats[slot];
}

sb32 sc1PRemix1PIsDuoPartnerFirst(void)
{
    return sSC1PRemix1PIsDuoPartnerFirst;
}

sb32 sc1PRemix1PIsMetalBros(void)
{
    return sSC1PRemix1PIsMetalBros;
}

#endif
