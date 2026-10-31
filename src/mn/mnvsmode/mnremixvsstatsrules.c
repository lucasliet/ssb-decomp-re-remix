#include "mn/mnvsmode/mnremixvsstatsrules.h"

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
