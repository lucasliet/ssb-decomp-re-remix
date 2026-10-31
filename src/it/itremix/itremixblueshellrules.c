#include "it/itremix/itremixblueshellrules.h"

int32_t itRemixBlueShellRulesIsStamina(int32_t game_rules)
{
    return (game_rules == ITREMIX_BLUESHELL_RULES_GAME_RULES_STAMINA);
}

/* Value test of get_player_in_first_: Stamina keeps the strictly lower value, the other rules keep the equal or higher one */
static int32_t itRemixBlueShellRulesIsBetterValue(int32_t is_lower_wins, int32_t value, int32_t best_value)
{
    if (is_lower_wins)
    {
        return (value < best_value);
    }
    return (value >= best_value);
}

/* A later port replaces the first place when it has a fighter, a better value, and on a tie a percent that is not higher */
static int32_t itRemixBlueShellRulesIsCandidate(int32_t is_lower_wins, int32_t value, int32_t percent, int32_t is_present, int32_t best_value, int32_t best_percent)
{
    if (is_present == 0)
    {
        return 0;
    }
    if (itRemixBlueShellRulesIsBetterValue(is_lower_wins, value, best_value) == 0)
    {
        return 0;
    }
    if ((value == best_value) && (best_percent < percent))
    {
        return 0;
    }
    return 1;
}

int32_t itRemixBlueShellRulesFirstPort(int32_t is_lower_wins, const int32_t values[], const int32_t percents[], const int32_t is_present[])
{
    int32_t first_port = 0;
    int32_t best_value = values[0];
    int32_t best_percent = percents[0];
    int32_t port;

    for (port = 1; port < ITREMIX_BLUESHELL_RULES_PLAYERS; port++)
    {
        if (itRemixBlueShellRulesIsCandidate(is_lower_wins, values[port], percents[port], is_present[port], best_value, best_percent))
        {
            first_port = port;
            best_value = values[port];
            best_percent = percents[port];
        }
    }
    return first_port;
}
