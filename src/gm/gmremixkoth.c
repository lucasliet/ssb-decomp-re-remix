/**
 * gmremixkoth.c - Remix King of the Hill state, king and score (KingOfTheHill.asm hill_update_routine_, track_score_,
 * check_match_end_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <if/interface.h>
#include <mp/map.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothdraw.h"
#include "gm/gmremixkothhill.h"
#include "gm/gmremixmode.h"
#include "mn/mnvsmode/mnremixvsmode.h"

#define GMREMIX_KOTH_FIRST_HILL_FRAMES 120
#define GMREMIX_KOTH_HILL_FRAMES_MIN 1200
#define GMREMIX_KOTH_HILL_FRAMES_RANGE 1200
#define GMREMIX_KOTH_CONTESTED_BUFFER 30
#define GMREMIX_KOTH_HIDDEN_FRAMES_MAX 120
#define GMREMIX_KOTH_WIN_FRAMES 210
#define GMREMIX_KOTH_PROC_PRIORITY 3
#define GMREMIX_KOTH_FGM_HILL_CHANGE 0xA2
#define GMREMIX_KOTH_FGM_KING_GAINED 0x115
#define GMREMIX_KOTH_FGM_KING_LOST 0xA9

extern void *func_800269C0_275C0(u16 id);
extern char *getenv(const char *name);
extern int atoi(const char *str);

typedef struct GMRemixKothState
{
    sb32 is_running;
    sb32 is_ended;
    s32 scores[GMCOMMON_PLAYERS_MAX];
    s32 team_scores[GMCOMMON_PLAYERS_MAX];
    s32 winner_times[GMCOMMON_PLAYERS_MAX];
    s32 hill_line;
    s32 timer;
    s32 hidden_frames;
    s32 king;
    s32 contested_buffer;
} GMRemixKothState;

static GMRemixKothState sGMRemixKoth;

sb32 gmRemixKothIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_KOTH);
}

s32 gmRemixKothGetScore(s32 port)
{
    return sGMRemixKoth.scores[port];
}

s32 gmRemixKothGetKing(void)
{
    return sGMRemixKoth.king;
}

s32 gmRemixKothGetHillLine(void)
{
    return sGMRemixKoth.hill_line;
}

s32 gmRemixKothGetContestedBuffer(void)
{
    return sGMRemixKoth.contested_buffer;
}

/* SSB64_KOTH_GOAL=<seconds> replaces the CSS goal, so the smoke tests can finish a match quickly. */
s32 gmRemixKothGetTargetFrames(void)
{
    const char *goal_env = getenv("SSB64_KOTH_GOAL");

    return ((goal_env != NULL) ? atoi(goal_env) : gmRemixModeGetGoal()) * 60;
}

s32 gmRemixKothGetScoreOf(s32 key)
{
    return sGMRemixKoth.team_scores[key];
}

static s32 gmRemixKothGetKey(FTStruct *fp)
{
    return fp->team;
}

sb32 gmRemixKothIsOnHill(FTStruct *fp)
{
    if (fp->coll_data.floor_line_id != sGMRemixKoth.hill_line)
    {
        return FALSE;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return FALSE;
    }
    return (fp->capture_gobj == NULL) ? TRUE : FALSE;
}

/* A fighter that holds another one only counts when the held fighter is of its own team (track_score_). */
static sb32 gmRemixKothHoldsEnemy(FTStruct *fp)
{
    FTStruct *held_fp;

    if (fp->catch_gobj == NULL)
    {
        return FALSE;
    }
    held_fp = ftGetStruct(fp->catch_gobj);

    return (gmRemixKothGetKey(held_fp) != gmRemixKothGetKey(fp)) ? TRUE : FALSE;
}

/* SSB64_KOTH_LINE=<floor line id> makes the first hill that line, so the smoke tests can put the hill under a fighter. */
static sb32 gmRemixKothPickDebugHill(void)
{
    const char *line_env = getenv("SSB64_KOTH_LINE");

    if ((line_env == NULL) || (sGMRemixKoth.hill_line >= 0))
    {
        return FALSE;
    }
    sGMRemixKoth.hill_line = atoi(line_env);
    return TRUE;
}

static void gmRemixKothPickHill(void)
{
    s32 tries;

    sGMRemixKoth.timer = GMREMIX_KOTH_HILL_FRAMES_MIN + syUtilsRandIntRange(GMREMIX_KOTH_HILL_FRAMES_RANGE);
    if (gmRemixKothPickDebugHill() != FALSE)
    {
        return;
    }

    for (tries = 0; tries < GMREMIX_KOTH_HILL_TRIES; tries++)
    {
        s32 line_id = gmRemixKothHillPickLine();

        if ((line_id < 0) || (line_id == sGMRemixKoth.hill_line))
        {
            continue;
        }
        sGMRemixKoth.hill_line = line_id;
        sGMRemixKoth.hidden_frames = 0;
        sGMRemixKoth.contested_buffer = 0;
        func_800269C0_275C0(GMREMIX_KOTH_FGM_HILL_CHANGE);
        {
            f32 x0, y0, x1, y1;
            s32 n = gmRemixKothHillGetVertexCount(line_id);

            gmRemixKothHillGetVertex(line_id, 0, &x0, &y0);
            gmRemixKothHillGetVertex(line_id, n - 1, &x1, &y1);
            port_log("koth: hill line %d vertices %d from (%d,%d) to (%d,%d)\n", (int)line_id, (int)n, (int)x0, (int)y0, (int)x1, (int)y1);
        }
        return;
    }
}

static sb32 gmRemixKothHillNeedsChange(void)
{
    if (sGMRemixKoth.hill_line < 0)
    {
        return TRUE;
    }
    if (gmRemixKothHillLineIsUsable(sGMRemixKoth.hill_line) == FALSE)
    {
        sGMRemixKoth.hidden_frames++;
        return (sGMRemixKoth.hidden_frames >= GMREMIX_KOTH_HIDDEN_FRAMES_MAX) ? TRUE : FALSE;
    }
    sGMRemixKoth.hidden_frames = 0;
    return (gmRemixKothHillIsInsideBlastZone(sGMRemixKoth.hill_line) == FALSE) ? TRUE : FALSE;
}

static void gmRemixKothUpdateHill(void)
{
    if (sGMRemixKoth.timer != 0)
    {
        sGMRemixKoth.timer--;

        if (gmRemixKothHillNeedsChange() == FALSE)
        {
            return;
        }
    }
    gmRemixKothPickHill();
}

/* Fills the on-hill state of every fighter and returns the king candidate of track_score_ (-1 none, -2 contested). */
static s32 gmRemixKothFindKing(sb32 on_hill[GMCOMMON_PLAYERS_MAX], sb32 eligible[GMCOMMON_PLAYERS_MAX])
{
    GObj *fighter_gobj;
    s32 candidate = GMREMIX_KOTH_NO_KING;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (gmRemixKothIsOnHill(fp) == FALSE)
        {
            continue;
        }
        on_hill[fp->player] = TRUE;

        if (gmRemixKothHoldsEnemy(fp) != FALSE)
        {
            candidate = GMREMIX_KOTH_CONTESTED;
            continue;
        }
        eligible[fp->player] = TRUE;

        if (candidate == GMREMIX_KOTH_NO_KING)
        {
            candidate = gmRemixKothGetKey(fp);
        }
        else if (candidate != gmRemixKothGetKey(fp))
        {
            candidate = GMREMIX_KOTH_CONTESTED;
        }
    }
    return candidate;
}

static void gmRemixKothSetKing(s32 candidate)
{
    if (candidate != GMREMIX_KOTH_CONTESTED)
    {
        if (sGMRemixKoth.contested_buffer != 0)
        {
            sGMRemixKoth.contested_buffer--;
            candidate = GMREMIX_KOTH_CONTESTED;
        }
    }
    else sGMRemixKoth.contested_buffer = GMREMIX_KOTH_CONTESTED_BUFFER;

    if (candidate == sGMRemixKoth.king)
    {
        return;
    }
    sGMRemixKoth.king = candidate;
    func_800269C0_275C0((candidate >= 0) ? GMREMIX_KOTH_FGM_KING_GAINED : GMREMIX_KOTH_FGM_KING_LOST);
}

static void gmRemixKothScoreKing(sb32 on_hill[GMCOMMON_PLAYERS_MAX], sb32 eligible[GMCOMMON_PLAYERS_MAX])
{
    GObj *fighter_gobj;
    sb32 team_scored[GMCOMMON_PLAYERS_MAX] = { FALSE, FALSE, FALSE, FALSE };
    sb32 winner_updated[GMCOMMON_PLAYERS_MAX] = { FALSE, FALSE, FALSE, FALSE };
    s32 target = gmRemixKothGetTargetFrames();

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);
        s32 key = gmRemixKothGetKey(fp);

        if (key != sGMRemixKoth.king)
        {
            sGMRemixKoth.winner_times[key] = 0;
            continue;
        }
        if ((on_hill[fp->player] == FALSE) || (eligible[fp->player] == FALSE))
        {
            continue;
        }
        if (sGMRemixKoth.team_scores[key] < target)
        {
            sGMRemixKoth.scores[fp->player]++;

            if (team_scored[key] == FALSE)
            {
                sGMRemixKoth.team_scores[key]++;
                team_scored[key] = TRUE;
            }
            continue;
        }
        if (winner_updated[key] == FALSE)
        {
            sGMRemixKoth.winner_times[key] += (gSCManagerBattleState->is_team_battle != FALSE) ? 1 : 2;
            winner_updated[key] = TRUE;
        }
    }
}

static void gmRemixKothTrackScore(void)
{
    sb32 on_hill[GMCOMMON_PLAYERS_MAX] = { FALSE, FALSE, FALSE, FALSE };
    sb32 eligible[GMCOMMON_PLAYERS_MAX] = { FALSE, FALSE, FALSE, FALSE };

    gmRemixKothSetKing(gmRemixKothFindKing(on_hill, eligible));

    if (sGMRemixKoth.king < 0)
    {
        return;
    }
    gmRemixKothScoreKing(on_hill, eligible);
}

static sb32 gmRemixKothPlayCountdown(s32 winner_time)
{
    switch (winner_time)
    {
    case 30:
        func_800269C0_275C0(nSYAudioVoiceAnnounceThree);
        return TRUE;

    case 90:
        func_800269C0_275C0(nSYAudioVoiceAnnounceTwo);
        return TRUE;

    case 150:
        func_800269C0_275C0(nSYAudioVoiceAnnounceOne);
        return TRUE;
    }
    return FALSE;
}

/* The king team gets one more frame of score when the win timer fills, then the match is over (check_match_end_). */
static void gmRemixKothEndMatch(s32 winner_key)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        if (gmRemixKothGetKey(fp) == winner_key)
        {
            sGMRemixKoth.scores[fp->player]++;
        }
    }
    sGMRemixKoth.is_ended = TRUE;
    ifCommonAnnounceEndMessage();
}

static void gmRemixKothCheckMatchEnd(void)
{
    s32 key;

    for (key = 0; key < GMCOMMON_PLAYERS_MAX; key++)
    {
        s32 winner_time = sGMRemixKoth.winner_times[key];

        if (gmRemixKothPlayCountdown(winner_time) != FALSE)
        {
            return;
        }
        if (winner_time >= GMREMIX_KOTH_WIN_FRAMES)
        {
            gmRemixKothEndMatch(key);
            return;
        }
    }
}

static void gmRemixKothProcUpdate(GObj *gobj)
{
    if ((gSCManagerBattleState->game_status != nSCBattleGameStatusGo) || (sGMRemixKoth.is_ended != FALSE))
    {
        return;
    }
    gmRemixKothUpdateHill();
    gmRemixKothTrackScore();
    gmRemixKothCheckMatchEnd();
    gmRemixKothHudUpdate();
}

void gmRemixKothSetup(void)
{
    GObj *gobj;
    s32 i;

    if (gmRemixKothIsActive() == FALSE)
    {
        return;
    }
    sGMRemixKoth.is_running = TRUE;
    sGMRemixKoth.is_ended = FALSE;
    sGMRemixKoth.hill_line = GMREMIX_KOTH_NO_LINE;
    sGMRemixKoth.timer = GMREMIX_KOTH_FIRST_HILL_FRAMES;
    sGMRemixKoth.hidden_frames = 0;
    sGMRemixKoth.king = GMREMIX_KOTH_NO_KING;
    sGMRemixKoth.contested_buffer = 0;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        sGMRemixKoth.scores[i] = 0;
        sGMRemixKoth.team_scores[i] = 0;
        sGMRemixKoth.winner_times[i] = 0;
    }
    gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, gmRemixKothProcUpdate, nGCProcessKindFunc, GMREMIX_KOTH_PROC_PRIORITY);
    gmRemixKothDrawMake();
    gmRemixKothHudSetup();
}

#endif
