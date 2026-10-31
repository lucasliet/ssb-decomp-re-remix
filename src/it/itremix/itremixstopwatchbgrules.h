#ifndef _ITREMIXSTOPWATCHBGRULES_H_
#define _ITREMIXSTOPWATCHBGRULES_H_

#include <stdint.h>

/* Pure rules of the Stopwatch backfire background rectangle (Stopwatch.asm backfire_bg_colors, _update_bg_alpha). */

#define ITREMIX_STOPWATCH_BG_COLOR_SLOW 0x0066AA77U  /* slow down, blue */
#define ITREMIX_STOPWATCH_BG_COLOR_FAST 0xA1333399U  /* speed up, red */
#define ITREMIX_STOPWATCH_BG_COLOR_CLEAR 0x00000000U /* the end of the timer makes the rectangle transparent */

/** Colour of the rectangle for a game speed value: speeds below 7 are the fast ones. */
uint32_t itRemixStopwatchBgGetColor(int32_t speed);

/**
 * Colour after the fade of one frame. Under 240 frames left, every fourth frame the alpha drops by 3 while it is 0x66 or more,
 * by 1 while it is 0x33 or more, and stays at its floor below 0x33.
 *
 * @param color 0xRRGGBBAA of this frame
 * @param timer frames left before this frame's decrement
 */
uint32_t itRemixStopwatchBgFadeColor(uint32_t color, int32_t timer);

#endif
