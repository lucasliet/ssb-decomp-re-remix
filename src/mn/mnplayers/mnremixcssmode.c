/**
 * mnremixcssmode.c - Remix VS modes on the VS character select, see mnremixcssmode.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/ftdef.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixtwelvecb.h"
#include "mn/mnplayers/mnremixcssmode.h"
#include "mn/mnplayers/mnremixcssmoderules.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_modes.h"

extern void *func_800269C0_275C0(u16 id);

_Static_assert(MNREMIX_CSS_MODE_COUNT == (MNREMIX_VSMODE_TUG_OF_WAR + 1), "the mode tables follow MNREMIX_VSMODE_*");

/** The finished 12-Character Battle announces the winner (TwelveCharBattle.asm update_announcer_on_entry_, STATUS_COMPLETE). */
static sb32 mnRemixCssModeIsTwelveCBComplete(void)
{
    const GMRemixTwelveCBState *state = gmRemixTwelveCBGetState();

    return ((state != NULL) && (state->status == GMREMIX_TWELVECB_STATUS_COMPLETE)) ? TRUE : FALSE;
}

intptr_t mnRemixCssModeTitleOffset(intptr_t vanilla_offset)
{
    int offset = mnRemixCssModeRulesTitleOffset(gmRemixModeGet(), gmRemixSmashketballIsSoccer());

    return (offset != 0) ? (intptr_t)offset : vanilla_offset;
}

/** FGM of the winner of the finished 12-Character Battle: "Player 1" or "Player 2". */
static int mnRemixCssModeGetTwelveCBWinnerFgm(void)
{
    const GMRemixTwelveCBState *state = gmRemixTwelveCBGetState();

    return mnRemixCssModeRulesTwelveCBWinnerFgm(state->players[0].stocks_remaining);
}

sb32 mnRemixCssModeAnnounce(void)
{
    s32 mode = gmRemixModeGet();
    int fgm = mnRemixCssModeRulesAnnouncerFgm(mode);

    if (fgm == 0)
    {
        return FALSE;
    }
    if ((mode == MNREMIX_VSMODE_TWELVE_CB) && (mnRemixCssModeIsTwelveCBComplete() != FALSE))
    {
        fgm = mnRemixCssModeGetTwelveCBWinnerFgm();
    }
    func_800269C0_275C0((u16)fgm);

    return TRUE;
}

void mnRemixCssModeAnnounceWins(u32 frame)
{
    int fgm;

    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return;
    }
    fgm = mnRemixCssModeRulesTwelveCBWinsFgm((mnRemixCssModeIsTwelveCBComplete() != FALSE) ? 1 : 0, frame);

    if (fgm != 0)
    {
        func_800269C0_275C0((u16)fgm);
    }
}

sb32 mnRemixCssModeKeepsToggleVoice(void)
{
    return (gmRemixModeGet() == MNREMIX_VSMODE_DEFAULT) ? TRUE : FALSE;
}

sb32 mnRemixCssModeIsSmashketball(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL);
}

s32 mnRemixCssModeGetTitleColorIndex(sb32 is_team_battle)
{
    if (mnRemixCssModeIsSmashketball() == FALSE)
    {
        return (is_team_battle != FALSE) ? 1 : 0;
    }
    return (gmRemixSmashketballIsSoccer() != FALSE) ? 1 : 0;
}

void mnRemixCssModeToggleSmashketballType(void)
{
    remix_modes_smashketball_set_type((remix_modes_smashketball_type() == GMREMIX_SMASHKETBALL_TYPE_SOCCER) ? GMREMIX_SMASHKETBALL_TYPE_BASKETBALL : GMREMIX_SMASHKETBALL_TYPE_SOCCER);
}

sb32 mnRemixCssModeTeamsAllowStart(const s32 *pkinds, const s32 *fkinds, const s32 *teams)
{
    int team_values[MNREMIX_CSS_MODE_TEAM_SLOTS];
    int is_present[MNREMIX_CSS_MODE_TEAM_SLOTS];
    s32 i;

    if (mnRemixCssModeIsSmashketball() == FALSE)
    {
        return TRUE;
    }
    for (i = 0; i < MNREMIX_CSS_MODE_TEAM_SLOTS; i++)
    {
        team_values[i] = teams[i];
        is_present[i] = ((pkinds[i] != nFTPlayerKindNot) && (fkinds[i] != nFTKindNull)) ? 1 : 0;
    }
    return (mnRemixCssModeRulesHasTwoTeams(team_values, is_present, MNREMIX_CSS_MODE_TEAM_SLOTS) != 0) ? TRUE : FALSE;
}

#endif
