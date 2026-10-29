/**
 * sc1premixallstar.c - All-Star order and progress, see sc1premixallstar.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <port_log.h>
#include <sys/utils.h>
#include <sc/scene.h>
#include "sc/sc1pmode/sc1premixallstar.h"
#include "sc/sc1pmode/sc1premix1pdata.h"
#include "sc/sc1pmode/sc1premixbonusexit.h"
#include "sc/sc1pmode/sc1premixmode.h"

#define SC1PREMIX_ALLSTAR_STAGE_ID_COUNT 256

static s32 sSC1PRemixAllstarFkinds[SC1PREMIX_ALLSTAR_FIGHTER_COUNT];
static s32 sSC1PRemixAllstarGkinds[SC1PREMIX_ALLSTAR_FIGHTER_COUNT];
static s32 sSC1PRemixAllstarProgress;
static sb32 sSC1PRemixAllstarIsReady;

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
    sSC1PRemixAllstarProgress = 0;

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
    sc1PRemixAllstarPickOrder(sc1PRemixAllstarRandom, stage_options, SC1PREMIX_ALLSTAR_STAGE_ID_COUNT, is_stage_used, order, stages);

    for (slot = 0; slot < SC1PREMIX_ALLSTAR_FIGHTER_COUNT; slot++)
    {
        sc1PRemix1PDataReadFighter(order[slot], &fighter);

        sSC1PRemixAllstarFkinds[slot] = fighter.fkind;
        sSC1PRemixAllstarGkinds[slot] = stages[slot];

        port_log("allstar: slot %d fkind=0x%02X gkind=0x%02X\n", (int)slot, (int)sSC1PRemixAllstarFkinds[slot], (int)sSC1PRemixAllstarGkinds[slot]);
    }
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

void sc1PRemixAllstarAdvance(void)
{
    sSC1PRemixAllstarProgress += sc1PRemixAllstarOpponentsAt(sSC1PRemixAllstarProgress);
}

#endif
