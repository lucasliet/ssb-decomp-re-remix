/**
 * gmremixkothking.c - Remix King of the Hill king and score (KingOfTheHill.asm track_score_), see gmremixkothking.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothdraw.h"
#include "gm/gmremixkothking.h"
#include "gm/gmremixkothstate.h"

extern void *func_800269C0_275C0(u16 id);

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
        if (gGMRemixKoth.contested_buffer != 0)
        {
            gGMRemixKoth.contested_buffer--;
            candidate = GMREMIX_KOTH_CONTESTED;
        }
    }
    else gGMRemixKoth.contested_buffer = GMREMIX_KOTH_CONTESTED_BUFFER;

    if (candidate == gGMRemixKoth.king)
    {
        return;
    }
    gGMRemixKoth.king = candidate;
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

        if (key != gGMRemixKoth.king)
        {
            gGMRemixKoth.winner_times[key] = 0;
            continue;
        }
        if ((on_hill[fp->player] == FALSE) || (eligible[fp->player] == FALSE))
        {
            continue;
        }
        if (gGMRemixKoth.team_scores[key] < target)
        {
            gGMRemixKoth.scores[fp->player]++;

            if (team_scored[key] == FALSE)
            {
                gGMRemixKoth.team_scores[key]++;
                team_scored[key] = TRUE;
            }
            continue;
        }
        if (winner_updated[key] == FALSE)
        {
            gGMRemixKoth.winner_times[key] += (gSCManagerBattleState->is_team_battle != FALSE) ? 1 : 2;
            winner_updated[key] = TRUE;
        }
    }
}

void gmRemixKothTrackScore(void)
{
    sb32 on_hill[GMCOMMON_PLAYERS_MAX] = { FALSE, FALSE, FALSE, FALSE };
    sb32 eligible[GMCOMMON_PLAYERS_MAX] = { FALSE, FALSE, FALSE, FALSE };

    gmRemixKothSetKing(gmRemixKothFindKing(on_hill, eligible));

    if (gGMRemixKoth.king < 0)
    {
        return;
    }
    gmRemixKothScoreKing(on_hill, eligible);
}

#endif
