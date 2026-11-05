#ifndef _FTREMIXSANDBAG_H_
#define _FTREMIXSANDBAG_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Sandbag landing patches (Sandbag/Sandbag.asm): the down bounce lands on the head or the feet, the damage multiplier
 * resets, the get-up is not delayed and the knockback X velocity does not decay. Each routine is a no-op for other fighters.
 */

#include <ft/fighter.h>

/**
 * sandbag_land (0x801443CC): status for the down bounce of ftCommonDownBounceSetStatus. Sandbag lands on its head (status
 * 0xE0) or upright (Idle) when its body is close enough to either.
 *
 * @param fp Fighter struct.
 * @param status_id Status the vanilla code picked (DownBounceD or DownBounceU).
 * @return The status to set.
 */
extern s32 ftRemixSandbagGetBounceStatus(FTStruct *fp, s32 status_id);

/**
 * _damage_multiplier (0x801444FC): after the down bounce sets damage_mul to 0.5, Sandbag in Idle or HeadStand takes it
 * back to 1.0.
 *
 * @param fp Fighter struct.
 */
extern void ftRemixSandbagResetDownBounceDamageMul(FTStruct *fp);

/**
 * sandbag_getup_ (0x8014422C): called by ftCommonDownWaitProcUpdate before it decrements stand_wait. Sandbag gets up on the
 * next decrement once its knockback X velocity is exactly zero.
 *
 * @param fp Fighter struct.
 */
extern void ftRemixSandbagPrepareGetUp(FTStruct *fp);

/**
 * preserve_sandbag_momentum_ (0x800E213C): whether ftMainProcPhysicsMap leaves the knockback X velocity alone. It does for
 * Sandbag.
 *
 * @param fp Fighter struct.
 * @return TRUE for Sandbag.
 */
extern sb32 ftRemixSandbagKeepsKnockbackX(FTStruct *fp);

#endif

#endif
