#ifndef _MNREMIXVSSTAMINARULES_H_
#define _MNREMIXVSSTAMINARULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Stamina HP picker (Stamina.asm vs_menu add_to_stock_time_value_ _scroll_left and _scroll_right, vs_css
 * stamina_scrolling_). HP is 1 to 999. The picker wraps at both ends in the VS menu and the CSS.
 */

#define MNREMIX_VSSTAMINA_RULES_HP_MIN 1
#define MNREMIX_VSSTAMINA_RULES_HP_MAX 999

/**
 * HP after one step of the VS menu picker. A d-pad, C or L/R button step is 1. A stick step is 10 with snaps: from 1 to 9 up to 10,
 * from 10 or less down to 1, from 991 or more down to 990, and from 990 or more up wraps to 1 (the asm sets the value to its maximum
 * before the vanilla wrap test, which then wraps it).
 *
 * @param hp Current HP, 1 to 999.
 * @param is_increase TRUE for the right arrow.
 * @param is_stick TRUE when the stick moved the picker, FALSE for a button.
 */
extern int mnRemixVSStaminaRulesMenuStep(int hp, int is_increase, int is_stick);

/**
 * HP after one arrow press of the CSS picker: 10 up or down, so 1 goes to 10 and then 20; 979 and 980 go up to 990, 990 to 999 and
 * 10 down to 1. A step below 1 wraps to 999 and a step above 999 wraps to 1.
 *
 * @param hp Current HP, 1 to 999.
 * @param is_increase TRUE for the right arrow.
 */
extern int mnRemixVSStaminaRulesCssStep(int hp, int is_increase);

#endif

#endif
