#ifndef _FTREMIXCRASHSHARED_H_
#define _FTREMIXCRASHSHARED_H_

/*
 * Crash and N Crash hooks into the common statuses (.toolchain/pkgs/smashremix-src/src/Crash/Crash.asm dair_bounce_ and
 * crash_eat_sfx). Declared here, defined in ftremixcrashshared.c.
 */

#include <ft/fighter.h>

/**
 * Down aerial hit of Crash and N Crash (dair_bounce_, 0x801508EC): bounces up and restarts the down aerial at frame 35.
 * @param fighter_gobj Fighter whose down aerial hit.
 * @return TRUE when the fighter is Crash or N Crash and the bounce was applied, FALSE to keep the vanilla routine.
 */
sb32 ftRemixCrashAttackAirLwProcHit(GObj *fighter_gobj);

/**
 * Crash eats a healing item (crash_eat_sfx, 0x80145C44): plays FGM 0x5BF. Other fighters, N Crash included, stay silent.
 * @param fp Fighter that eats the item.
 */
void ftRemixCrashEatSfx(FTStruct *fp);

#endif
