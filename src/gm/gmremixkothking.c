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
#include "gm/gmremixkothrules.h"
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
    GMRemixKothRulesFighter entries[GMCOMMON_PLAYERS_MAX];
    s32 ports[GMCOMMON_PLAYERS_MAX];
    int is_eligible[GMCOMMON_PLAYERS_MAX];
    GObj *fighter_gobj;
    s32 count = 0;
    s32 i;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; (fighter_gobj != NULL) && (count < GMCOMMON_PLAYERS_MAX); fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *fp = ftGetStruct(fighter_gobj);

        entries[count].is_on_hill = (gmRemixKothIsOnHill(fp) != FALSE) ? 1 : 0;
        entries[count].holds_enemy = (gmRemixKothHoldsEnemy(fp) != FALSE) ? 1 : 0;
        entries[count].key = gmRemixKothGetKey(fp);
        ports[count] = fp->player;
        count++;
    }
    s32 candidate = gmRemixKothRulesCandidate(entries, count, is_eligible);

    for (i = 0; i < count; i++)
    {
        on_hill[ports[i]] = (entries[i].is_on_hill != 0) ? TRUE : on_hill[ports[i]];
        eligible[ports[i]] = (is_eligible[i] != 0) ? TRUE : eligible[ports[i]];
    }
    return candidate;
}

static void gmRemixKothSetKing(s32 candidate)
{
    candidate = gmRemixKothRulesSettle(candidate, &gGMRemixKoth.contested_buffer, GMREMIX_KOTH_CONTESTED_BUFFER);

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
