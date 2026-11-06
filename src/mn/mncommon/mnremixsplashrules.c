/**
 * mnremixsplashrules.c - state-free rules of the Remix splash image, see mnremixsplashrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/mncommon/mnremixsplashrules.h>

int mnRemixSplashRulesGetJingleFgm(int held_l_count)
{
    if (held_l_count >= MN_REMIX_SPLASH_PLAYER_COUNT)
    {
        return MN_REMIX_SPLASH_FGM_ALL_L;
    }
    if (held_l_count > 0)
    {
        return MN_REMIX_SPLASH_FGM_ONE_L;
    }
    return MN_REMIX_SPLASH_FGM_NICE;
}

int mnRemixSplashRulesTickCountdown(int *frames_left)
{
    if (*frames_left <= 0)
    {
        return 1;
    }
    (*frames_left)--;
    return 0;
}

#endif
