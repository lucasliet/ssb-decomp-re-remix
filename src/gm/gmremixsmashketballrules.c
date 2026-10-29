/**
 * gmremixsmashketballrules.c - pure scoring rules of Smashketball, see gmremixsmashketballrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixsmashketballrules.h"

/* Smashketball.winner_score_distribution: index 0 is an attributed basket, index N the unattributed share of a team of N. */
static const int dGMRemixSmashketballRulesWinnerDistribution[GMREMIX_SMASHKETBALL_RULES_PLAYERS] = { 6, 6, 3, 2 };

int gmRemixSmashketballRulesScoringTeam(int side, int home_team, int away_team)
{
    return (side < 0) ? away_team : home_team;
}

int gmRemixSmashketballRulesBasketPoints(int is_golden)
{
    return (is_golden != 0) ? GMREMIX_SMASHKETBALL_RULES_GOLDEN_POINTS : 1;
}

int gmRemixSmashketballRulesIsOwnerScoring(int has_owner, int owner_team, int scoring_team)
{
    return ((has_owner != 0) && (owner_team == scoring_team)) ? 1 : 0;
}

int gmRemixSmashketballRulesWinnerShare(int teammates, int points)
{
    return dGMRemixSmashketballRulesWinnerDistribution[teammates] * points;
}

int gmRemixSmashketballRulesTeamScore(int team, const int *team_scores)
{
    return (team != GMREMIX_SMASHKETBALL_RULES_NO_TEAM) ? team_scores[team] : 0;
}

void gmRemixSmashketballRulesRecordTeam(int *home_team, int *away_team, int team)
{
    if (*home_team == GMREMIX_SMASHKETBALL_RULES_NO_TEAM)
    {
        *home_team = team;
        return;
    }
    if (team != *home_team)
    {
        *away_team = team;
    }
}

void gmRemixSmashketballRulesRecordBasket(int *streaks, int port)
{
    int i;

    for (i = 0; i < GMREMIX_SMASHKETBALL_RULES_PLAYERS; i++)
    {
        if (i != port)
        {
            streaks[i] = 0;
        }
    }
    streaks[port]++;
}

#endif
