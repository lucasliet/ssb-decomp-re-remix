/**
 * gmremixsmashketball.c - Remix Smashketball state, baskets, scores and match end (Smashketball.asm setup_,
 * basketball_main_, check_match_end_, start_match_setup_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gr/grremix/grremixids.h>
#include <if/interface.h>
#include <it/itremix/itremixbasketball.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixsmashketballscore.h"
#include "gm/gmremixsmashketballstate.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_debug.h"
#include "remix/remix_modes.h"

#define GMREMIX_SMASHKETBALL_PROC_PRIORITY 3
#define GMREMIX_SMASHKETBALL_BALL_WAIT_FRAMES 0x135
#define GMREMIX_SMASHKETBALL_BALL_Y 2048.0F
#define GMREMIX_SMASHKETBALL_GOLDEN_BALL_Y 5632.0F
#define GMREMIX_SMASHKETBALL_SPAWN_Y 5.0F

/* Smashketball.spawn_table: home team left to right 2, 0, 1; away team right to left 3, 1, 0. */
static const s32 dGMRemixSmashketballSpawnTable[GMCOMMON_PLAYERS_MAX] = { 2, 0, 1, 3 };

/* Spawn.asm 5A and D1 neutral spawns: slots S1 to S4 (x), the same on both stages. */
static const f32 dGMRemixSmashketballSpawnX[GMCOMMON_PLAYERS_MAX] = { -3042.0F, 3042.0F, -1615.0F, 1615.0F };

GMRemixSmashketballState gGMRemixSmashketball;

sb32 gmRemixSmashketballIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL);
}

s32 gmRemixSmashketballGetHomeTeam(void)
{
    return gGMRemixSmashketball.home_team;
}

/* SSB64_SMASH_TYPE=basketball|soccer overrides the port menu choice. */
sb32 gmRemixSmashketballIsSoccer(void)
{
    const char *type_env = remix_debug_env_text("SSB64_SMASH_TYPE");

    if (type_env != NULL)
    {
        return (type_env[0] == 's') ? TRUE : FALSE;
    }
    return (remix_modes_smashketball_type() == GMREMIX_SMASHKETBALL_TYPE_SOCCER) ? TRUE : FALSE;
}

sb32 gmRemixSmashketballIsGolden(void)
{
    return gGMRemixSmashketball.spawn_golden;
}

s32 gmRemixSmashketballGetStage(void)
{
    return (gmRemixSmashketballIsSoccer() != FALSE) ? nGRRemixStage_SOCCER : nGRRemixStage_SMASHKETBALL;
}

s32 gmRemixSmashketballGetWinnerScore(s32 port)
{
    return gGMRemixSmashketball.winner_scores[port];
}

/* SSB64_SMASH_GOAL=<score> replaces the CSS goal, so the smoke tests can finish a match quickly. */
static s32 gmRemixSmashketballGetGoal(void)
{
    return remix_debug_env_int("SSB64_SMASH_GOAL", gmRemixModeGetGoal());
}

static sb32 gmRemixSmashketballPortIsPresent(s32 port)
{
    return (gSCManagerBattleState->players[port].pkind != nFTPlayerKindNot) ? TRUE : FALSE;
}

void gmRemixSmashketballGetSpawnPos(s32 slot, f32 *x, f32 *y)
{
    *x = dGMRemixSmashketballSpawnX[slot];
    *y = GMREMIX_SMASHKETBALL_SPAWN_Y;
}

/* start_match_setup_: port 1 always takes S3, then each port fills the home or the away side in port order. */
s32 gmRemixSmashketballGetSpawnSlot(s32 player)
{
    s32 slots[GMCOMMON_PLAYERS_MAX] = { 2, 0, 0, 0 };
    s32 home_count = 0;
    s32 away_next = 3;
    s32 home_team = -1;
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        s32 team;

        if (gmRemixSmashketballPortIsPresent(port) == FALSE)
        {
            continue;
        }
        team = gSCManagerBattleState->players[port].team;

        if (home_team < 0)
        {
            home_team = team;
        }
        if (team == home_team)
        {
            slots[port] = dGMRemixSmashketballSpawnTable[home_count++];
        }
        else slots[port] = dGMRemixSmashketballSpawnTable[away_next--];
    }
    return slots[player];
}

/* SSB64_SMASH_BALL_X=<x> drops the ball at that x, so the smoke tests can send it through a hoop. */
static GObj* gmRemixSmashketballSpawnBall(void)
{
    Vec3f pos;
    Vec3f vel;

    pos.x = (f32)remix_debug_env_int("SSB64_SMASH_BALL_X", 0);
    pos.y = (gGMRemixSmashketball.spawn_golden != FALSE) ? GMREMIX_SMASHKETBALL_GOLDEN_BALL_Y : GMREMIX_SMASHKETBALL_BALL_Y;
    pos.z = 0.0F;
    vel.x = vel.y = vel.z = 0.0F;

    itRemixBasketballMakeItem(NULL, &pos, &vel, 0);
    gGMRemixSmashketball.spawn_golden = FALSE;

    return itRemixBasketballFind();
}

/* check_match_end_: a team at the goal wins; the goal 100 means there is none. */
static void gmRemixSmashketballCheckMatchEnd(void)
{
    s32 goal = gmRemixSmashketballGetGoal();
    s32 home = gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.home_team);
    s32 away = gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.away_team);

    if (gSCManagerSceneData.is_suddendeath != FALSE)
    {
        if (home == away)
        {
            return;
        }
    }
    else if (goal == GMREMIX_SMASHKETBALL_GOAL_NONE)
    {
        return;
    }
    else if ((home < goal) && (away < goal))
    {
        return;
    }
    gGMRemixSmashketball.is_ended = TRUE;
    ifCommonAnnounceEndMessage();
}

static void gmRemixSmashketballProcUpdate(GObj *gobj)
{
    GObj *ball_gobj;
    s32 side;

    gmRemixSmashketballBoardUpdate(gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.home_team),
                                   gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.away_team));

    if ((gSCManagerBattleState->game_status != nSCBattleGameStatusGo) || (gGMRemixSmashketball.is_ended != FALSE))
    {
        return;
    }
    gGMRemixSmashketball.frames++;

    if (gGMRemixSmashketball.frames < GMREMIX_SMASHKETBALL_BALL_WAIT_FRAMES)
    {
        return;
    }
    ball_gobj = itRemixBasketballFind();

    if (ball_gobj == NULL)
    {
        ball_gobj = gmRemixSmashketballSpawnBall();
    }
    if (ball_gobj == NULL)
    {
        return;
    }
    if (itRemixBasketballCheckHoop(ball_gobj, gmRemixSmashketballGetBasketsY(), gmRemixSmashketballIsSoccer(), &side) != FALSE)
    {
        gmRemixSmashketballScore(ball_gobj, side);
        gmRemixSmashketballCheckMatchEnd();
    }
}

sb32 gmRemixSmashketballPrepareSuddenDeath(void)
{
    s32 home = gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.home_team);
    s32 away = gmRemixSmashketballGetTeamScore(gGMRemixSmashketball.away_team);
    s32 i;

    if (home != away)
    {
        return FALSE;
    }
    gSCManagerVSBattleState = gSCManagerTransferBattleState;
    gSCManagerVSBattleState.pl_count = gSCManagerVSBattleState.cp_count = 0;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        if (gSCManagerVSBattleState.players[i].pkind == nFTPlayerKindMan)
        {
            gSCManagerVSBattleState.pl_count++;
        }
        else if (gSCManagerVSBattleState.players[i].pkind == nFTPlayerKindCom)
        {
            gSCManagerVSBattleState.cp_count++;
        }
    }
    gSCManagerVSBattleState.game_rules = SCBATTLE_GAMERULE_STOCK;
    gSCManagerVSBattleState.time_limit = SCBATTLE_TIMELIMIT_INFINITE;
    gSCManagerVSBattleState.is_show_score = FALSE;
    gSCManagerSceneData.is_suddendeath = TRUE;

    return TRUE;
}

s32 gmRemixSmashketballGetSuddenDeathDamage(s32 default_damage)
{
    return (gmRemixSmashketballIsActive() != FALSE) ? 0 : default_damage;
}

static void gmRemixSmashketballProcTimeOnly(GObj *gobj)
{
    gmRemixSmashketballBoardUpdateTimeOnly();
}

/* setup_ time only flag: the Smashketball stage in any other mode keeps a scoreboard with the time and dashes for the scores. */
static void gmRemixSmashketballSetupTimeOnly(void)
{
    GObj *gobj;

    if (gSCManagerBattleState->gkind != nGRRemixStage_SMASHKETBALL)
    {
        return;
    }
    gmRemixSmashketballBoardSetup();
    gmRemixSmashketballBoardShowDashScores();

    gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, gmRemixSmashketballProcTimeOnly, nGCProcessKindFunc, GMREMIX_SMASHKETBALL_PROC_PRIORITY);
}

void gmRemixSmashketballSetup(void)
{
    GObj *gobj;
    s32 i;

    if (gmRemixSmashketballIsActive() == FALSE)
    {
        gmRemixSmashketballSetupTimeOnly();
        return;
    }
    gGMRemixSmashketball.frames = 0;
    gGMRemixSmashketball.is_ended = FALSE;
    gGMRemixSmashketball.spawn_golden = FALSE;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        if (gSCManagerSceneData.is_suddendeath == FALSE)
        {
            gGMRemixSmashketball.scores[i] = 0;
            gGMRemixSmashketball.team_scores[i] = 0;
            gGMRemixSmashketball.winner_scores[i] = 0;
        }
        gGMRemixSmashketball.teammates[i] = 0;
        gGMRemixSmashketball.consecutive_baskets[i] = 0;
    }
    gmRemixSmashketballFindTeams();

    if (gmRemixSmashketballIsSoccer() == FALSE)
    {
        gmRemixSmashketballMoveBaskets();
    }
    gmRemixSmashketballBoardSetup();
    gmRemixSmashketballBoardSetTeamColors(gGMRemixSmashketball.home_team, gGMRemixSmashketball.away_team);

    gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, gmRemixSmashketballProcUpdate, nGCProcessKindFunc, GMREMIX_SMASHKETBALL_PROC_PRIORITY);
}

#endif
