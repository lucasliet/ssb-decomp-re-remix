/**
 * gmremixsmashketballscore.c - Remix Smashketball teams and baskets (Smashketball.asm _scored), see gmremixsmashketballscore.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftpublic.h>
#include <it/itremix/itremixbasketball.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sys/objman.h>
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixsmashketballrules.h"
#include "gm/gmremixsmashketballscore.h"
#include "gm/gmremixsmashketballstate.h"

#define GMREMIX_SMASHKETBALL_FGM_BASKET 0x118
#define GMREMIX_SMASHKETBALL_FGM_CHEER 0x272
#define GMREMIX_SMASHKETBALL_FGM_GROAN 0x267
#define GMREMIX_SMASHKETBALL_CHANT_KNOCKBACK 160.0F

extern void *func_800269C0_275C0(u16 id);
extern GObj* efManagerDeadExplodeMakeEffect(Vec3f *pos, s32 player, u32 type);

void gmRemixSmashketballFindTeams(void)
{
    GObj *fighter_gobj;

    gGMRemixSmashketball.home_team = GMREMIX_SMASHKETBALL_NO_TEAM;
    gGMRemixSmashketball.away_team = GMREMIX_SMASHKETBALL_NO_TEAM;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        s32 team = ftGetStruct(fighter_gobj)->team;

        gGMRemixSmashketball.teammates[team]++;
        gmRemixSmashketballRulesRecordTeam(&gGMRemixSmashketball.home_team, &gGMRemixSmashketball.away_team, team);
    }
}

s32 gmRemixSmashketballGetTeamScore(s32 team)
{
    return gmRemixSmashketballRulesTeamScore(team, gGMRemixSmashketball.team_scores);
}

/* Unattributed basket: every fighter of the scoring team shares winner score points by team size. */
static void gmRemixSmashketballShareWinnerScore(s32 team, s32 points)
{
    GObj *fighter_gobj;
    s32 share = gmRemixSmashketballRulesWinnerShare(gGMRemixSmashketball.teammates[team], points);

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (fp->team == team)
        {
            gGMRemixSmashketball.winner_scores[fp->player] += share;
        }
    }
}

/* Attributed basket: the owner scores for the player and for the team, and keeps a streak that chants from 3 on. Returns the crowd sound. */
static s32 gmRemixSmashketballScoreOwner(FTStruct *owner_fp, s32 points)
{
    s32 port = owner_fp->player;

    gmRemixSmashketballRulesRecordBasket(gGMRemixSmashketball.consecutive_baskets, port);
    gGMRemixSmashketball.scores[port] += points;
    gGMRemixSmashketball.winner_scores[port] += gmRemixSmashketballRulesWinnerShare(GMREMIX_SMASHKETBALL_RULES_ATTRIBUTED_SHARE, points);

    if (gmRemixSmashketballRulesIsChantStreak(gGMRemixSmashketball.consecutive_baskets[port]) != FALSE)
    {
        ftPublicRemixForceCall(owner_fp->fighter_gobj, GMREMIX_SMASHKETBALL_CHANT_KNOCKBACK, port);
    }
    return GMREMIX_SMASHKETBALL_FGM_CHEER;
}

static void gmRemixSmashketballPlayScoreEffects(GObj *ball_gobj, FTStruct *owner_fp, s32 side, s32 crowd_fgm)
{
    Vec3f pos;
    s32 type = (gmRemixSmashketballIsSoccer() != FALSE) ? (2 - side) : 0;

    itRemixBasketballGetPosition(ball_gobj, &pos);
    efManagerDeadExplodeMakeEffect(&pos, (owner_fp != NULL) ? owner_fp->player : 0, type);
    func_800269C0_275C0(GMREMIX_SMASHKETBALL_FGM_BASKET);

    if (crowd_fgm != 0)
    {
        func_800269C0_275C0(crowd_fgm);
    }
}

/* _scored: the left hoop credits the Away team, the right one the Home team. */
void gmRemixSmashketballScore(GObj *ball_gobj, s32 side)
{
    s32 team = gmRemixSmashketballRulesScoringTeam(side, gGMRemixSmashketball.home_team,
                                                   gGMRemixSmashketball.away_team);
    s32 points = gmRemixSmashketballRulesBasketPoints(itRemixBasketballIsGoldenBall(ball_gobj) != FALSE);
    GObj *owner_gobj = itGetStruct(ball_gobj)->owner_gobj;
    FTStruct *owner_fp = (owner_gobj != NULL) ? ftGetStruct(owner_gobj) : NULL;
    s32 owner_team = (owner_fp != NULL) ? owner_fp->team : GMREMIX_SMASHKETBALL_NO_TEAM;
    s32 crowd_fgm = 0;

    if (team == GMREMIX_SMASHKETBALL_NO_TEAM)
    {
        return;
    }
    gGMRemixSmashketball.team_scores[team] += points;

    if (gmRemixSmashketballRulesIsOwnerScoring(owner_fp != NULL, owner_team, team) == 0)
    {
        gmRemixSmashketballShareWinnerScore(team, points);

        if (owner_fp != NULL)
        {
            crowd_fgm = GMREMIX_SMASHKETBALL_FGM_GROAN;
        }
    }
    else crowd_fgm = gmRemixSmashketballScoreOwner(owner_fp, points);

    gmRemixSmashketballPlayScoreEffects(ball_gobj, owner_fp, side, crowd_fgm);
    port_log("smashketball: score home %d away %d\n", (int)gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.home_team),
             (int)gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.away_team));
}

#endif
