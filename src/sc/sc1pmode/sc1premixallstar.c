/**
 * sc1premixallstar.c - All-Star order and progress, see sc1premixallstar.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <port_log.h>
#include <ft/fighter.h>
#include <if/ifcommon.h>
#include <it/itmanager.h>
#include <mp/mpdef.h>
#include <sys/utils.h>
#include <sc/scene.h>
#include "sc/sc1pmode/sc1premixallstar.h"
#include "sc/sc1pmode/sc1premix1pdata.h"
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixmode.h"

extern char *getenv(const char *name);
extern int atoi(const char *str);

#define SC1PREMIX_ALLSTAR_STAGE_ID_COUNT 256
#define SC1PREMIX_ALLSTAR_EXIT_CLIP 0xE
#define SC1PREMIX_ALLSTAR_EXIT_WAIT 1
#define SC1PREMIX_ALLSTAR_HEARTS 3
#define SC1PREMIX_ALLSTAR_HEART_Y 300.0F

static s32 sSC1PRemixAllstarFkinds[SC1PREMIX_ALLSTAR_FIGHTER_COUNT];
static s32 sSC1PRemixAllstarGkinds[SC1PREMIX_ALLSTAR_FIGHTER_COUNT];
static s32 sSC1PRemixAllstarProgress;
static sb32 sSC1PRemixAllstarIsReady;
static sb32 sSC1PRemixAllstarIsLeaving;
static s32 sSC1PRemixAllstarPercent;
static s32 sSC1PRemixAllstarHearts;
static const f32 dSC1PRemixAllstarHeartX[SC1PREMIX_ALLSTAR_HEARTS] = { -1200.0F, -800.0F, -400.0F };
static SC1PGameStage sSC1PRemixAllstarStages[SC1PREMIX_1P_STAGE_COUNT];
static SC1PGameComputer sSC1PRemixAllstarComputers[SC1PREMIX_1P_STAGE_COUNT];

static int sc1PRemixAllstarRandom(int range)
{
    return (int)syUtilsRandIntRange(range);
}

sb32 sc1PRemixAllstarIsActive(void)
{
    return ((sc1PRemixModeIs(SC1PREMIX_MODE_ALLSTAR) != FALSE) && (sSC1PRemixAllstarIsReady != FALSE)) ? TRUE : FALSE;
}

void sc1PRemixAllstarRandomize(void)
{
    int stage_options[SC1PREMIX_ALLSTAR_FIGHTER_COUNT][SC1PREMIX_1P_STAGE_OPTIONS];
    int is_stage_used[SC1PREMIX_ALLSTAR_STAGE_ID_COUNT] = { 0 };
    int order[SC1PREMIX_ALLSTAR_FIGHTER_COUNT];
    int stages[SC1PREMIX_ALLSTAR_FIGHTER_COUNT];
    SC1PRemix1PFighter fighter;
    s32 slot;
    s32 i;

    sSC1PRemixAllstarIsReady = FALSE;
    sSC1PRemixAllstarIsLeaving = FALSE;
    sSC1PRemixAllstarProgress = 0;
    sSC1PRemixAllstarPercent = 0;
    sSC1PRemixAllstarHearts = 0;

    if (sc1PRemixModeIs(SC1PREMIX_MODE_ALLSTAR) == FALSE)
    {
        return;
    }
    sc1PRemixModeAdvanceSeed();

    for (i = 0; i < SC1PREMIX_ALLSTAR_FIGHTER_COUNT; i++)
    {
        sc1PRemix1PDataReadFighter(i, &fighter);

        stage_options[i][0] = fighter.stages[0];
        stage_options[i][1] = fighter.stages[1];
        stage_options[i][2] = fighter.stages[2];
    }
    for (i = 0; i < SC1PREMIX_1P_STAGE_COUNT; i++)
    {
        sc1PRemix1PDataReadAllstarStage(i, &sSC1PRemixAllstarStages[i]);
        sc1PRemix1PDataReadAllstarComputer(i, &sSC1PRemixAllstarComputers[i]);
    }
    /* rest_area_and_hrc_spawning: the Rest Area line holds a CPU that never spawns. */
    sSC1PRemixAllstarStages[SC1PREMIX_ALLSTAR_STAGE_REST].opponent_count = 0;

    sc1PRemixAllstarPickOrder(sc1PRemixAllstarRandom, stage_options, SC1PREMIX_ALLSTAR_STAGE_ID_COUNT, is_stage_used, order, stages);

    for (slot = 0; slot < SC1PREMIX_ALLSTAR_FIGHTER_COUNT; slot++)
    {
        sc1PRemix1PDataReadFighter(order[slot], &fighter);

        sSC1PRemixAllstarFkinds[slot] = fighter.fkind;
        sSC1PRemixAllstarGkinds[slot] = stages[slot];

        port_log("allstar: slot %d fkind=0x%02X gkind=0x%02X\n", (int)slot, (int)sSC1PRemixAllstarFkinds[slot], (int)sSC1PRemixAllstarGkinds[slot]);
    }
#ifdef PORT
    {
        const char *progress_env = getenv("SSB64_ALLSTAR_PROGRESS");

        if (progress_env != NULL)
        {
            sSC1PRemixAllstarProgress = atoi(progress_env);
        }
    }
#endif
    sSC1PRemixAllstarIsReady = TRUE;
}

s32 sc1PRemixAllstarGetProgress(void)
{
    return sSC1PRemixAllstarProgress;
}

s32 sc1PRemixAllstarGetFkind(s32 slot)
{
    return ((slot >= 0) && (slot < SC1PREMIX_ALLSTAR_FIGHTER_COUNT)) ? sSC1PRemixAllstarFkinds[slot] : 0;
}

s32 sc1PRemixAllstarGetGkind(s32 slot)
{
    return ((slot >= 0) && (slot < SC1PREMIX_ALLSTAR_FIGHTER_COUNT)) ? sSC1PRemixAllstarGkinds[slot] : 0;
}

s32 sc1PRemixAllstarOpponentsAt(s32 progress)
{
    if (progress < SC1PREMIX_ALLSTAR_DOUBLE_PROGRESS)
    {
        return 1;
    }
    return (progress < SC1PREMIX_ALLSTAR_TRIPLE_PROGRESS) ? 2 : 3;
}

SC1PGameStage* sc1PRemixAllstarGetStage(s32 stage_index)
{
    if ((sc1PRemixAllstarIsActive() == FALSE) || (stage_index < 0) || (stage_index >= SC1PREMIX_1P_STAGE_COUNT))
    {
        return NULL;
    }
    return &sSC1PRemixAllstarStages[stage_index];
}

SC1PGameComputer* sc1PRemixAllstarGetComputer(s32 stage_index)
{
    if ((sc1PRemixAllstarIsActive() == FALSE) || (stage_index < 0) || (stage_index >= SC1PREMIX_1P_STAGE_COUNT))
    {
        return NULL;
    }
    return &sSC1PRemixAllstarComputers[stage_index];
}

static s32 sc1PRemixAllstarMatchTypeAt(s32 progress)
{
    if (progress < SC1PREMIX_ALLSTAR_DOUBLE_PROGRESS)
    {
        return SC1PREMIX_ALLSTAR_STAGE_SINGLE;
    }
    if (progress < SC1PREMIX_ALLSTAR_TRIPLE_PROGRESS)
    {
        return SC1PREMIX_ALLSTAR_STAGE_DOUBLE;
    }
    return (progress < SC1PREMIX_ALLSTAR_FINAL_PROGRESS) ? SC1PREMIX_ALLSTAR_STAGE_TRIPLE : SC1PREMIX_ALLSTAR_STAGE_FINAL;
}

/* rest_area_routine: the line of the match type gets the fighters and the stage of the order, one fighter per opponent. */
static void sc1PRemixAllstarPatchMatch(s32 type)
{
    SC1PGameStage *line = &sSC1PRemixAllstarStages[type];
    s32 opponents = (type == SC1PREMIX_ALLSTAR_STAGE_FINAL) ? 1 : sc1PRemixAllstarOpponentsAt(sSC1PRemixAllstarProgress);

    line->fkind[0] = sc1PRemixAllstarGetFkind(sSC1PRemixAllstarProgress);
    line->gkind = sc1PRemixAllstarGetGkind(sSC1PRemixAllstarProgress);

    if (opponents > 1)
    {
        line->fkind[1] = sc1PRemixAllstarGetFkind(sSC1PRemixAllstarProgress + 1);
    }
    if (opponents > 2)
    {
        line->opponent_behavior = sc1PRemixAllstarGetFkind(sSC1PRemixAllstarProgress + 2);
    }
    port_log("allstar: match type %d progress %d fkind=0x%02X,0x%02X,0x%02X gkind=0x%02X\n", (int)type, (int)sSC1PRemixAllstarProgress, (int)line->fkind[0],
             (int)line->fkind[1], (int)line->opponent_behavior, (int)line->gkind);
}

void sc1PRemixAllstarUpdate(void)
{
    GObj *player_gobj;
    FTStruct *player_fp;

    if ((sc1PRemixAllstarIsActive() == FALSE) || (gSCManagerSceneData.spgame_stage != SC1PREMIX_ALLSTAR_STAGE_REST) || (sSC1PRemixAllstarIsLeaving != FALSE))
    {
        return;
    }
    player_gobj = gSCManagerBattleState->players[gSCManagerSceneData.player].fighter_gobj;

    if (player_gobj == NULL)
    {
        return;
    }
    player_fp = ftGetStruct(player_gobj);

    if ((player_fp->ga != nMPKineticsGround) || ((player_fp->coll_data.floor_flags & 0xFF) != SC1PREMIX_ALLSTAR_EXIT_CLIP))
    {
        return;
    }
    sSC1PRemixAllstarIsLeaving = TRUE;

    port_log("allstar: rest exit at tic %d x=%d\n", (int)gSCManagerBattleState->time_passed, (int)player_fp->joints[nFTPartsJointTopN]->translate.vec.f.x);

    ifCommonBattleSetInterface(ifCommonBattleInterfaceProcUpdate, ifCommon1PGameInterfaceProcSet, nSYAudioFGMVoiceEnd, SC1PREMIX_ALLSTAR_EXIT_WAIT);
}

s32 sc1PRemixAllstarPlayerDamage(s32 player)
{
    if ((sc1PRemixAllstarIsActive() == FALSE) || (gSCManagerBattleState->players[player].pkind != nFTPlayerKindMan))
    {
        return 0;
    }
    return sSC1PRemixAllstarPercent;
}

void sc1PRemixAllstarRecordPercent(s32 percent)
{
    if (sc1PRemixAllstarIsActive() != FALSE)
    {
        sSC1PRemixAllstarPercent = percent;
        port_log("allstar: percent %d recorded\n", (int)percent);
    }
}

void sc1PRemixAllstarClearPercent(void)
{
    sSC1PRemixAllstarPercent = 0;
}

void sc1PRemixAllstarSetupRestArea(void)
{
    Vec3f pos;
    Vec3f vel;
    s32 i;

    if ((sc1PRemixAllstarIsActive() == FALSE) || (gSCManagerSceneData.spgame_stage != SC1PREMIX_ALLSTAR_STAGE_REST))
    {
        return;
    }
    vel.x = vel.y = vel.z = 0.0F;

    pos.y = SC1PREMIX_ALLSTAR_HEART_Y;
    pos.z = 0.0F;

    for (i = sSC1PRemixAllstarHearts; i < SC1PREMIX_ALLSTAR_HEARTS; i++)
    {
        pos.x = dSC1PRemixAllstarHeartX[i];

        itManagerMakeItemSetupCommon(NULL, nITKindHeart, &pos, &vel, ITEM_FLAG_PARENT_DEFAULT);
    }
}

void sc1PRemixAllstarOnHeartUsed(void)
{
    if ((sc1PRemixAllstarIsActive() != FALSE) && (gSCManagerSceneData.spgame_stage == SC1PREMIX_ALLSTAR_STAGE_REST))
    {
        sSC1PRemixAllstarHearts++;
    }
}

s32 sc1PRemixAllstarStepStage(s32 stage, sb32 is_retry)
{
    s32 type;

    if (is_retry != FALSE)
    {
        return stage + 1;
    }
    sSC1PRemixAllstarIsLeaving = FALSE;

    if (stage == SC1PREMIX_ALLSTAR_STAGE_REST)
    {
        type = sc1PRemixAllstarMatchTypeAt(sSC1PRemixAllstarProgress);

        sc1PRemixAllstarPatchMatch(type);

        return type;
    }
    sc1PRemixAllstarAdvance();

    return (sSC1PRemixAllstarProgress >= SC1PREMIX_ALLSTAR_FIGHTER_COUNT) ? SC1PREMIX_ALLSTAR_STAGE_END : SC1PREMIX_ALLSTAR_STAGE_REST;
}

void sc1PRemixAllstarAdvance(void)
{
    sSC1PRemixAllstarProgress += sc1PRemixAllstarOpponentsAt(sSC1PRemixAllstarProgress);
}

#endif
