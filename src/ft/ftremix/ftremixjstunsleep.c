/**
 * ftremixjstunsleep.c - Smash Remix Japanese.asm japanese_stun_mash and japanese_sleep_mash: the Japanese Stun/Sleep
 * toggle lowers the breakout wait of the Fura Fura (stun) and Fura Sleep (sleep) statuses.
 *
 * The vanilla breakout wait already includes the U minimum (FTCOMMON_FURAFURA_BREAKOUT_WAIT_MIN and
 * FTCOMMON_FURASLEEP_BREAKOUT_WAIT_MIN); the J formula subtracts 30 (stun) and 35 (sleep) from it.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "gm/gmremixtoggle.h"

/* Japanese.asm: "subtracts 30 from U formula for convenience" (stun) and "subtracts 35" (sleep) */
#define FTREMIX_JSTUN_BREAKOUT_SUB 30
#define FTREMIX_JSLEEP_BREAKOUT_SUB 35

/**
 * Returns the Fura Fura breakout wait for the Japanese Stun/Sleep toggle. breakout_wait is the vanilla value,
 * already including FTCOMMON_FURAFURA_BREAKOUT_WAIT_MIN. Bonus and 1P Game ignore the toggle (gmRemixToggleVS).
 */
s32 ftRemixFuraFuraBreakoutWait(s32 breakout_wait)
{
    if (gmRemixToggleVS(j_stun_sleep))
    {
        return breakout_wait - FTREMIX_JSTUN_BREAKOUT_SUB;
    }
    return breakout_wait;
}

/**
 * Returns the Fura Sleep breakout wait for the Japanese Stun/Sleep toggle. breakout_wait is the vanilla value,
 * already including FTCOMMON_FURASLEEP_BREAKOUT_WAIT_MIN. Bonus and 1P Game ignore the toggle (gmRemixToggleVS).
 */
s32 ftRemixFuraSleepBreakoutWait(s32 breakout_wait)
{
    if (gmRemixToggleVS(j_stun_sleep))
    {
        return breakout_wait - FTREMIX_JSLEEP_BREAKOUT_SUB;
    }
    return breakout_wait;
}

#endif
