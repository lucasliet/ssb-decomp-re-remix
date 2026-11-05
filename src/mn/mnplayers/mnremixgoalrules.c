/**
 * mnremixgoalrules.c - pure rules of the stock and time picker of the Remix VS character select, see mnremixgoalrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixgoalrules.h"

#define MNREMIX_GOAL_PICKER_PADDING 16
#define MNREMIX_GOAL_CURSOR_HOT_X 20.0F
#define MNREMIX_GOAL_CURSOR_HOT_Y 3.0F
#define MNREMIX_GOAL_THREE_DIGITS_FROM 100

/* VsRemixMenu.asm 856-863: 1/2/3/5/8/10/12/15/20/30/60/99/infinite. */
static const int sMNRemixGoalTimeValues[MNREMIX_GOAL_VALUE_COUNT] = { 1, 2, 3, 5, 8, 10, 12, 15, 20, 30, 60, 99, 100 };
static const int sMNRemixGoalKothValues[MNREMIX_GOAL_VALUE_COUNT] = { 5, 10, 15, 20, 25, 30, 45, 60, 90, 120, 240, 480, 960 };
static const int sMNRemixGoalSmashketballValues[MNREMIX_GOAL_VALUE_COUNT] = { 1, 2, 3, 4, 5, 6, 7, 10, 15, 20, 50, 99, 100 };

const int *mnRemixGoalRulesValues(int table)
{
    switch (table)
    {
    case MNREMIX_GOAL_TABLE_TIME:
        return sMNRemixGoalTimeValues;

    case MNREMIX_GOAL_TABLE_KOTH:
        return sMNRemixGoalKothValues;

    case MNREMIX_GOAL_TABLE_SMASHKETBALL:
        return sMNRemixGoalSmashketballValues;

    default:
        return 0;
    }
}

int mnRemixGoalRulesNext(const int *values, int current)
{
    int i;

    for (i = 0; i < MNREMIX_GOAL_VALUE_COUNT; i++)
    {
        if (current < values[i])
        {
            return values[i];
        }
    }
    return values[0];
}

int mnRemixGoalRulesPrev(const int *values, int current)
{
    int i;

    for (i = MNREMIX_GOAL_VALUE_COUNT - 1; i >= 0; i--)
    {
        if (values[i] < current)
        {
            return values[i];
        }
    }
    return values[MNREMIX_GOAL_VALUE_COUNT - 1];
}

float mnRemixGoalRulesNumberX(int number)
{
    return (number < MNREMIX_GOAL_THREE_DIGITS_FROM) ? 212.0F : 218.0F;
}

int mnRemixGoalRulesPickerPressed(float cursor_x, float cursor_y, float picker_x, float picker_y, int picker_width,
                                  int picker_height)
{
    float hot_x = cursor_x + MNREMIX_GOAL_CURSOR_HOT_X;
    float hot_y = cursor_y + MNREMIX_GOAL_CURSOR_HOT_Y;
    float zone_left = picker_x + MNREMIX_GOAL_PICKER_PADDING;
    float zone_right = zone_left + (float)(picker_width - (2 * MNREMIX_GOAL_PICKER_PADDING));

    if ((hot_y < picker_y) || (hot_y > (picker_y + (float)picker_height)))
    {
        return 0;
    }
    return ((hot_x >= zone_left) && (hot_x <= zone_right)) ? 1 : 0;
}

int mnRemixGoalRulesToggleGoal(MNRemixGoalState *state, int picker_value)
{
    if (state->is_showing_time == 0)
    {
        state->is_showing_time = 1;
        state->saved_goal = picker_value;
        return state->vs_time;
    }
    state->is_showing_time = 0;
    state->vs_time = picker_value;
    return state->saved_goal;
}

int mnRemixGoalRulesSettleGoal(MNRemixGoalState *state, int picker_value)
{
    if (state->is_showing_time == 0)
    {
        return picker_value;
    }
    state->is_showing_time = 0;
    state->vs_time = picker_value;
    return state->saved_goal;
}

#endif
