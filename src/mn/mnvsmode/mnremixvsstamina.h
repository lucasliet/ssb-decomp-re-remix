#ifndef _MNREMIXVSSTAMINA_H_
#define _MNREMIXVSSTAMINA_H_

#include <ssb_types.h>
#include <sys/objdef.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * VS menu and CSS parts of the Remix Stamina rule (Stamina.asm vs_menu, vs_css). The sprites are read from the loaded
 * VS menu file (`file`, sMNVSModeFiles[1]) at the offsets of the asm, like mnremixvsmode.c does.
 */

/** TRUE when `rule` (MNVSModeRule) is Stamina or Stamina Team. */
extern sb32 mnRemixVSStaminaIsRule(s32 rule);

/** Rule of the VS menu for a Stamina match: Stamina Team when `is_team`, otherwise Stamina. */
extern s32 mnRemixVSStaminaGetRule(sb32 is_team);

/** Sets gGMRemixStaminaRule from the menu rule: TRUE for Stamina and Stamina Team, FALSE for the other rules. */
extern void mnRemixVSStaminaSaveRule(s32 rule);

/**
 * HP after one scroll step of the VS menu picker (1..999, wrapping). A held L/C-left/D-left or R/C-right/D-right button steps 1, the stick
 * steps 10 with snaps (Stamina.asm vs_menu _scroll_left and _scroll_right); the buttons are read from every controller.
 */
extern s32 mnRemixVSStaminaStepHP(s32 hp, sb32 is_increase);

/** Scrolls gGMRemixStaminaHP one step of the VS menu; returns TRUE when the value reached its limit. */
extern sb32 mnRemixVSStaminaScrollHP(sb32 is_increase);

/** Scrolls gGMRemixStaminaHP one arrow press of the CSS picker (10 up or down, wrapping) and lowers the Initial Damage of every port to it (stamina_scrolling_). */
extern void mnRemixVSStaminaScrollHPCSS(sb32 is_increase);

/** Lowers the Initial Damage of the panel of every port to the total HP of the Stamina rule (restrict_damage_for_stamina_). */
extern void mnRemixVSStaminaRestrictDamage(void);

/** Draws the Rule button text of a Stamina rule (Stamina image, or Stamina and Team images for Stamina Team). */
extern void mnRemixVSStaminaMakeRuleValue(GObj* rule_value_gobj, void *file, s32 rule);

/** Draws the "Stamina." and "H.P." labels of the Time/Stock button for a Stamina rule. */
extern void mnRemixVSStaminaMakeTimeStockLabels(GObj* time_stock_button_gobj, void *file);

/** Draws the HP value (3 digits at most) of the Time/Stock button. */
extern void mnRemixVSStaminaMakeHPValue(GObj* time_stock_value_gobj, s32 hp, u32 *colors);

/** Draws the HP value of the CSS rule selector (3 digits at most). */
extern void mnRemixVSStaminaMakeCSSValue(GObj* rule_selector_gobj, s32 hp, u32 *colors);

#endif

#endif
