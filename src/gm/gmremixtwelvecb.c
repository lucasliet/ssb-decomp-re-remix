/**
 * gmremixtwelvecb.c - ties the 12-Character Battle rules (gmremixtwelvecbrules.c) to the VS match, see gmremixtwelvecb.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixtwelvecb.h"
#include "gm/gmremixtwelvecbrules.h"
#include "mn/mnvsmode/mnremixvsmode.h"

static GMRemixTwelveCBState sGMRemixTwelveCB;
static sb32 sGMRemixTwelveCBIsInit = FALSE;
static s32 sGMRemixTwelveCBStocks[GMREMIX_TWELVECB_PLAYERS];

sb32 gmRemixTwelveCBIsActive(void)
{
    return gmRemixModeIs(MNREMIX_VSMODE_TWELVE_CB);
}

static void gmRemixTwelveCBEnsureInit(void)
{
    if (sGMRemixTwelveCBIsInit != FALSE)
    {
        return;
    }
    gmRemixTwelveCBInit(&sGMRemixTwelveCB);
    sGMRemixTwelveCBIsInit = TRUE;
}

void gmRemixTwelveCBReset(void)
{
    sGMRemixTwelveCBIsInit = FALSE;
}

sb32 gmRemixTwelveCBIsComplete(void)
{
    return (sGMRemixTwelveCBIsInit != FALSE) && (sGMRemixTwelveCB.status == GMREMIX_TWELVECB_STATUS_COMPLETE);
}

static void gmRemixTwelveCBPickPortraits(void)
{
    s32 port;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        gmRemixTwelveCBSetNextPortrait(&sGMRemixTwelveCB, port,
                                       gmRemixTwelveCBPortraitOf(gSCManagerBattleState->players[port].fkind));
    }
}

void gmRemixTwelveCBBeginMatch(void)
{
    s32 fkinds[GMREMIX_TWELVECB_PLAYERS];
    s32 was_random[GMREMIX_TWELVECB_PLAYERS];
    s32 port;

    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return;
    }
    if (gmRemixTwelveCBIsComplete() != FALSE)
    {
        gmRemixTwelveCBReset();
    }
    gmRemixTwelveCBEnsureInit();

    if (sGMRemixTwelveCB.status == GMREMIX_TWELVECB_STATUS_NOT_STARTED)
    {
        gmRemixTwelveCBUpdateStocks(&sGMRemixTwelveCB, gSCManagerBattleState->stocks);
    }
    gmRemixTwelveCBPickPortraits();

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        fkinds[port] = gSCManagerBattleState->players[port].fkind;
        was_random[port] = FALSE;
        sGMRemixTwelveCBStocks[port] = gmRemixTwelveCBGetInitialStocks(&sGMRemixTwelveCB, port);
    }
    gmRemixTwelveCBStartGame(&sGMRemixTwelveCB, fkinds, sGMRemixTwelveCBStocks, was_random);
    port_log("twelvecb: game %d p1 fkind %d stocks %d pool %d, p2 fkind %d stocks %d pool %d\n",
             (int)sGMRemixTwelveCB.current_game, (int)fkinds[0], (int)sGMRemixTwelveCBStocks[0],
             (int)sGMRemixTwelveCB.players[0].stocks_remaining, (int)fkinds[1], (int)sGMRemixTwelveCBStocks[1],
             (int)sGMRemixTwelveCB.players[1].stocks_remaining);
}

s32 gmRemixTwelveCBGetStocks(s32 player, s32 match_stocks)
{
    if ((gmRemixTwelveCBIsActive() == FALSE) || (player >= GMREMIX_TWELVECB_PLAYERS))
    {
        return match_stocks;
    }
    return sGMRemixTwelveCBStocks[player];
}

void gmRemixTwelveCBOnKo(FTStruct *fp)
{
    if ((gmRemixTwelveCBIsActive() == FALSE) || (sGMRemixTwelveCBIsInit == FALSE))
    {
        return;
    }
    gmRemixTwelveCBRecordKo(&sGMRemixTwelveCB, fp->player, fp->stock_count);
    port_log("twelvecb: port %d KO, stocks left %d, pool %d, status %d\n", (int)fp->player, (int)fp->stock_count,
             (int)sGMRemixTwelveCB.players[fp->player].stocks_remaining, (int)sGMRemixTwelveCB.status);
}

#endif
