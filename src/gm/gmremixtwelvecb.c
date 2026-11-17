/**
 * gmremixtwelvecb.c - ties the 12-Character Battle rules (gmremixtwelvecbrules.c) to the VS match, see gmremixtwelvecb.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixstockmode.h"
#include "gm/gmremixtwelvecb.h"
#include "gm/gmremixtwelvecbrules.h"
#include "mn/mnplayers/mnremixtwelvecbcustom.h"
#include "mn/mnplayers/mnremixtwelvecblayout.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_roster_data.h"

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

/**
 * Puts the Random portrait back on the Custom portraits of games that started from it (set_portrait_ with the placeholder), for
 * the games of a port from first_game up to the current one.
 */
static void gmRemixTwelveCBRestoreRandomPortraits(s32 first_game)
{
    s32 portraits[GMREMIX_TWELVECB_GAMES];
    s32 port;
    s32 count;
    s32 index;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        count = gmRemixTwelveCBCollectRandomPortraits(&sGMRemixTwelveCB, port, first_game, portraits);

        for (index = 0; index < count; index++)
        {
            mnRemixTwelveCBCustomSetFkind(port, portraits[index], REMIX_ID_RANDOM);
        }
    }
}

/* handle_reset_: the reset of the battle shows the Random portrait again on every portrait whose game started from it. */
void gmRemixTwelveCBReset(void)
{
    if (sGMRemixTwelveCBIsInit != FALSE)
    {
        gmRemixTwelveCBRestoreRandomPortraits(0);
    }
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

sb32 gmRemixTwelveCBCanEditStocks(s32 port, s32 slot)
{
    gmRemixTwelveCBEnsureInit();
    if ((port < 0) || (port >= GMREMIX_TWELVECB_PLAYERS) || (gmRemixStockModeIsManual(port) == FALSE))
    {
        return FALSE;
    }
    if (sGMRemixTwelveCB.status == GMREMIX_TWELVECB_STATUS_COMPLETE)
    {
        return FALSE;
    }
    return (gmRemixTwelveCBWasPortraitPlayed(&sGMRemixTwelveCB, port, slot) == 0) ? TRUE : FALSE;
}

void gmRemixTwelveCBStepStocks(s32 port, s32 slot, s32 delta)
{
    gmRemixTwelveCBEnsureInit();
    gmRemixTwelveCBStepPortraitStocks(&sGMRemixTwelveCB, port, slot, delta);
}

s32 gmRemixTwelveCBGetWinnerPortrait(s32 port)
{
    if (sGMRemixTwelveCBIsInit == FALSE)
    {
        return -1;
    }
    return gmRemixTwelveCBGetWinnerPortraitOf(&sGMRemixTwelveCB, port);
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

/**
 * Portrait of a port: the one the CSS reported (the slot under the token, TwelveCharBattle.asm get_portrait_id_ reads the same
 * panel field). Without a CSS run, as when the battle scene is entered directly, the slot the fighter holds in the half of the
 * port in his character set, so the two ports never share a portrait; else the plain fighter mapping.
 */
static s32 gmRemixTwelveCBPortraitOfPort(s32 port, s32 fkind)
{
    s32 slot;

    if ((port < 0) || (port >= GMREMIX_TWELVECB_PLAYERS))
    {
        return gmRemixTwelveCBPortraitOf(fkind);
    }
    if (sGMRemixTwelveCBCssPortrait[port] >= 0)
    {
        return sGMRemixTwelveCBCssPortrait[port];
    }
    slot = mnRemixTwelveCBLayoutSlotOfFkind(gmRemixTwelveCBGetCssSet(port), port, fkind);

    return (slot != MNREMIX_TCB_NO_SLOT) ? slot : gmRemixTwelveCBPortraitOf(fkind);
}

/* random_char_flag_vs of each port: set when the CSS resolved a token on the Random portrait, consumed by the game that starts. */
static sb32 sGMRemixTwelveCBCssRandom[GMREMIX_TWELVECB_PLAYERS];

void gmRemixTwelveCBSetCssRandom(s32 port, sb32 is_random)
{
    if ((port >= 0) && (port < GMREMIX_TWELVECB_PLAYERS))
    {
        sGMRemixTwelveCBCssRandom[port] = is_random;
    }
}

/**
 * game_setup_, Random branch: a game started from the Random portrait shows the fighter it rolled on that portrait from now on.
 * Called after gmRemixTwelveCBStartGame so the portrait of the game is the one the record holds.
 */
static void gmRemixTwelveCBRevealRandomPortraits(void)
{
    s32 port;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        const GMRemixTwelveCBMatch *match = &sGMRemixTwelveCB.players[port].matches[sGMRemixTwelveCB.current_game];

        if (match->was_random != 0)
        {
            mnRemixTwelveCBCustomSetFkind(port, match->portrait, match->fkind);
        }
    }
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
        was_random[port] = sGMRemixTwelveCBCssRandom[port];
        sGMRemixTwelveCBCssRandom[port] = FALSE;
        sGMRemixTwelveCBStocks[port] = gmRemixTwelveCBGetInitialStocks(&sGMRemixTwelveCB, port);
    }
    gmRemixTwelveCBStartGame(&sGMRemixTwelveCB, fkinds, sGMRemixTwelveCBStocks, was_random);
    gmRemixTwelveCBRevealRandomPortraits();
    port_log("twelvecb: game %d p1 fkind %d stocks %d pool %d random %d, p2 fkind %d stocks %d pool %d random %d\n",
             (int)sGMRemixTwelveCB.current_game, (int)fkinds[0], (int)sGMRemixTwelveCBStocks[0],
             (int)sGMRemixTwelveCB.players[0].stocks_remaining, (int)was_random[0], (int)fkinds[1], (int)sGMRemixTwelveCBStocks[1],
             (int)sGMRemixTwelveCB.players[1].stocks_remaining, (int)was_random[1]);
}

void gmRemixTwelveCBUndoCurrentGame(void)
{
    if ((gmRemixTwelveCBIsActive() == FALSE) || (sGMRemixTwelveCBIsInit == FALSE))
    {
        return;
    }
    gmRemixTwelveCBRestoreRandomPortraits(sGMRemixTwelveCB.current_game);
    gmRemixTwelveCBUndoGame(&sGMRemixTwelveCB);
    port_log("twelvecb: game taken back, now game %d, pool %d / %d\n", (int)sGMRemixTwelveCB.current_game,
             (int)sGMRemixTwelveCB.players[0].stocks_remaining, (int)sGMRemixTwelveCB.players[1].stocks_remaining);
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
    if ((sGMRemixTwelveCBIsInit == FALSE) || (pkind == nFTPlayerKindNot))
    {
        return TRUE;
    }
    return (gmRemixTwelveCBSlotMayStart(gmRemixTwelveCBGetPortraitStocks(&sGMRemixTwelveCB, gmRemixTwelveCBPortraitOfPort(slot, fkind)),
                                        sGMRemixTwelveCB.players[slot].stocks_remaining) != 0) ? TRUE : FALSE;
}

sb32 gmRemixTwelveCBTokenIsFree(s32 port, sb32 is_human)
{
    if ((gmRemixTwelveCBIsActive() == FALSE) || (sGMRemixTwelveCBIsInit == FALSE))
    {
        return TRUE;
    }
    return (gmRemixTwelveCBCanPickUpToken(&sGMRemixTwelveCB, port, (is_human != FALSE) ? 1 : 0) != 0) ? TRUE : FALSE;
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
