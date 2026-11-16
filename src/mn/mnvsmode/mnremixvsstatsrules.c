#include "mn/mnvsmode/mnremixvsstatsrules.h"

/* VsStats.asm setup_: Total Dealt, Total Taken, Highest Taken; Max Hits Taken, Max Damage Taken; the draw_row calls of page 2 */
#define PAGE_ONE_TOTAL_ROWS 3
#define PAGE_ONE_COMBO_MAX_ROWS 2
#define PAGE_TWO_ROWS 10

int mnRemixVSStatsRulesPercent(int part, int total)
{
    int scaled = part * 100;
    int quotient;
    int remainder;

    if (total <= 0)
    {
        return 0;
    }
    quotient = scaled / total;
    remainder = scaled % total;

    if ((2 * remainder > total) || ((2 * remainder == total) && ((quotient & 1) != 0)))
    {
        quotient++;
    }
    return quotient;
}

int mnRemixVSStatsRulesHighest(int highest, int current)
{
    return (current > highest) ? current : highest;
}

int mnRemixVSStatsRulesPageOneRows(int active_ports, int is_combo_meter_on)
{
    int rows = active_ports + PAGE_ONE_TOTAL_ROWS;

    if (is_combo_meter_on)
    {
        rows += active_ports + PAGE_ONE_COMBO_MAX_ROWS;
    }
    return rows;
}

int mnRemixVSStatsRulesScreenRows(int active_ports, int is_combo_meter_on)
{
    int rows = mnRemixVSStatsRulesPageOneRows(active_ports, is_combo_meter_on);

    if (is_combo_meter_on)
    {
        rows += PAGE_TWO_ROWS;
    }
    return rows;
}

int mnRemixVSStatsRulesStripeAfterRows(int is_stripe_on, int rows)
{
    return (is_stripe_on != 0) ^ (rows & 1);
}

int mnRemixVSStatsRulesNextPage(int page)
{
    int next = page + 1;

    return (next < MNREMIX_VSSTATS_RULES_TOTAL_PAGES) ? next : 0;
}

int mnRemixVSStatsRulesIsPromptActive(int results_tics, int allow_exit_wait)
{
    return (results_tics + MNREMIX_VSSTATS_RULES_TIME_UNTIL_ACTIVE) >= allow_exit_wait;
}

int mnRemixVSStatsRulesTickBuffer(int buffer)
{
    return (buffer > 0) ? buffer - 1 : 0;
}
