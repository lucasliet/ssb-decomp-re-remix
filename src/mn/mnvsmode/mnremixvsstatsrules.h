#ifndef _MNREMIXVSSTATSRULES_H_
#define _MNREMIXVSSTATSRULES_H_

/*
 * Pure arithmetic of the Smash Remix VS match stats page (VsStats.asm). Only int arguments and results, no port or
 * libultraship include, so a unit test can call every function directly.
 */

/* VsStats.asm STATS_TIME_UNTIL_ACTIVE: Results timer offset that reveals the Press A prompt */
#define MNREMIX_VSSTATS_RULES_TIME_UNTIL_ACTIVE 105

/* VsStats.asm STATS_PRESS_BUFFER_PERIOD: frames after a page or visibility change that ignore button presses */
#define MNREMIX_VSSTATS_RULES_PRESS_BUFFER_PERIOD 15

/* VsStats.asm TOTAL_PAGES */
#define MNREMIX_VSSTATS_RULES_TOTAL_PAGES 2

/**
 * Share of part in total as a whole percentage, the VsStats.asm calculate_tracker_percentage_ formula. The asm converts
 * the float quotient with cvt.w.s, which rounds to nearest with ties to even, so 1 of 8 gives 12 and 3 of 8 gives 38.
 *
 * @param part Count of the successful (or measured) events, 0 or more.
 * @param total Count of all events, part plus the failed events.
 * @return 0 to 100, or 0 when total is 0 or less.
 */
int mnRemixVSStatsRulesPercent(int part, int total);

/**
 * Raises a recorded highest value to the current one, the VsStats.asm collect_stats_midmatch comparison.
 *
 * @param highest Highest value recorded so far.
 * @param current Value of this frame.
 * @return The larger of the two.
 */
int mnRemixVSStatsRulesHighest(int highest, int current);

/**
 * Page shown after the given one, wrapping to the first page after the last (VsStats.asm check_menu_toggle_, R press).
 *
 * @param page Page index, 0 to MNREMIX_VSSTATS_RULES_TOTAL_PAGES - 1.
 * @return The next page index.
 */
int mnRemixVSStatsRulesNextPage(int page);

/**
 * TRUE once the Press A prompt is revealed (VsStats.asm check_menu_toggle_, _check_victory_dance_timer).
 *
 * @param results_tics Frames elapsed on the Results screen.
 * @param allow_exit_wait Frames until START exits the Results screen.
 * @return TRUE when results_tics + MNREMIX_VSSTATS_RULES_TIME_UNTIL_ACTIVE reaches allow_exit_wait.
 */
int mnRemixVSStatsRulesIsPromptActive(int results_tics, int allow_exit_wait);

/**
 * Press buffer countdown, one step per frame (VsStats.asm check_menu_toggle_, stats_button_press_buffer).
 *
 * @param buffer Frames left before a button press is accepted again.
 * @return buffer - 1, or 0 when it is already 0.
 */
int mnRemixVSStatsRulesTickBuffer(int buffer);

#endif
