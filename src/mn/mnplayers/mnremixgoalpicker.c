/**
 * mnremixgoalpicker.c - goal picker of King of the Hill and Smashketball on the VS character select, and the Time toggle of
 * the picker (VsRemixMenu.asm get_next_value and get_prev_value, KingOfTheHill.asm time_value_array, Smashketball.asm
 * score_value_array and use_goal_yellow_picker_, CharacterSelect.asm check_picker_press_). The CSS time picker of the VS
 * screen (mnplayersvs.c) asks this module for the next and previous value, the image to draw and the toggle.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "gm/gmremixmode.h"
#include "mn/mnplayers/mnremixgoalpicker.h"
#include "mn/mnplayers/mnremixgoalrules.h"
#include "mn/mnvsmode/mnremixvsmode.h"

static MNRemixGoalState sMNRemixGoalState;
static sb32 sMNRemixGoalIsTimedStockTime;
static s32 sMNRemixGoalRuleBeforeToggle;

/**
 * Value table of the picker.
 *
 * @return Goal values of the picked mode while the goal shows, else the Time values.
 */
static const s32 *mnRemixGoalPickerValues(void)
{
	if ((mnRemixGoalPickerIsActive() == FALSE) || (sMNRemixGoalState.is_showing_time != 0))
	{
		return mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_TIME);
	}
	if (gmRemixModeIs(MNREMIX_VSMODE_KOTH) != FALSE)
	{
		return mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_KOTH);
	}
	return mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_SMASHKETBALL);
}

sb32 mnRemixGoalPickerIsActive(void)
{
	return ((gmRemixModeIs(MNREMIX_VSMODE_KOTH) != FALSE) || (gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL) != FALSE)) ? TRUE : FALSE;
}

void mnRemixGoalPickerReset(s32 vs_time)
{
	sMNRemixGoalState.is_showing_time = FALSE;
	sMNRemixGoalState.saved_goal = 0;
	sMNRemixGoalState.vs_time = gmRemixModeGetVsTime(vs_time);
	sMNRemixGoalIsTimedStockTime = FALSE;
	sMNRemixGoalRuleBeforeToggle = 0;
}

sb32 mnRemixGoalPickerIsShowingTime(void)
{
	return (sMNRemixGoalState.is_showing_time != 0) ? TRUE : FALSE;
}

s32 mnRemixGoalPickerNext(s32 current_value)
{
	return mnRemixGoalRulesNext(mnRemixGoalPickerValues(), current_value);
}

s32 mnRemixGoalPickerPrev(s32 current_value)
{
	return mnRemixGoalRulesPrev(mnRemixGoalPickerValues(), current_value);
}

f32 mnRemixGoalPickerNumberX(s32 number)
{
	return mnRemixGoalRulesNumberX(number);
}

intptr_t mnRemixGoalPickerImageOffset(intptr_t time_offset)
{
	if ((mnRemixGoalPickerIsActive() == FALSE) || (sMNRemixGoalState.is_showing_time != 0))
	{
		return time_offset;
	}
	return MNREMIX_GOAL_PICKER_SPRITE_OFFSET;
}

sb32 mnRemixGoalPickerCanToggle(s32 game_rule)
{
	if ((mnRemixGoalPickerIsActive() != FALSE) || (sMNRemixGoalIsTimedStockTime != FALSE))
	{
		return TRUE;
	}
	return (game_rule == SCBATTLE_GAMERULE_TIMED_STOCK) ? TRUE : FALSE;
}

s32 mnRemixGoalPickerToggle(s32 picker_value, s32 *game_rule)
{
	if (mnRemixGoalPickerIsActive() != FALSE)
	{
		return mnRemixGoalRulesToggleGoal(&sMNRemixGoalState, picker_value);
	}
	if (sMNRemixGoalIsTimedStockTime != FALSE)
	{
		sMNRemixGoalIsTimedStockTime = FALSE;
		*game_rule = sMNRemixGoalRuleBeforeToggle;
		return picker_value;
	}
	sMNRemixGoalIsTimedStockTime = TRUE;
	sMNRemixGoalRuleBeforeToggle = *game_rule;
	*game_rule = SCBATTLE_GAMERULE_TIME;

	return picker_value;
}

s32 mnRemixGoalPickerSettle(s32 picker_value, s32 *game_rule)
{
	if (sMNRemixGoalIsTimedStockTime != FALSE)
	{
		sMNRemixGoalIsTimedStockTime = FALSE;
		*game_rule = sMNRemixGoalRuleBeforeToggle;
	}
	if ((mnRemixGoalPickerIsActive() == FALSE) || (sMNRemixGoalState.is_showing_time == 0))
	{
		return picker_value;
	}
	picker_value = mnRemixGoalRulesSettleGoal(&sMNRemixGoalState, picker_value);
	gmRemixModeSetVsTime(sMNRemixGoalState.vs_time);

	return picker_value;
}

#endif
