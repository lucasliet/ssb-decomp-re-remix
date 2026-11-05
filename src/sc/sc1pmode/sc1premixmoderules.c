/**
 * sc1premixmoderules.c - state-free rules of the Remix 1P modes, see sc1premixmoderules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixmoderules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

#define SC1PREMIX_START_CHEER_SKIP_BONUS 2
#define SC1PREMIX_SEED_FRAME_MASK 0x3F
#define SC1PREMIX_MULTIMAN_ENEMY_HANDICAP 0x15
#define SC1PREMIX_CRUEL_ENEMY_HANDICAP 0x18
#define SC1PREMIX_CRUEL_HUMAN_HANDICAP 0x16

int sc1PRemixModeLeavesBonusCSSFor1P(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_BONUS3:
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
    case SC1PREMIX_MODE_HRC:
        return 1;

    default:
        return 0;
    }
}

int sc1PRemixModeSkipsBonusStartCheer(int toggle_value, int mode, int is_bonus_practice)
{
    if (toggle_value < SC1PREMIX_START_CHEER_SKIP_BONUS)
    {
        return 0;
    }
    if ((mode == SC1PREMIX_MODE_BONUS3) || (mode == SC1PREMIX_MODE_HRC))
    {
        return 1;
    }
    return (is_bonus_practice != 0) ? 1 : 0;
}

unsigned int sc1PRemixModeSeedAdvanceCount(unsigned int frame_count)
{
    return (frame_count & SC1PREMIX_SEED_FRAME_MASK) + 1;
}

int sc1PRemixModeIsMultiman(int mode)
{
    return ((mode == SC1PREMIX_MODE_MULTIMAN) || (mode == SC1PREMIX_MODE_CRUEL)) ? 1 : 0;
}

int sc1PRemixModeEnemyHandicap(int mode)
{
    if (mode == SC1PREMIX_MODE_MULTIMAN)
    {
        return SC1PREMIX_MULTIMAN_ENEMY_HANDICAP;
    }
    return (mode == SC1PREMIX_MODE_CRUEL) ? SC1PREMIX_CRUEL_ENEMY_HANDICAP : 0;
}

int sc1PRemixModeHumanHandicap(int mode)
{
    return (mode == SC1PREMIX_MODE_CRUEL) ? SC1PREMIX_CRUEL_HUMAN_HANDICAP : 0;
}

int sc1PRemixMultimanRestartsRun(int is_reset)
{
    return (is_reset != 0) ? 1 : 0;
}

void sc1PRemixMultimanKosReset(SC1PRemixMultimanKos *counter)
{
    counter->kos = 0;
    counter->is_player_ko = 0;
}

void sc1PRemixMultimanKosAddEnemy(SC1PRemixMultimanKos *counter)
{
    if (counter->is_player_ko != 0)
    {
        return;
    }
    counter->kos++;
}

void sc1PRemixMultimanKosSetPlayerKo(SC1PRemixMultimanKos *counter)
{
    counter->is_player_ko = 1;
}

void sc1PRemixMultimanHudY(int dragon_king_hud, float *polygon_y, float *cross_y, float *counter_y)
{
    int is_low = (dragon_king_hud == 1);

    *polygon_y = is_low ? 214.0F : 20.0F;
    *cross_y = is_low ? 212.0F : 18.0F;
    *counter_y = is_low ? 212.0F : 18.0F;
}

#endif
