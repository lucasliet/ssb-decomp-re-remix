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

const GMRemixTwelveCBState* gmRemixTwelveCBGetState(void)
{
    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return NULL;
    }
    gmRemixTwelveCBEnsureInit();
    gmRemixTwelveCBCalculateBest(&sGMRemixTwelveCB);
    return &sGMRemixTwelveCB;
}

sb32 gmRemixTwelveCBIsComplete(void)
{
    return (sGMRemixTwelveCBIsInit != FALSE) && (sGMRemixTwelveCB.status == GMREMIX_TWELVECB_STATUS_COMPLETE);
}

static s32 sGMRemixTwelveCBCssSet[GMREMIX_TWELVECB_PLAYERS];

s32 gmRemixTwelveCBGetCssSet(s32 port)
{
    return ((port >= 0) && (port < GMREMIX_TWELVECB_PLAYERS)) ? sGMRemixTwelveCBCssSet[port] : 0;
}

void gmRemixTwelveCBSetCssSet(s32 port, s32 set)
{
    if ((port >= 0) && (port < GMREMIX_TWELVECB_PLAYERS))
    {
        sGMRemixTwelveCBCssSet[port] = set;
    }
}

sb32 gmRemixTwelveCBTrySetStocks(s32 stocks)
{
    gmRemixTwelveCBEnsureInit();
    if (sGMRemixTwelveCB.status != GMREMIX_TWELVECB_STATUS_NOT_STARTED)
    {
        return FALSE;
    }
    gmRemixTwelveCBUpdateStocks(&sGMRemixTwelveCB, stocks);
    return TRUE;
}

/* Portrait slot the CSS reported for each port, -1 when none. */
static s32 sGMRemixTwelveCBCssPortrait[GMREMIX_TWELVECB_PLAYERS] = { -1, -1 };

void gmRemixTwelveCBSetCssPortrait(s32 port, s32 slot)
{
    if ((port >= 0) && (port < GMREMIX_TWELVECB_PLAYERS))
    {
        sGMRemixTwelveCBCssPortrait[port] = slot;
    }
}

/** Portrait of a port: the one the CSS reported, else the default set slot of the fighter. */
static s32 gmRemixTwelveCBPortraitOfPort(s32 port, s32 fkind)
{
    if ((port >= 0) && (port < GMREMIX_TWELVECB_PLAYERS) && (sGMRemixTwelveCBCssPortrait[port] >= 0))
    {
        return sGMRemixTwelveCBCssPortrait[port];
    }
    return gmRemixTwelveCBPortraitOf(fkind);
}

static void gmRemixTwelveCBPickPortraits(void)
{
    s32 port;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        gmRemixTwelveCBSetNextPortrait(&sGMRemixTwelveCB, port,
                                       gmRemixTwelveCBPortraitOfPort(port, gSCManagerBattleState->players[port].fkind));
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

sb32 gmRemixTwelveCBCheckSlot(s32 slot, s32 pkind, s32 fkind)
{
    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return TRUE;
    }
    if (slot >= GMREMIX_TWELVECB_PLAYERS)
    {
        return (pkind == nFTPlayerKindNot) ? TRUE : FALSE;
    }
    if ((sGMRemixTwelveCBIsInit == FALSE) || (gmRemixTwelveCBIsComplete() != FALSE) || (pkind == nFTPlayerKindNot))
    {
        return TRUE;
    }
    return (gmRemixTwelveCBGetPortraitStocks(&sGMRemixTwelveCB, gmRemixTwelveCBPortraitOfPort(slot, fkind)) > 0) ? TRUE : FALSE;
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
