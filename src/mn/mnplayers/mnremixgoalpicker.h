#ifndef _MNREMIXGOALPICKER_H_
#define _MNREMIXGOALPICKER_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Offset of the "GOAL" yellow picker image, next to the "TIME" one, in the common CSS file (Smashketball.asm
 * use_goal_yellow_picker_: li t0, 0x00027FC8).
 */
#define MNREMIX_GOAL_PICKER_SPRITE_OFFSET 0x27FC8

/** TRUE when the picked mode is King of the Hill or Smashketball, whose picker holds a goal. */
extern sb32 mnRemixGoalPickerIsActive(void);

/** Opens the picker on the goal (CharacterSelect.showing_time off) and remembers the Time of the VS menu. */
extern void mnRemixGoalPickerReset(s32 vs_time);

/** TRUE while the picker of a goal mode shows the Time of the match instead of the goal. */
extern sb32 mnRemixGoalPickerIsShowingTime(void);

/** Next value of the picker after `current_value`: the goal values of the mode, or the Time values of every Remix VS match. */
extern s32 mnRemixGoalPickerNext(s32 current_value);

/** Previous value of the picker before `current_value`, see mnRemixGoalPickerNext. */
extern s32 mnRemixGoalPickerPrev(s32 current_value);

/** Right edge x of a number of the picker; three digit goals move 6 pixels right. */
extern f32 mnRemixGoalPickerNumberX(s32 number);

/**
 * Offset of the yellow picker image: the "GOAL" one in King of the Hill and Smashketball, the "TIME" one otherwise.
 *
 * @param time_offset Offset of the "TIME" image.
 */
extern intptr_t mnRemixGoalPickerImageOffset(intptr_t time_offset);

/**
 * TRUE when pressing the picker switches between its two readings (check_picker_press_): goal and Time in King of the Hill
 * and Smashketball, stock and Time when the rule is Timed Stock.
 *
 * @param game_rule Rule the CSS shows now.
 */
extern sb32 mnRemixGoalPickerCanToggle(s32 game_rule);

/**
 * Switches the picker between its two readings.
 *
 * @param picker_value Value the picker shows now.
 * @param game_rule Rule the CSS shows, updated: Time while a Timed Stock match shows its Time.
 * @return Value the picker shows after the switch.
 */
extern s32 mnRemixGoalPickerToggle(s32 picker_value, s32 *game_rule);

/**
 * Puts the picker back to its own reading when the CSS is left or the match starts (leave_css_setup_ and start_match_setup_).
 * The Time chosen in a goal mode becomes the Time of the VS menu (gmRemixModeSetVsTime).
 *
 * @param picker_value Value the picker shows now.
 * @param game_rule Rule the CSS shows, restored.
 * @return Value that belongs to the rule of the CSS: the goal of a goal mode, else `picker_value`.
 */
extern s32 mnRemixGoalPickerSettle(s32 picker_value, s32 *game_rule);

#endif

#endif
