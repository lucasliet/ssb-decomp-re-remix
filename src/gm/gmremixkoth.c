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
#include "gm/gmremixkothking.h"
#include "gm/gmremixkothmodel.h"
#include "gm/gmremixkothstate.h"
#include "gm/gmremixmode.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_debug.h"

#define GMREMIX_KOTH_FIRST_HILL_FRAMES 120
#define GMREMIX_KOTH_HILL_FRAMES_MIN 1200
#define GMREMIX_KOTH_HILL_FRAMES_RANGE 1200
#define GMREMIX_KOTH_HIDDEN_FRAMES_MAX 120
#define GMREMIX_KOTH_WIN_FRAMES 210
#define GMREMIX_KOTH_PROC_PRIORITY 3
#define GMREMIX_KOTH_DEAD_WAIT 120
#define GMREMIX_KOTH_DEAD_CAMERA_FRAMES 45
#define GMREMIX_KOTH_FGM_HILL_CHANGE 0xA2

extern void *func_800269C0_275C0(u16 id);

GMRemixKothState gGMRemixKoth;

sb32 gmRemixKothIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_KOTH);
}

s32 gmRemixKothGetScore(s32 port)
{
    return gGMRemixKoth.scores[port];
}

s32 gmRemixKothGetKing(void)
{
    return gGMRemixKoth.king;
}

s32 gmRemixKothGetHillLine(void)
{
    return gGMRemixKoth.hill_line;
}

s32 gmRemixKothGetContestedBuffer(void)
{
    return gGMRemixKoth.contested_buffer;
}

/* SSB64_KOTH_GOAL=<seconds> replaces the CSS goal, so the smoke tests can finish a match quickly. */
s32 gmRemixKothGetTargetFrames(void)
{
    return remix_debug_env_int("SSB64_KOTH_GOAL", gmRemixModeGetGoal()) * 60;
}

s32 gmRemixKothGetScoreOf(s32 key)
{
    return gGMRemixKoth.team_scores[key];
}

s32 gmRemixKothGetKey(FTStruct *fp)
{
    return fp->team;
}

s32 gmRemixKothGetDeadWait(s32 default_wait)
{
    return (gmRemixKothIsActive() != FALSE) ? GMREMIX_KOTH_DEAD_WAIT : default_wait;
}

void gmRemixKothDeadUpdateCamera(FTStruct *fp)
{
    if (gmRemixKothIsActive() == FALSE)
    {
        return;
    }
    if (fp->status_vars.common.dead.wait == (GMREMIX_KOTH_DEAD_WAIT - GMREMIX_KOTH_DEAD_CAMERA_FRAMES))
    {
        fp->camera_mode = nFTCameraModeGhost;
    }
}

sb32 gmRemixKothIsOnHill(FTStruct *fp)
{
    if (fp->coll_data.floor_line_id != gGMRemixKoth.hill_line)
    {
        return FALSE;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return FALSE;
    }
    return (fp->capture_gobj == NULL) ? TRUE : FALSE;
}

/* SSB64_KOTH_LINE=<floor line id> makes the first hill that line, so the smoke tests can put the hill under a fighter. */
static sb32 gmRemixKothPickDebugHill(void)
{
    s32 line = remix_debug_env_int("SSB64_KOTH_LINE", -1);

    if ((line < 0) || (gGMRemixKoth.hill_line >= 0))
    {
        return FALSE;
    }
    gGMRemixKoth.hill_line = line;
    return TRUE;
}

static void gmRemixKothPickHill(void)
{
    s32 tries;

    gGMRemixKoth.timer = GMREMIX_KOTH_HILL_FRAMES_MIN + syUtilsRandIntRange(GMREMIX_KOTH_HILL_FRAMES_RANGE);
    if (gmRemixKothPickDebugHill() != FALSE)
    {
        return;
    }

    for (tries = 0; tries < GMREMIX_KOTH_HILL_TRIES; tries++)
    {
        s32 line_id = gmRemixKothHillPickLine();

        if ((line_id < 0) || (line_id == gGMRemixKoth.hill_line))
        {
            continue;
        }
        gGMRemixKoth.hill_line = line_id;
        gGMRemixKoth.hidden_frames = 0;
        gGMRemixKoth.contested_buffer = 0;
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
    if (gGMRemixKoth.hill_line < 0)
    {
        return TRUE;
    }
    if (gmRemixKothHillLineIsUsable(gGMRemixKoth.hill_line) == FALSE)
    {
        gGMRemixKoth.hidden_frames++;
        return (gGMRemixKoth.hidden_frames >= GMREMIX_KOTH_HIDDEN_FRAMES_MAX) ? TRUE : FALSE;
    }
    gGMRemixKoth.hidden_frames = 0;
    return (gmRemixKothHillIsInsideBlastZone(gGMRemixKoth.hill_line) == FALSE) ? TRUE : FALSE;
}

static void gmRemixKothUpdateHill(void)
{
    if (gGMRemixKoth.timer != 0)
    {
        gGMRemixKoth.timer--;

        if (gmRemixKothHillNeedsChange() == FALSE)
        {
            return;
        }
    }
    gmRemixKothPickHill();
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
            gGMRemixKoth.scores[fp->player]++;
        }
    }
    gGMRemixKoth.is_ended = TRUE;
    ifCommonAnnounceEndMessage();
}

static void gmRemixKothCheckMatchEnd(void)
{
    s32 key;

    for (key = 0; key < GMCOMMON_PLAYERS_MAX; key++)
    {
        s32 winner_time = gGMRemixKoth.winner_times[key];

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
    if ((gSCManagerBattleState->game_status != nSCBattleGameStatusGo) || (gGMRemixKoth.is_ended != FALSE))
    {
        return;
    }
    gmRemixKothUpdateHill();
    gmRemixKothModelSync();
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
    gGMRemixKoth.is_running = TRUE;
    gGMRemixKoth.is_ended = FALSE;
    gGMRemixKoth.hill_line = GMREMIX_KOTH_NO_LINE;
    gGMRemixKoth.timer = GMREMIX_KOTH_FIRST_HILL_FRAMES;
    gGMRemixKoth.hidden_frames = 0;
    gGMRemixKoth.king = GMREMIX_KOTH_NO_KING;
    gGMRemixKoth.contested_buffer = 0;

    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        gGMRemixKoth.scores[i] = 0;
        gGMRemixKoth.team_scores[i] = 0;
        gGMRemixKoth.winner_times[i] = 0;
    }
    gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(gobj, gmRemixKothProcUpdate, nGCProcessKindFunc, GMREMIX_KOTH_PROC_PRIORITY);
    gmRemixKothDrawMake();
    gmRemixKothHudSetup();
}

#endif
