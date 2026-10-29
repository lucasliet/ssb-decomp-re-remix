#ifndef _GMREMIXSTAMINARULES_H_
#define _GMREMIXSTAMINARULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of Stamina (Stamina.asm TOTAL_HP, percent_port), so the headless unit tests (tests/remix) can run them. */

/**
 * Total HP of the Stamina rule: the HP chosen in the VS menu minus one. A fighter is out when its percent is above it.
 *
 * hp: HP of the match (gGMRemixStaminaHP, 1..999).
 */
extern int gmRemixStaminaRulesTotalHP(int hp);

/**
 * Hit points shown in the menu for Stamina: total_hp + 1, the counter that the damage rules count down from.
 *
 * total_hp: value from gmRemixStaminaRulesTotalHP.
 */
extern int gmRemixStaminaRulesMaxHP(int total_hp);

/**
 * TRUE when `percent` is above `total_hp`, the test that KOs a fighter and makes it out of HP in Stamina.
 *
 * percent: damage percent of the fighter, or the percent it would reach with the damage of this frame.
 * total_hp: value from gmRemixStaminaRulesTotalHP.
 */
extern int gmRemixStaminaRulesIsOverTotal(int percent, int total_hp);

/**
 * Damage that brings a fighter to total_hp + 1 on the hit that KOs it, so its percent stops at the KO value.
 *
 * total_hp: value from gmRemixStaminaRulesTotalHP.
 * percent: damage percent of the fighter before the hit.
 */
extern int gmRemixStaminaRulesDamageToFinish(int total_hp, int percent);

/**
 * HP left after `damage`, counting down from total_hp + 1 and never below 0.
 *
 * total_hp: value from gmRemixStaminaRulesTotalHP.
 * damage: damage percent of the fighter.
 */
extern int gmRemixStaminaRulesHPLeft(int total_hp, int damage);

#endif

#endif
