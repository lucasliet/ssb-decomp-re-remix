#ifndef _FTREMIXBURY_H_
#define _FTREMIXBURY_H_

/*
 * Bury and plunge of Smash Remix (.toolchain/pkgs/smashremix-src/src/Damage.asm bury_or_plunge_initial_, bury_initial_,
 * bury_player_, buried_main_, plunge_player_). Declared here, defined in ftremixbury.c.
 */

#include <ft/fighter.h>

/* Damage.asm bury_or_plunge_initial_ (stun_damage_action_ BURY branch): is_start is FALSE when the fighter is already buried */
void ftRemixDamageBuryOrPlungeInitial(GObj *fighter_gobj, sb32 is_start);

/* Damage.asm bury_initial_, called by ZCancel.asm for the bury punishment */
void ftRemixDamageBuryInitial(GObj *fighter_gobj);

/* TRUE while the fighter runs the buried main routine (Damage.asm buried_main_, compared through the main process) */
sb32 ftRemixDamageIsBuried(FTStruct *fp);

#endif
