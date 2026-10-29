/**
 * gmremixsmashketballscore.c - Remix Smashketball teams and baskets (Smashketball.asm _scored), see gmremixsmashketballscore.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <it/itremix/itremixbasketball.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sys/objman.h>
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixsmashketballscore.h"
#include "gm/gmremixsmashketballstate.h"

#define GMREMIX_SMASHKETBALL_GOLDEN_POINTS 3
#define GMREMIX_SMASHKETBALL_FGM_BASKET 0x118
#define GMREMIX_SMASHKETBALL_FGM_CHEER 0x272
#define GMREMIX_SMASHKETBALL_FGM_GROAN 0x267

extern void *func_800269C0_275C0(u16 id);
extern GObj* efManagerDeadExplodeMakeEffect(Vec3f *pos, s32 player, u32 type);

/* Smashketball.winner_score_distribution: index 0 is an attributed basket, index N the unattributed share of a team of N. */
static const s32 dGMRemixSmashketballWinnerDistribution[GMCOMMON_PLAYERS_MAX] = { 6, 6, 3, 2 };

void gmRemixSmashketballFindTeams(void)
{
    GObj *fighter_gobj;

    gGMRemixSmashketball.home_team = GMREMIX_SMASHKETBALL_NO_TEAM;
    gGMRemixSmashketball.away_team = GMREMIX_SMASHKETBALL_NO_TEAM;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        s32 team = ftGetStruct(fighter_gobj)->team;

        gGMRemixSmashketball.teammates[team]++;

        if (gGMRemixSmashketball.home_team == GMREMIX_SMASHKETBALL_NO_TEAM)
        {
            gGMRemixSmashketball.home_team = team;
        }
        else if (team != gGMRemixSmashketball.home_team)
        {
            gGMRemixSmashketball.away_team = team;
        }
    }
}

s32 gmRemixSmashketballGetTeamScore(s32 team)
{
    return (team != GMREMIX_SMASHKETBALL_NO_TEAM) ? gGMRemixSmashketball.team_scores[team] : 0;
}

/* Unattributed basket: every fighter of the scoring team shares winner score points by team size. */
static void gmRemixSmashketballShareWinnerScore(s32 team, s32 points)
{
    GObj *fighter_gobj;
    s32 share = dGMRemixSmashketballWinnerDistribution[gGMRemixSmashketball.teammates[team]] * points;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (fp->team == team)
        {
            gGMRemixSmashketball.winner_scores[fp->player] += share;
        }
    }
}

/* Attributed basket: the owner scores for the player and for the team, and keeps a streak. Returns the crowd sound. */
static s32 gmRemixSmashketballScoreOwner(FTStruct *owner_fp, s32 points)
{
    s32 port = owner_fp->player;
    s32 i;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        if (i != port)
        {
            gGMRemixSmashketball.consecutive_baskets[i] = 0;
        }
    }
    gGMRemixSmashketball.consecutive_baskets[port]++;
    gGMRemixSmashketball.scores[port] += points;
    gGMRemixSmashketball.winner_scores[port] += dGMRemixSmashketballWinnerDistribution[0] * points;

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
    s32 team = (side < 0) ? gGMRemixSmashketball.away_team : gGMRemixSmashketball.home_team;
    s32 points = (itRemixBasketballIsGoldenBall(ball_gobj) != FALSE) ? GMREMIX_SMASHKETBALL_GOLDEN_POINTS : 1;
    GObj *owner_gobj = itGetStruct(ball_gobj)->owner_gobj;
    FTStruct *owner_fp = (owner_gobj != NULL) ? ftGetStruct(owner_gobj) : NULL;
    s32 crowd_fgm = 0;

    if (team == GMREMIX_SMASHKETBALL_NO_TEAM)
    {
        return;
    }
    gGMRemixSmashketball.team_scores[team] += points;

    if ((owner_fp == NULL) || (owner_fp->team != team))
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
