/**
 * gmremixmodecss.c - per-mode CSS lifecycle of the Remix VS modes: the forced rule, stocks, goal and teams of each mode
 * (VsRemixMenu.asm mode_specific_setup_), kept between visits, with the vanilla settings restored when the CSS closes.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixsmashketball.h"
#include "mn/mnvsmode/mnremixvsmode.h"

typedef enum GMRemixModeTeams
{
    GMREMIX_MODE_TEAMS_OFF,
    GMREMIX_MODE_TEAMS_ON,
    GMREMIX_MODE_TEAMS_SAVED
} GMRemixModeTeams;

typedef struct GMRemixModeProfile
{
    u8 game_rules;
    sb32 uses_stocks;
    sb32 uses_goal;
    u8 teams;
    sb32 stage_select_off;
    s32 default_stocks;
    s32 default_goal;
} GMRemixModeProfile;

typedef struct GMRemixModeSaved
{
    s32 stocks;
    s32 goal;
    sb32 teams;
} GMRemixModeSaved;

typedef struct GMRemixVanillaSettings
{
    u8 game_rules;
    u8 stocks;
    u8 time_limit;
    sb32 is_team_battle;
    sb32 is_stage_select;
} GMRemixVanillaSettings;

/* Rules, forced settings and defaults of the modes, indexed by MNREMIX_VSMODE_* (TwelveCharBattle, TagTeam, KingOfTheHill,
 * Smashketball and TugOfWar .asm before_css_setup_ and the defaults of their CSS settings). */
static const GMRemixModeProfile dGMRemixModeProfiles[] =
{
    { 0, FALSE, FALSE, GMREMIX_MODE_TEAMS_SAVED, FALSE, 0, 0 },
    { SCBATTLE_GAMERULE_STOCK, TRUE, FALSE, GMREMIX_MODE_TEAMS_OFF, FALSE, 3, 0 },
    { SCBATTLE_GAMERULE_TIMED_STOCK, TRUE, FALSE, GMREMIX_MODE_TEAMS_SAVED, FALSE, 3, 0 },
    { SCBATTLE_GAMERULE_TIME, FALSE, TRUE, GMREMIX_MODE_TEAMS_SAVED, FALSE, 0, 30 },
    { SCBATTLE_GAMERULE_TIME, FALSE, TRUE, GMREMIX_MODE_TEAMS_ON, TRUE, 0, 10 },
    { SCBATTLE_GAMERULE_TIMED_STOCK, TRUE, FALSE, GMREMIX_MODE_TEAMS_SAVED, FALSE, 3, 0 }
};

static GMRemixModeSaved sGMRemixModeSaved[ARRAY_COUNT(dGMRemixModeProfiles)];
static sb32 sGMRemixModeSavedIsInit = FALSE;
static GMRemixVanillaSettings sGMRemixModeVanilla;
static sb32 sGMRemixModeVanillaIsSaved = FALSE;

static void gmRemixModeInitSaved(void)
{
    s32 i;

    if (sGMRemixModeSavedIsInit != FALSE)
    {
        return;
    }
    sGMRemixModeSavedIsInit = TRUE;

    for (i = 0; i < ARRAY_COUNT(dGMRemixModeProfiles); i++)
    {
        sGMRemixModeSaved[i].stocks = dGMRemixModeProfiles[i].default_stocks;
        sGMRemixModeSaved[i].goal = dGMRemixModeProfiles[i].default_goal;
        sGMRemixModeSaved[i].teams = (dGMRemixModeProfiles[i].teams == GMREMIX_MODE_TEAMS_ON) ? TRUE : FALSE;
    }
}

static void gmRemixModeSaveVanilla(void)
{
    sGMRemixModeVanilla.game_rules = gSCManagerTransferBattleState.game_rules;
    sGMRemixModeVanilla.stocks = gSCManagerTransferBattleState.stocks;
    sGMRemixModeVanilla.time_limit = gSCManagerTransferBattleState.time_limit;
    sGMRemixModeVanilla.is_team_battle = gSCManagerTransferBattleState.is_team_battle;
    sGMRemixModeVanilla.is_stage_select = gSCManagerTransferBattleState.is_stage_select;
    sGMRemixModeVanillaIsSaved = TRUE;
}

static void gmRemixModeRestoreVanilla(void)
{
    if (sGMRemixModeVanillaIsSaved == FALSE)
    {
        return;
    }
    gSCManagerTransferBattleState.game_rules = sGMRemixModeVanilla.game_rules;
    gSCManagerTransferBattleState.stocks = sGMRemixModeVanilla.stocks;
    gSCManagerTransferBattleState.time_limit = sGMRemixModeVanilla.time_limit;
    gSCManagerTransferBattleState.is_team_battle = sGMRemixModeVanilla.is_team_battle;
    gSCManagerTransferBattleState.is_stage_select = sGMRemixModeVanilla.is_stage_select;
    sGMRemixModeVanillaIsSaved = FALSE;
}

static void gmRemixModeSaveFromCSS(s32 mode)
{
    const GMRemixModeProfile *profile = &dGMRemixModeProfiles[mode];
    GMRemixModeSaved *saved = &sGMRemixModeSaved[mode];

    if (profile->uses_stocks != FALSE)
    {
        saved->stocks = gSCManagerTransferBattleState.stocks;
    }
    if (profile->uses_goal != FALSE)
    {
        saved->goal = gSCManagerTransferBattleState.time_limit;
    }
    if (profile->teams == GMREMIX_MODE_TEAMS_SAVED)
    {
        saved->teams = gSCManagerTransferBattleState.is_team_battle;
    }
}

void gmRemixModeBeforeCSS(void)
{
    s32 mode = gmRemixModeGet();
    const GMRemixModeProfile *profile;
    GMRemixModeSaved *saved;

    if (mode == MNREMIX_VSMODE_DEFAULT)
    {
        return;
    }
    gmRemixModeInitSaved();

    if (sGMRemixModeVanillaIsSaved == FALSE)
    {
        gmRemixModeSaveVanilla();
    }
    profile = &dGMRemixModeProfiles[mode];
    saved = &sGMRemixModeSaved[mode];

    gSCManagerTransferBattleState.game_rules = profile->game_rules;
    gSCManagerTransferBattleState.is_team_battle = saved->teams;

    if (profile->uses_stocks != FALSE)
    {
        gSCManagerTransferBattleState.stocks = saved->stocks;
    }
    if (profile->uses_goal != FALSE)
    {
        gSCManagerTransferBattleState.time_limit = saved->goal;
    }
    if (profile->stage_select_off != FALSE)
    {
        gSCManagerTransferBattleState.is_stage_select = FALSE;
    }
}

void gmRemixModeLeaveCSS(void)
{
    s32 mode = gmRemixModeGet();

    if (mode == MNREMIX_VSMODE_DEFAULT)
    {
        return;
    }
    gmRemixModeInitSaved();
    gmRemixModeSaveFromCSS(mode);
    gmRemixModeRestoreVanilla();
}

void gmRemixModeStartMatch(void)
{
    s32 mode = gmRemixModeGet();

    if (mode == MNREMIX_VSMODE_DEFAULT)
    {
        return;
    }
    gmRemixModeInitSaved();
    gmRemixModeSaveFromCSS(mode);

    if (mode == MNREMIX_VSMODE_SMASHKETBALL)
    {
        gSCManagerTransferBattleState.gkind = gmRemixSmashketballGetStage();
        gSCManagerSceneData.gkind = gmRemixSmashketballGetStage();
    }
    if ((dGMRemixModeProfiles[mode].uses_goal != FALSE) && (sGMRemixModeVanillaIsSaved != FALSE))
    {
        gSCManagerTransferBattleState.time_limit = sGMRemixModeVanilla.time_limit;
    }
}

s32 gmRemixModeGetGoal(void)
{
    gmRemixModeInitSaved();
    return sGMRemixModeSaved[gmRemixModeGet()].goal;
}

#endif
