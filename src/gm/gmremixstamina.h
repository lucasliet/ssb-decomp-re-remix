#ifndef _GMREMIXSTAMINA_H_
#define _GMREMIXSTAMINA_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Smash Remix Stamina (Stamina.asm): a Timed Stock match where every fighter has one stock and a pool of HP instead of the
 * percent. The Rule button of the VS menu picks it (gGMRemixStaminaRule) and the Time/Stock button picks the HP
 * (gGMRemixStaminaHP, displayed 1..999). The asm keeps the HP minus one (TOTAL_HP); gmRemixStaminaGetTotalHP is that value.
 */

#define GMREMIX_STAMINA_HP_DEFAULT 200
#define GMREMIX_STAMINA_HP_MIN 1
#define GMREMIX_STAMINA_HP_MAX 999
#define GMREMIX_STAMINA_KNOCKBACK_PERCENT 20

/** TRUE when the VS menu Rule button is on Stamina (Stamina FFA or Stamina Team). */
extern sb32 gGMRemixStaminaRule;

/** HP of every fighter as the menu shows it (HP_MIN..HP_MAX). */
extern s32 gGMRemixStaminaHP;

/** TRUE during a battle whose rule is Stamina: the rule flag is set and the battle is a Timed Stock match. */
extern sb32 gmRemixStaminaIsActive(void);

/** Stamina.TOTAL_HP: the HP minus one, a fighter is out when its percent is above this value. */
extern s32 gmRemixStaminaGetTotalHP(void);

/** Stamina.percent_port[]: TRUE when `player` (0..3) already ran out of HP in this match. */
extern sb32 gmRemixStaminaIsDefeated(s32 player);

/** Marks `player` (0..3) as out of HP. */
extern void gmRemixStaminaSetDefeated(s32 player);

/** Stamina.percent_port_refresh: clears the defeated flags. Called at the start of each VS battle. */
extern void gmRemixStaminaResetDefeated(void);

#endif

#endif
