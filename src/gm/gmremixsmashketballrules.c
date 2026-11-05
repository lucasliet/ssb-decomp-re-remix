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

/* Smashketball.team_colors: Color.tag.RED, BLUE, GREEN, YELLOW (Color.asm). */
static const unsigned int dGMRemixSmashketballRulesTeamColors[GMREMIX_SMASHKETBALL_RULES_PLAYERS] =
{
    0xED3636FF, 0x4E4EE9FF, 0x4EB94EFF, 0xFFDF1AFF
};

unsigned int gmRemixSmashketballRulesTeamColor(int team)
{
    return dGMRemixSmashketballRulesTeamColors[team];
}

int gmRemixSmashketballRulesIsChantStreak(int streak)
{
    return (streak >= GMREMIX_SMASHKETBALL_RULES_CHANT_STREAK) ? 1 : 0;
}

void gmRemixSmashketballRulesFollowBall(float ball_x, float ball_y, float *left, float *right, float *bottom, float *top)
{
    float padded_left = ball_x - GMREMIX_SMASHKETBALL_RULES_CAMERA_PADDING;
    float padded_right = ball_x + GMREMIX_SMASHKETBALL_RULES_CAMERA_PADDING;
    float padded_top = ball_y + GMREMIX_SMASHKETBALL_RULES_CAMERA_PADDING;

    if (padded_left < *left)
    {
        *left = padded_left;
    }
    if (*right < padded_right)
    {
        *right = padded_right;
    }
    if (GMREMIX_SMASHKETBALL_RULES_CAMERA_FLOOR < *bottom)
    {
        *bottom = GMREMIX_SMASHKETBALL_RULES_CAMERA_FLOOR;
    }
    if (*top < padded_top)
    {
        *top = padded_top;
    }
}

#endif
