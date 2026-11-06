/**
 * mnremixcssmoderules.c - pure rules of the Remix VS modes on the character select, see mnremixcssmoderules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcssmoderules.h"

/* TwelveCharBattle.asm 1872-1923 (update_css_header_): offsets in the game modes file, indexed by mode. */
static const int sMNRemixCssModeTitleOffsets[MNREMIX_CSS_MODE_COUNT] = { 0, 0x2048, 0x2738, 0x29A8, 0x2C18, 0x30F0 };

#define MNREMIX_CSS_MODE_SMASHKETBALL 4
#define MNREMIX_CSS_MODE_SMASHKETBALL_SOCCER_TITLE 0x2E88

/* TwelveCharBattle.asm 1752-1770 (update_announcer_on_entry_) and FGM.asm 2002-2017 (announcer.css), indexed by mode. */
static const int sMNRemixCssModeAnnouncerFgms[MNREMIX_CSS_MODE_COUNT] = { 0, 1016, 783, 775, 776, 479 };

/* FGM.asm announcer.fight PLAYER_1 / PLAYER_2 and announcer.results WINS. */
#define MNREMIX_CSS_MODE_FGM_PLAYER_1 503
#define MNREMIX_CSS_MODE_FGM_PLAYER_2 504
#define MNREMIX_CSS_MODE_FGM_WINS 533

int mnRemixCssModeRulesTitleOffset(int mode, int is_soccer)
{
    if ((mode < 0) || (mode >= MNREMIX_CSS_MODE_COUNT))
    {
        return 0;
    }
    if ((mode == MNREMIX_CSS_MODE_SMASHKETBALL) && (is_soccer != 0))
    {
        return MNREMIX_CSS_MODE_SMASHKETBALL_SOCCER_TITLE;
    }
    return sMNRemixCssModeTitleOffsets[mode];
}

int mnRemixCssModeRulesAnnouncerFgm(int mode)
{
    if ((mode < 0) || (mode >= MNREMIX_CSS_MODE_COUNT))
    {
        return 0;
    }
    return sMNRemixCssModeAnnouncerFgms[mode];
}

int mnRemixCssModeRulesTwelveCBWinnerFgm(int player_one_stocks_remaining)
{
    return (player_one_stocks_remaining == 0) ? MNREMIX_CSS_MODE_FGM_PLAYER_2 : MNREMIX_CSS_MODE_FGM_PLAYER_1;
}

int mnRemixCssModeRulesTwelveCBWinsFgm(int is_complete, unsigned int frame)
{
    return ((is_complete != 0) && (frame == MNREMIX_CSS_MODE_WINS_FRAME)) ? MNREMIX_CSS_MODE_FGM_WINS : 0;
}

int mnRemixCssModeRulesHasTwoTeams(const int *teams, const int *is_present, int count)
{
    int team_present[MNREMIX_CSS_MODE_TEAM_SLOTS] = { 0, 0, 0, 0 };
    int teams_counted = 0;
    int i;

    for (i = 0; i < count; i++)
    {
        if ((is_present[i] != 0) && (teams[i] >= 0) && (teams[i] < MNREMIX_CSS_MODE_TEAM_SLOTS))
        {
            team_present[teams[i]] = 1;
        }
    }
    for (i = 0; i < MNREMIX_CSS_MODE_TEAM_SLOTS; i++)
    {
        teams_counted += team_present[i];
    }
    return (teams_counted == 2) ? 1 : 0;
}

#endif
