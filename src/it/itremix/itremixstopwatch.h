#ifndef _ITREMIXSTOPWATCH_H_
#define _ITREMIXSTOPWATCH_H_

#include "it/itremix/itremix.h"

/* Return values of itRemixStopwatchCheckFunction, consumed by the Speed.asm hook. */
#define ITREMIX_STOPWATCH_CHECK_RUN_NORMAL 0
#define ITREMIX_STOPWATCH_CHECK_SKIP 1
#define ITREMIX_STOPWATCH_CHECK_RUN_TWICE 2

/* Spawns Stopwatch (Remix item 0x39). Fighter call sites use this signature. */
GObj* itRemixStopwatchMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Speed.asm hook (Item.Stopwatch.check_function_), run before each fighter or projectile update. */
s32 itRemixStopwatchCheckFunction(GObj *gobj, void (*update_proc)(GObj *gobj));

/* Read by the vanilla hitlag patches to freeze a player for the current frame. */
sb32 itRemixStopwatchIsFunctionAbbr(void);

/* Read by Speed.asm (Item.Stopwatch.watch_speed_flag): 0 when no backfire speed is active. */
u8 itRemixStopwatchGetWatchSpeed(void);

/* Item.asm clear_active_stopwatch_: resets all Stopwatch state at scene end. */
void itRemixStopwatchClearActive(void);

extern const ITRemixItemDesc dITRemixStopwatchItemDesc;

#endif
