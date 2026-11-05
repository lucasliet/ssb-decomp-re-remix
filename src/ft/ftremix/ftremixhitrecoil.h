#ifndef _FTREMIXHITRECOIL_H_
#define _FTREMIXHITRECOIL_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Hit recoil patches of Wario/Wario.asm: body_slam_recoil_ (0x800E26D0, start of ftMainSetHitInteractStats) and the four
 * body_slam_clang_*_ patches (0x800E294C and 0x800E29EC in ftMainUpdateAttackStatFighter, 0x800E2F50 in
 * ftMainUpdateAttackStatWeapon, 0x800E3608 in ftMainUpdateAttackStatItem). The rules behind them are in
 * ftremixhitrecoilrules.h.
 */

#include <ft/fighter.h>

/**
 * body_slam_recoil_: a hitbox of fp made contact. Wario and Kirby with the Wario hat in the body slam, Sheik in the down
 * special attack and Banjo in the up special attack start their recoil (temp variable 1, motion_vars.flags.flag0, set to 1).
 * Ebisumaru and Kirby with the Ebisumaru hat at the end of the neutral special pop out a Dango (99%) or a Tomato (1%) on a
 * hurtbox contact.
 *
 * @param fp Attacker struct.
 * @param hit_type GMHitType of the contact.
 */
extern void ftRemixHitRecoilOnHit(FTStruct *fp, s32 hit_type);

/**
 * check_body_slam_invincible_: whether fp is in the invincible stage of the body slam (Wario, or Kirby with the Wario hat,
 * in the neutral special with the shoulder hurtbox invincible). Such a hitbox wins every clang.
 *
 * @param fp Fighter struct.
 * @return TRUE while the shoulder is invincible.
 */
extern sb32 ftRemixHitRecoilIsBodySlamInvincible(FTStruct *fp);

#endif

#endif
