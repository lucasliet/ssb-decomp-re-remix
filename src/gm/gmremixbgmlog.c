/*
 * gmremixbgmlog.c - SSB64_BGM_LOG for the headless smoke tests. See gmremixbgmlog.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixbgmlog.h"
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <stdlib.h>

static s32 sGMRemixBgmLogEnabled = -1;

static sb32 gmRemixBgmLogIsEnabled(void)
{
    const char *text;

    if (sGMRemixBgmLogEnabled < 0)
    {
        text = getenv("SSB64_BGM_LOG");

        sGMRemixBgmLogEnabled = ((text != NULL) && (text[0] != '\0') && (text[0] != '0')) ? 1 : 0;
    }
    return (sGMRemixBgmLogEnabled != 0) ? TRUE : FALSE;
}

static u32 gmRemixBgmLogGetTic(void)
{
    if (gSCManagerBattleState == NULL)
    {
        return 0;
    }
    return gSCManagerBattleState->time_passed;
}

void gmRemixBgmLogPlay(s32 sngplayer, u32 bgm)
{
    if (gmRemixBgmLogIsEnabled() == FALSE)
    {
        return;
    }
    port_log("[bgm] play t=%u track=%u player=%d\n", gmRemixBgmLogGetTic(), bgm, sngplayer);
}

void gmRemixBgmLogStop(s32 sngplayer)
{
    if (gmRemixBgmLogIsEnabled() == FALSE)
    {
        return;
    }
    port_log("[bgm] stop t=%u player=%d\n", gmRemixBgmLogGetTic(), sngplayer);
}

#endif
