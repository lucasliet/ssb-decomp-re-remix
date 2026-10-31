#include <mn/mncommon/mnremixtimeouts.h>

/* Shared condition of the idle-return branches. timeouts.asm replaces the vanilla
 * bne of every screen with `b`, so the Remix build never takes the branch. */
static sb32 mnRemixTimeoutsIsReturnTic(s32 total_tics, s32 return_tic)
{
#ifdef REGION_REMIX
    return FALSE;
#else
    return total_tics == return_tic;
#endif
}

/**
 * Gates the idle return of the single-column menus (mode select, 1P mode, options, data, stage select).
 *
 * @param total_tics Tics the screen has been running.
 * @param return_tic Tic at which the idle return fires.
 * @return TRUE when the vanilla return-to-title branch must run.
 */
sb32 mnRemixTimeoutsMenuReturns(s32 total_tics, s32 return_tic)
{
    return mnRemixTimeoutsIsReturnTic(total_tics, return_tic);
}

/**
 * Gates the idle return of the VS menu and the VS options screen.
 *
 * @param total_tics Tics the screen has been running.
 * @param return_tic Tic at which the idle return fires.
 * @return TRUE when the vanilla return-to-title branch must run.
 */
sb32 mnRemixTimeoutsVSReturns(s32 total_tics, s32 return_tic)
{
    return mnRemixTimeoutsIsReturnTic(total_tics, return_tic);
}

/**
 * Gates the idle return of the character select screens (VS, 1P, training, bonus).
 *
 * @param total_tics Tics the screen has been running.
 * @param return_tic Tic at which the idle return fires.
 * @return TRUE when the vanilla return-to-title branch must run.
 */
sb32 mnRemixTimeoutsCSSReturns(s32 total_tics, s32 return_tic)
{
    return mnRemixTimeoutsIsReturnTic(total_tics, return_tic);
}
