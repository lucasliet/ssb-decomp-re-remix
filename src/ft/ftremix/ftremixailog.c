/*
 * ftremixailog.c - SSB64_REMIX_AI_LOG recovery episode log. See ftremixailog.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmdef.h>
#include <port_log.h>
#include <sc/scene.h>
#include <sc/scmanager.h>
#include <stdlib.h>
#include "ft/ftremix/ftremixailog.h"

typedef struct FTRemixAiLogEpisode
{
    sb32 is_open;
    sb32 was_airborne;

} FTRemixAiLogEpisode;

static FTRemixAiLogEpisode sFTRemixAiLogEpisodes[GMCOMMON_PLAYERS_MAX];

static sb32 ftRemixAiLogIsEnabled(void)
{
    static s32 is_enabled = -1;

    if (is_enabled < 0)
    {
        const char *text = getenv("SSB64_REMIX_AI_LOG");

        is_enabled = ((text != NULL) && (text[0] != '\0') && (text[0] != '0')) ? 1 : 0;
    }
    return (is_enabled != 0) ? TRUE : FALSE;
}

static FTRemixAiLogEpisode* ftRemixAiLogGetEpisode(FTStruct *fp)
{
    if ((fp->player < 0) || (fp->player >= GMCOMMON_PLAYERS_MAX))
    {
        return NULL;
    }
    return &sFTRemixAiLogEpisodes[fp->player];
}

static sb32 ftRemixAiLogIsSafe(FTStruct *fp)
{
    if (fp->ga == nMPKineticsGround)
    {
        return TRUE;
    }
    return ((fp->status_id == nFTCommonStatusCliffCatch) || (fp->status_id == nFTCommonStatusCliffWait)) ? TRUE : FALSE;
}

static void ftRemixAiLogEnd(FTStruct *fp, FTRemixAiLogEpisode *episode, const char *result)
{
    episode->is_open = FALSE;
    port_log("[remix-ai] recover_end t=%u port=%d result=%s\n", (u32)gSCManagerBattleState->time_passed, (int)fp->player, result);
}

void ftRemixAiLogRecoverBegin(FTStruct *fp)
{
    FTRemixAiLogEpisode *episode;
    Vec3f *pos;

    if (ftRemixAiLogIsEnabled() == FALSE)
    {
        return;
    }
    episode = ftRemixAiLogGetEpisode(fp);

    if ((episode == NULL) || (episode->is_open != FALSE))
    {
        return;
    }
    pos = &fp->joints[nFTPartsJointTopN]->translate.vec.f;

    episode->is_open = TRUE;
    episode->was_airborne = (fp->ga != nMPKineticsGround) ? TRUE : FALSE;

    port_log("[remix-ai] recover_begin t=%u port=%d fkind=%d lvl=%d x=%.0f y=%.0f\n",
             (u32)gSCManagerBattleState->time_passed, (int)fp->player, (int)fp->fkind, (int)fp->level, pos->x, pos->y);
}

void ftRemixAiLogRecoverUpdate(FTStruct *fp)
{
    FTRemixAiLogEpisode *episode;

    if (ftRemixAiLogIsEnabled() == FALSE)
    {
        return;
    }
    episode = ftRemixAiLogGetEpisode(fp);

    if ((episode == NULL) || (episode->is_open == FALSE))
    {
        return;
    }
    if (ftRemixAiLogIsSafe(fp) == FALSE)
    {
        episode->was_airborne = TRUE;
        return;
    }
    if (episode->was_airborne != FALSE)
    {
        ftRemixAiLogEnd(fp, episode, "ok");
    }
}

void ftRemixAiLogRecoverDead(FTStruct *fp)
{
    FTRemixAiLogEpisode *episode;

    if (ftRemixAiLogIsEnabled() == FALSE)
    {
        return;
    }
    episode = ftRemixAiLogGetEpisode(fp);

    if ((episode == NULL) || (episode->is_open == FALSE))
    {
        return;
    }
    ftRemixAiLogEnd(fp, episode, "ko");
}

#endif
