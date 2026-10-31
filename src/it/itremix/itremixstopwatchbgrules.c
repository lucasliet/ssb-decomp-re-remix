#include "it/itremix/itremixstopwatchbgrules.h"

#define FAST_SPEED_LIMIT 7
#define FADE_TIMER_LIMIT 240
#define FADE_PERIOD_MASK 3
#define ALPHA_FLOOR 0x33
#define ALPHA_SLOW_FADE_LIMIT 0x66

uint32_t itRemixStopwatchBgGetColor(int32_t speed)
{
    return (speed < FAST_SPEED_LIMIT) ? ITREMIX_STOPWATCH_BG_COLOR_FAST : ITREMIX_STOPWATCH_BG_COLOR_SLOW;
}

uint32_t itRemixStopwatchBgFadeColor(uint32_t color, int32_t timer)
{
    uint32_t alpha = color & 0xFF;

    if ((timer >= FADE_TIMER_LIMIT) || ((timer & FADE_PERIOD_MASK) != 0))
    {
        return color;
    }
    if (alpha < ALPHA_FLOOR)
    {
        return color;
    }
    alpha -= (alpha < ALPHA_SLOW_FADE_LIMIT) ? 1 : 3;

    return (color & 0xFFFFFF00U) | alpha;
}
