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
 * Rows with values of page 1 (VsStats.asm setup_ _page_1): one damage row per active port and the three totals, plus, with
 * the Combo Meter on, one combo row per active port and the two maximums. Each one flips stripe_on in draw_line_.
 *
 * @param active_ports Ports with a fighter in the match.
 * @param is_combo_meter_on Non-zero when the Combo Meter setting is on.
 * @return Number of rows with values on page 1.
 */
int mnRemixVSStatsRulesPageOneRows(int active_ports, int is_combo_meter_on);

/**
 * Rows with values the asm draws for one results screen: page 1 and the ten rows of page 2. With the Combo Meter off the
 * asm branches past page 2 (VsStats.asm:989-995), so only the page 1 rows count.
 *
 * @param active_ports Ports with a fighter in the match.
 * @param is_combo_meter_on Non-zero when the Combo Meter setting is on.
 * @return Number of draw_line_ calls that flip stripe_on in that screen.
 */
int mnRemixVSStatsRulesScreenRows(int active_ports, int is_combo_meter_on);

/**
 * State of stripe_on after a number of rows with values (VsStats.asm draw_line_ flips the byte once per row and nothing
 * ever resets it, so it carries from page 1 to page 2 and from one results screen to the next).
 *
 * @param is_stripe_on stripe_on before the rows.
 * @param rows Rows with values drawn.
 * @return 1 when the row after them is striped, else 0.
 */
int mnRemixVSStatsRulesStripeAfterRows(int is_stripe_on, int rows);

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
