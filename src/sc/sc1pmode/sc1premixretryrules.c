#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixpracticerules.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixretryrules.h"

int sc1PRemixRetryIsFakeScreenMode(int mode)
{
    return ((mode == SC1PREMIX_MODE_BONUS3) || (mode == SC1PREMIX_MODE_MULTIMAN) || (mode == SC1PREMIX_MODE_CRUEL) ||
            (mode == SC1PREMIX_MODE_HRC)) ? 1 : 0;
}

int sc1PRemixRetryIsPauseRetry(int mode, int is_1p_game_scene, unsigned int button_tap)
{
    if ((is_1p_game_scene == 0) || (sc1PRemixRetryIsFakeScreenMode(mode) == 0))
    {
        return 0;
    }
    return ((button_tap & GMREMIX_PRACTICE_BUTTON_L) != 0) ? 1 : 0;
}

int sc1PRemixRetryIsGameSetRetry(int mode, int is_1p_game_scene, int is_bonus_practice_scene, unsigned int button_hold)
{
    int is_retry_scene = (is_bonus_practice_scene != 0) || ((is_1p_game_scene != 0) && (sc1PRemixRetryIsFakeScreenMode(mode) != 0));

    if (is_retry_scene == 0)
    {
        return 0;
    }
    return gmRemixPracticeIsRetryHeld(button_hold);
}

#endif
