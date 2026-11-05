#ifndef _FTREMIXSANDBAGRULES_H_
#define _FTREMIXSANDBAGRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Sandbag landing patches (Sandbag/Sandbag.asm sandbag_land, sandbag_getup_). Floats and ints only, so the
 * headless unit tests (tests/remix) can run them.
 */

/** Action.HeadStand: Sandbag's unique headstand status (Character.edit_action(SANDBAG, 0xE0)). */
#define FTREMIXSANDBAG_STATUS_HEADSTAND 0xE0

/** Action.Idle. */
#define FTREMIXSANDBAG_STATUS_IDLE 0x09

/** Returned by ftRemixSandbagGetLandingStatus when the vanilla down bounce runs. */
#define FTREMIXSANDBAG_STATUS_NONE (-1)

/**
 * sandbag_land (0x801443CC): status Sandbag takes when it lands from knockback, from the X rotation of its body. The angle
 * goes from 0 to -2pi and -pi is upside down. Within 0.2pi of upside down it lands on its head, within 0.2pi of upright it
 * stands; anything in between bounces as usual.
 *
 * @param rotation_x X rotation of the body joint in radians.
 * @return FTREMIXSANDBAG_STATUS_HEADSTAND, FTREMIXSANDBAG_STATUS_IDLE or FTREMIXSANDBAG_STATUS_NONE.
 */
extern int ftRemixSandbagGetLandingStatus(float rotation_x);

/**
 * sandbag_getup_ (0x8014422C): the down wait counter Sandbag decrements. It gets up as soon as its knockback X velocity is
 * exactly zero, so the counter reads 1 then.
 *
 * @param stand_wait Down wait counter.
 * @param is_knockback_x_zero 1 when the bits of vel_damage_air.x are zero.
 * @return The counter value to decrement.
 */
extern int ftRemixSandbagGetStandWait(int stand_wait, int is_knockback_x_zero);

/**
 * _damage_multiplier (0x801444FC): damage multiplier Sandbag gets after a down bounce status change. It resets to 1.0 in Idle
 * and HeadStand; every other status keeps the 0.5 of the down bounce.
 *
 * @param status_id Status after the change.
 * @return 1 when the multiplier must be 1.0, 0 when it stays 0.5.
 */
extern int ftRemixSandbagResetsDamageMul(int status_id);

#endif

#endif
