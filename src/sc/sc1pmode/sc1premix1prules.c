/**
 * sc1premix1prules.c - random picks of the Remix 1P path, see sc1premix1prules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premix1prules.h"

#define SC1PREMIX_1P_BOSS_COUNT 3
#define SC1PREMIX_1P_SONIC_STAGE_COUNT 3
#define SC1PREMIX_1P_NO_PICK (-1)

int sc1PRemix1PPickFighter(SC1PRemix1PRandom rand, int *is_used, int count)
{
    int free_count = 0;
    int target;
    int i;

    for (i = 0; i < count; i++)
    {
        free_count += (is_used[i] == 0) ? 1 : 0;
    }
    if (free_count == 0)
    {
        return SC1PREMIX_1P_NO_PICK;
    }
    target = rand(free_count);

    for (i = 0; i < count; i++)
    {
        if (is_used[i] != 0)
        {
            continue;
        }
        if (target-- == 0)
        {
            is_used[i] = 1;
            return i;
        }
    }
    return SC1PREMIX_1P_NO_PICK;
}

static int sc1PRemix1PStageIsFree(int stage, const int *is_stage_used, int stage_count)
{
    return ((stage >= 0) && (stage < stage_count) && (is_stage_used[stage] == 0)) ? 1 : 0;
}

int sc1PRemix1PPickStage(SC1PRemix1PRandom rand, const int options[SC1PREMIX_1P_STAGE_OPTIONS], int *is_stage_used, int stage_count)
{
    int free_options[SC1PREMIX_1P_STAGE_OPTIONS];
    int free_count = 0;
    int stage;
    int i;

    for (i = 0; i < SC1PREMIX_1P_STAGE_OPTIONS; i++)
    {
        if (sc1PRemix1PStageIsFree(options[i], is_stage_used, stage_count) != 0)
        {
            free_options[free_count++] = options[i];
        }
    }
    if (free_count == 0)
    {
        return SC1PREMIX_1P_NO_PICK;
    }
    stage = free_options[rand(free_count)];
    is_stage_used[stage] = 1;

    return stage;
}

void sc1PRemix1PPickDistinct(SC1PRemix1PRandom rand, int count, int *picks, int picks_count)
{
    int is_used[256] = { 0 };
    int i;

    if (count > 256)
    {
        count = 256;
    }
    for (i = 0; i < picks_count; i++)
    {
        picks[i] = sc1PRemix1PPickFighter(rand, is_used, count);
    }
}

int sc1PRemix1PPickBoss(SC1PRemix1PRandom rand)
{
    return rand(SC1PREMIX_1P_BOSS_COUNT);
}

int sc1PRemix1PPickSonicStage(SC1PRemix1PRandom rand)
{
    return (rand(SC1PREMIX_1P_SONIC_STAGE_COUNT) == SC1PREMIX_1P_SONIC_STAGE_CASINO) ? SC1PREMIX_1P_SONIC_STAGE_CASINO : SC1PREMIX_1P_SONIC_STAGE_GHZ;
}

#endif
