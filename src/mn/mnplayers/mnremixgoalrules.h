#ifndef _MNREMIXGOALRULES_H_
#define _MNREMIXGOALRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the stock and time picker of the Remix VS character select, so the headless unit tests (tests/remix) can run
 * them: the value tables of the picker (VsRemixMenu.asm change_time_options, KingOfTheHill.asm time_value_array,
 * Smashketball.asm score_value_array), the press zone of the picker and the Time toggle of the picker (CharacterSelect.asm
 * check_picker_press_ with showing_time and saved_nontime_value).
 */

#define MNREMIX_GOAL_VALUE_COUNT 13

#define MNREMIX_GOAL_TABLE_TIME 0
#define MNREMIX_GOAL_TABLE_KOTH 1
#define MNREMIX_GOAL_TABLE_SMASHKETBALL 2

/**
 * Values the picker steps through.
 *
 * @param table MNREMIX_GOAL_TABLE_*: the time values of every Remix VS match, or the goal values of a mode.
 * @return Table of MNREMIX_GOAL_VALUE_COUNT ascending values, NULL for an unknown table.
 */
extern const int *mnRemixGoalRulesValues(int table);

/** First value of `values` above `current`, or the first value when there is none (get_next_value). */
extern int mnRemixGoalRulesNext(const int *values, int current);

/** Last value of `values` below `current`, or the last value when there is none (get_prev_value). */
extern int mnRemixGoalRulesPrev(const int *values, int current);

/** Right edge x of the number of the picker: 3 digit numbers move 6 pixels right (allow_3_digits_for_picker_). */
extern float mnRemixGoalRulesNumberX(int number);

/**
 * TRUE when the cursor presses the picker image (check_picker_press_, check_image_footer_press_ with 16 pixels of padding
 * taken off each side, check_press_ with the hot spot of the cursor at x + 20 and y + 3).
 *
 * @param cursor_x Cursor sprite x.
 * @param cursor_y Cursor sprite y.
 * @param picker_x Picker image x.
 * @param picker_y Picker image y.
 * @param picker_width Picker image width.
 * @param picker_height Picker image height.
 * @return Nonzero when the hot spot is inside the zone.
 */
extern int mnRemixGoalRulesPickerPressed(float cursor_x, float cursor_y, float picker_x, float picker_y, int picker_width,
                                         int picker_height);

/** State of the Time toggle of the goal modes (CharacterSelect.showing_time, saved_nontime_value, VsRemixMenu.global_time). */
typedef struct MNRemixGoalState
{
    int is_showing_time;
    int saved_goal;
    int vs_time;
} MNRemixGoalState;

/**
 * Toggles the picker of King of the Hill or Smashketball between its goal and the Time of the match.
 *
 * @param state Toggle state, updated.
 * @param picker_value Value the picker shows now.
 * @return Value the picker shows after the toggle.
 */
extern int mnRemixGoalRulesToggleGoal(MNRemixGoalState *state, int picker_value);

/**
 * Splits the picker of a goal mode into goal and Time when the CSS is left or the match starts (leave_css_setup_ and
 * start_match_setup_): with the Time showing, the picker holds the Time and the goal is the saved one.
 *
 * @param state Toggle state, updated: the Time showing is cleared and vs_time holds the Time to keep.
 * @param picker_value Value the picker shows now.
 * @return The goal.
 */
extern int mnRemixGoalRulesSettleGoal(MNRemixGoalState *state, int picker_value);

#endif

#endif
