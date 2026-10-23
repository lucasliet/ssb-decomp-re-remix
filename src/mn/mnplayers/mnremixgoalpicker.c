/**
 * mnremixgoalpicker.c - CSS goal values of King of the Hill and Smashketball (VsRemixMenu.asm get_next_value and
 * get_prev_value, KingOfTheHill.asm time_value_array, Smashketball.asm score_value_array). The CSS time picker of the VS
 * screen (mnplayersvs.c) asks this module for the next and previous value only when one of these modes is picked.
 *
 * REMIX-TODO(koth-goal-label): the "Goal" text label and the CharacterSelect.asm showing_time toggle are not ported. The
 * picker always steps through the goal values of the mode, as the asm does with showing_time off.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "gm/gmremixmode.h"
#include "mn/mnplayers/mnremixgoalpicker.h"
#include "mn/mnvsmode/mnremixvsmode.h"

#define MNREMIX_GOAL_VALUE_COUNT 13

static const s32 sMNRemixGoalKothValues[MNREMIX_GOAL_VALUE_COUNT] = { 5, 10, 15, 20, 25, 30, 45, 60, 90, 120, 240, 480, 960 };
static const s32 sMNRemixGoalSmashketballValues[MNREMIX_GOAL_VALUE_COUNT] = { 1, 2, 3, 4, 5, 6, 7, 10, 15, 20, 50, 99, 100 };

/**
 * Goal value table of the picked mode, or NULL when the mode has no goal picker.
 *
 * @return The table of the mode, or NULL for the other modes.
 */
static const s32 *mnRemixGoalPickerValues(void)
{
	if (gmRemixModeIs(MNREMIX_VSMODE_KOTH) != FALSE)
	{
		return sMNRemixGoalKothValues;
	}
	if (gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL) != FALSE)
	{
		return sMNRemixGoalSmashketballValues;
	}
	return NULL;
}

/**
 * TRUE when the CSS time picker shows the goal of King of the Hill or Smashketball.
 *
 * @return TRUE for KOTH and Smashketball, FALSE for the other modes.
 */
sb32 mnRemixGoalPickerIsActive(void)
{
	return (mnRemixGoalPickerValues() != NULL) ? TRUE : FALSE;
}

/**
 * Next goal value after `current_value`. Like the asm, the first value above the current one is taken, and the first
 * value of the table is taken when the current value is the last one or above it.
 *
 * @param current_value Goal shown by the picker.
 * @return Next goal value of the mode, or `current_value` for the other modes.
 */
s32 mnRemixGoalPickerNext(s32 current_value)
{
	const s32 *values = mnRemixGoalPickerValues();
	s32 i;

	if (values == NULL)
	{
		return current_value;
	}
	for (i = 0; i < MNREMIX_GOAL_VALUE_COUNT; i++)
	{
		if (current_value < values[i])
		{
			return values[i];
		}
	}
	return values[0];
}

/**
 * Previous goal value before `current_value`. Like the asm, the last value below the current one is taken, and the last
 * value of the table is taken when the current value is the first one or below it.
 *
 * @param current_value Goal shown by the picker.
 * @return Previous goal value of the mode, or `current_value` for the other modes.
 */
s32 mnRemixGoalPickerPrev(s32 current_value)
{
	const s32 *values = mnRemixGoalPickerValues();
	s32 i;

	if (values == NULL)
	{
		return current_value;
	}
	for (i = MNREMIX_GOAL_VALUE_COUNT - 1; i >= 0; i--)
	{
		if (values[i] < current_value)
		{
			return values[i];
		}
	}
	return values[MNREMIX_GOAL_VALUE_COUNT - 1];
}

/**
 * Right edge x of a goal number of three digits. The asm moves the picker number 6 pixels right when it has 3 digits
 * (allow_3_digits_for_picker_, KingOfTheHill.asm line 303).
 *
 * @param number Goal shown by the picker.
 * @return X position of the right edge of the number.
 */
f32 mnRemixGoalPickerNumberX(s32 number)
{
	if (number < 100)
	{
		return 212.0F;
	}
	return 218.0F;
}

#endif
