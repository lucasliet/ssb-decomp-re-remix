#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixpracticerules.h"

int gmRemixPracticeIsRunActive(int mode, int panel_value)
{
    if ((mode != GMREMIX_PRACTICE_MODE_STANDARD) && (mode != GMREMIX_PRACTICE_MODE_REMIX_1P))
    {
        return 0;
    }
    return panel_value != 0;
}

int gmRemixPracticeIsRetryHeld(unsigned int button_hold)
{
    return (button_hold & GMREMIX_PRACTICE_BUTTON_L) != 0;
}

int gmRemixPracticeIsPauseRetry(int is_practice_active, int is_1p_game_scene, unsigned int button_tap)
{
    return is_practice_active && is_1p_game_scene && ((button_tap & GMREMIX_PRACTICE_BUTTON_L) != 0);
}

#endif
