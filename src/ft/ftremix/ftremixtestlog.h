#ifndef _FTREMIXTESTLOG_H_
#define _FTREMIXTESTLOG_H_

/*
 * Env-gated logs read by the headless smoke tests (PORT + REGION_REMIX only). Every log is off by default and
 * costs one cached integer test per call:
 *
 * - SSB64_REMIX_FIGHTER_LOG=1: [remix-fighter] status/hit/ammo and [remix-weapon] end lines.
 *
 * An unset, empty or "0" variable means off. `t` is the battle tic, or 0 outside a battle.
 */

#include <ft/fighter.h>
#include <wp/weapon.h>

/**
 * Logs `[remix-fighter] status t=<tic> port=<p> fkind=<k> status=<id>`.
 *
 * @param fp Fighter that changes status.
 * @param status_id Final status id, after the Remix status remaps.
 */
void ftRemixTestLogStatus(FTStruct *fp, s32 status_id);

/**
 * Logs `[remix-fighter] hit t=<tic> port=<p> fkind=<k> damage=<n> percent=<total> knockback=<f>`.
 * Call right after the fighter's percent was raised; knockback is the value left after the armor was subtracted.
 *
 * @param fp Fighter that took the damage.
 */
void ftRemixTestLogHit(FTStruct *fp);

/**
 * Logs `[remix-fighter] ammo t=<tic> port=<p> value=<n>` when the fire breath ammo of a Bowser, or of a Kirby
 * with the Bowser hat, differs from the value last logged for that port. Other fighters log nothing.
 *
 * @param fp Fighter to inspect; call once per frame.
 */
void ftRemixTestLogAmmo(FTStruct *fp);

/**
 * Remembers the battle tic at which a weapon was created, for the lifetime printed by ftRemixTestLogWeaponEnd.
 *
 * @param wp Weapon just created, with its spawn serial assigned.
 */
void ftRemixTestLogWeaponSpawn(WPStruct *wp);

/**
 * Logs `[remix-weapon] end t=<tic> kind=<wkind> lifetime=<frames>`, the frames since the weapon was created.
 *
 * @param wp Weapon about to be destroyed.
 */
void ftRemixTestLogWeaponEnd(WPStruct *wp);

#endif
