/**
 * mnremixteamsrules.c - pure rules of the Remix team extension, see mnremixteamsrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <string.h>
#include "mn/mnplayers/mnremixteamsrules.h"

int mnRemixTeamsRulesNextTeam(int team)
{
    return (team >= MNREMIX_TEAMS_LAST) ? 0 : (team + 1);
}

int mnRemixTeamsRulesIsAdded(int team)
{
    return ((team >= MNREMIX_TEAMS_VANILLA_COUNT) && (team < MNREMIX_TEAMS_COUNT)) ? 1 : 0;
}

int mnRemixTeamsRulesVanillaColor(int team)
{
    static const int colors[MNREMIX_TEAMS_VANILLA_COUNT] = { 0, 1, 3 };

    if ((team < 0) || (team >= MNREMIX_TEAMS_VANILLA_COUNT))
    {
        return -1;
    }
    return colors[team];
}

int mnRemixTeamsRulesCostumeIndex(int fkind, int team)
{
    if ((fkind < 0) || (mnRemixTeamsRulesIsAdded(team) == 0))
    {
        return -1;
    }
    return (fkind * MNREMIX_TEAMS_ADDED_COUNT) + (team - MNREMIX_TEAMS_VANILLA_COUNT);
}

int mnRemixTeamsRulesBigTextOffset(int palette_index)
{
    return palette_index * MNREMIX_TEAMS_BIG_TEXT_ENTRY_SIZE;
}

float mnRemixTeamsRulesWordToFloat(unsigned int word)
{
    float value;

    memcpy(&value, &word, sizeof(value));

    return value;
}

int mnRemixTeamsRulesShieldSource(int is_vs_team_battle, int is_cpu)
{
    if (is_vs_team_battle != 0)
    {
        return MNREMIX_TEAMS_SHIELD_TEAM;
    }
    return (is_cpu != 0) ? MNREMIX_TEAMS_SHIELD_CPU : MNREMIX_TEAMS_SHIELD_PORT;
}

int mnRemixTeamsRulesLastResultTeam(void)
{
    return MNREMIX_TEAMS_LAST;
}

#endif
