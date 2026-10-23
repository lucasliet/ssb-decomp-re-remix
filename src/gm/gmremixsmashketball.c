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
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_modes.h"

#define GMREMIX_SMASHKETBALL_PROC_PRIORITY 3
#define GMREMIX_SMASHKETBALL_BALL_WAIT_FRAMES 0x135
#define GMREMIX_SMASHKETBALL_BALL_Y 2048.0F
#define GMREMIX_SMASHKETBALL_GOLDEN_BALL_Y 5632.0F
#define GMREMIX_SMASHKETBALL_GOLDEN_POINTS 3
#define GMREMIX_SMASHKETBALL_FGM_BASKET 0x118
#define GMREMIX_SMASHKETBALL_FGM_CHEER 0x272
#define GMREMIX_SMASHKETBALL_FGM_GROAN 0x267
#define GMREMIX_SMASHKETBALL_FGM_END_BUZZER 0x431
#define GMREMIX_SMASHKETBALL_CHANT_STREAK 3
#define GMREMIX_SMASHKETBALL_SPAWN_Y 5.0F

extern void *func_800269C0_275C0(u16 id);
extern char *getenv(const char *name);
extern int atoi(const char *str);
extern GObj* efManagerDeadExplodeMakeEffect(Vec3f *pos, s32 player, u32 type);

typedef struct GMRemixSmashketballState
{
    s32 frames;
    sb32 is_ended;
    sb32 spawn_golden;
    s32 home_team;
    s32 away_team;
    s32 scores[GMCOMMON_PLAYERS_MAX];
    s32 team_scores[GMCOMMON_PLAYERS_MAX];
    s32 winner_scores[GMCOMMON_PLAYERS_MAX];
    s32 teammates[GMCOMMON_PLAYERS_MAX];
    s32 consecutive_baskets[GMCOMMON_PLAYERS_MAX];
} GMRemixSmashketballState;

/* Smashketball.winner_score_distribution: index 0 is an attributed basket, index N the unattributed share of a team of N. */
static const s32 dGMRemixSmashketballWinnerDistribution[GMCOMMON_PLAYERS_MAX] = { 6, 6, 3, 2 };

/* Smashketball.spawn_table: home team left to right 2, 0, 1; away team right to left 3, 1, 0. */
static const s32 dGMRemixSmashketballSpawnTable[GMCOMMON_PLAYERS_MAX] = { 2, 0, 1, 3 };

/* Spawn.asm 5A and D1 neutral spawns: slots S1 to S4 (x), the same on both stages. */
static const f32 dGMRemixSmashketballSpawnX[GMCOMMON_PLAYERS_MAX] = { -3042.0F, 3042.0F, -1615.0F, 1615.0F };

static GMRemixSmashketballState sGMRemixSmashketball;

sb32 gmRemixSmashketballIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL);
}

/* SSB64_SMASH_TYPE=basketball|soccer overrides the port menu choice. */
sb32 gmRemixSmashketballIsSoccer(void)
{
    const char *type_env = getenv("SSB64_SMASH_TYPE");

    if (type_env != NULL)
    {
        return (type_env[0] == 's') ? TRUE : FALSE;
    }
    return (remix_modes_smashketball_type() == GMREMIX_SMASHKETBALL_TYPE_SOCCER) ? TRUE : FALSE;
}

sb32 gmRemixSmashketballIsGolden(void)
{
    return sGMRemixSmashketball.spawn_golden;
}

s32 gmRemixSmashketballGetStage(void)
{
    return (gmRemixSmashketballIsSoccer() != FALSE) ? nGRRemixStage_SOCCER : nGRRemixStage_SMASHKETBALL;
}

s32 gmRemixSmashketballGetWinnerScore(s32 port)
{
    return sGMRemixSmashketball.winner_scores[port];
}

/* SSB64_SMASH_GOAL=<score> replaces the CSS goal, so the smoke tests can finish a match quickly. */
static s32 gmRemixSmashketballGetGoal(void)
{
    const char *goal_env = getenv("SSB64_SMASH_GOAL");

    return (goal_env != NULL) ? atoi(goal_env) : gmRemixModeGetGoal();
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

static void gmRemixSmashketballFindTeams(void)
{
    GObj *fighter_gobj;

    sGMRemixSmashketball.home_team = GMREMIX_SMASHKETBALL_NO_TEAM;
    sGMRemixSmashketball.away_team = GMREMIX_SMASHKETBALL_NO_TEAM;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        s32 team = ftGetStruct(fighter_gobj)->team;

        sGMRemixSmashketball.teammates[team]++;

        if (sGMRemixSmashketball.home_team == GMREMIX_SMASHKETBALL_NO_TEAM)
        {
            sGMRemixSmashketball.home_team = team;
        }
        else if (team != sGMRemixSmashketball.home_team)
        {
            sGMRemixSmashketball.away_team = team;
        }
    }
}

static s32 gmRemixSmashketballGetTeamScore(s32 team)
{
    return (team != GMREMIX_SMASHKETBALL_NO_TEAM) ? sGMRemixSmashketball.team_scores[team] : 0;
}

/* Unattributed basket: every fighter of the scoring team shares winner score points by team size. */
static void gmRemixSmashketballShareWinnerScore(s32 team, s32 points)
{
    GObj *fighter_gobj;
    s32 share = dGMRemixSmashketballWinnerDistribution[sGMRemixSmashketball.teammates[team]] * points;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (fp->team == team)
        {
            sGMRemixSmashketball.winner_scores[fp->player] += share;
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
            sGMRemixSmashketball.consecutive_baskets[i] = 0;
        }
    }
    sGMRemixSmashketball.consecutive_baskets[port]++;
    sGMRemixSmashketball.scores[port] += points;
    sGMRemixSmashketball.winner_scores[port] += dGMRemixSmashketballWinnerDistribution[0] * points;

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
static void gmRemixSmashketballScore(GObj *ball_gobj, s32 side)
{
    s32 team = (side < 0) ? sGMRemixSmashketball.away_team : sGMRemixSmashketball.home_team;
    s32 points = (itRemixBasketballIsGoldenBall(ball_gobj) != FALSE) ? GMREMIX_SMASHKETBALL_GOLDEN_POINTS : 1;
    GObj *owner_gobj = itGetStruct(ball_gobj)->owner_gobj;
    FTStruct *owner_fp = (owner_gobj != NULL) ? ftGetStruct(owner_gobj) : NULL;
    s32 crowd_fgm = 0;

    if (team == GMREMIX_SMASHKETBALL_NO_TEAM)
    {
        return;
    }
    sGMRemixSmashketball.team_scores[team] += points;

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
    port_log("smashketball: score home %d away %d\n", (int)gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.home_team),
             (int)gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.away_team));
}

/* SSB64_SMASH_BALL_X=<x> drops the ball at that x, so the smoke tests can send it through a hoop. */
static GObj* gmRemixSmashketballSpawnBall(void)
{
    Vec3f pos;
    Vec3f vel;

    pos.x = (getenv("SSB64_SMASH_BALL_X") != NULL) ? (f32)atoi(getenv("SSB64_SMASH_BALL_X")) : 0.0F;
    pos.y = (sGMRemixSmashketball.spawn_golden != FALSE) ? GMREMIX_SMASHKETBALL_GOLDEN_BALL_Y : GMREMIX_SMASHKETBALL_BALL_Y;
    pos.z = 0.0F;
    vel.x = vel.y = vel.z = 0.0F;

    itRemixBasketballMakeItem(NULL, &pos, &vel, 0);
    sGMRemixSmashketball.spawn_golden = FALSE;

    return itRemixBasketballFind();
}

/* check_match_end_: a team at the goal wins; the goal 100 means there is none. */
static void gmRemixSmashketballCheckMatchEnd(void)
{
    s32 goal = gmRemixSmashketballGetGoal();
    s32 home = gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.home_team);
    s32 away = gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.away_team);

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
    sGMRemixSmashketball.is_ended = TRUE;
    func_800269C0_275C0(GMREMIX_SMASHKETBALL_FGM_END_BUZZER);
    ifCommonAnnounceEndMessage();
}

static void gmRemixSmashketballProcUpdate(GObj *gobj)
{
    GObj *ball_gobj;
    s32 side;

    gmRemixSmashketballBoardUpdate(gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.home_team),
                                   gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.away_team));

    if ((gSCManagerBattleState->game_status != nSCBattleGameStatusGo) || (sGMRemixSmashketball.is_ended != FALSE))
    {
        return;
    }
    sGMRemixSmashketball.frames++;

    if (sGMRemixSmashketball.frames < GMREMIX_SMASHKETBALL_BALL_WAIT_FRAMES)
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
    s32 home = gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.home_team);
    s32 away = gmRemixSmashketballGetTeamScore(sGMRemixSmashketball.away_team);
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

void gmRemixSmashketballSetup(void)
{
    GObj *gobj;
    s32 i;

    if (gmRemixSmashketballIsActive() == FALSE)
    {
        return;
    }
    sGMRemixSmashketball.frames = 0;
    sGMRemixSmashketball.is_ended = FALSE;
    sGMRemixSmashketball.spawn_golden = FALSE;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        if (gSCManagerSceneData.is_suddendeath == FALSE)
        {
            sGMRemixSmashketball.scores[i] = 0;
            sGMRemixSmashketball.team_scores[i] = 0;
            sGMRemixSmashketball.winner_scores[i] = 0;
        }
        sGMRemixSmashketball.teammates[i] = 0;
        sGMRemixSmashketball.consecutive_baskets[i] = 0;
    }
    gmRemixSmashketballFindTeams();

    if (gmRemixSmashketballIsSoccer() == FALSE)
    {
        gmRemixSmashketballMoveBaskets();
    }
    gmRemixSmashketballBoardSetup();

    gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, gmRemixSmashketballProcUpdate, nGCProcessKindFunc, GMREMIX_SMASHKETBALL_PROC_PRIORITY);
}

#endif
