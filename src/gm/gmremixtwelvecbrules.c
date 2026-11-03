/**
 * gmremixtwelvecbrules.c - rules of the 12-Character Battle (TwelveCharBattle.asm), see gmremixtwelvecbrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtwelvecbrules.h"

#define GMREMIX_TWELVECB_KEY_FKIND_SHIFT 8

typedef struct GMRemixTwelveCBRun
{
    int32_t key;
    int32_t tkos_for;
    int32_t tkos_against;
} GMRemixTwelveCBRun;

typedef struct GMRemixTwelveCBRuns
{
    GMRemixTwelveCBRun runs[GMREMIX_TWELVECB_GAMES];
    int32_t count;
    int32_t own_lost;
    int32_t opponent_lost;
} GMRemixTwelveCBRuns;

static int32_t gmRemixTwelveCBPortIsValid(int32_t port)
{
    return ((port >= 0) && (port < GMREMIX_TWELVECB_PLAYERS)) ? 1 : 0;
}

static int32_t gmRemixTwelveCBPortraitIsValid(int32_t portrait)
{
    return ((portrait >= 0) && (portrait < GMREMIX_TWELVECB_PORTRAITS)) ? 1 : 0;
}

void gmRemixTwelveCBInit(GMRemixTwelveCBState *state)
{
    int32_t port;
    int32_t game;

    state->status = GMREMIX_TWELVECB_STATUS_NOT_STARTED;
    state->current_game = -1;
    state->num_stocks = -1;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        for (game = 0; game < GMREMIX_TWELVECB_GAMES; game++)
        {
            state->players[port].matches[game] = (GMRemixTwelveCBMatch){ 0, 0, 0, 0, 0 };
        }
        state->players[port].best_fkind = GMREMIX_TWELVECB_NO_FKIND;
        state->players[port].best_tkos_for = -1;
        state->players[port].best_tkos_against = 0;
    }
    gmRemixTwelveCBUpdateStocks(state, GMREMIX_TWELVECB_DEFAULT_STOCKS);
}

void gmRemixTwelveCBUpdateStocks(GMRemixTwelveCBState *state, int32_t stocks)
{
    int32_t port;
    int32_t portrait;

    if (state->num_stocks == stocks)
    {
        return;
    }
    state->num_stocks = stocks;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        state->players[port].stocks_remaining = GMREMIX_TWELVECB_CHARACTERS * (stocks + 1);
    }
    for (portrait = 0; portrait < GMREMIX_TWELVECB_PORTRAITS; portrait++)
    {
        state->stocks_by_portrait[portrait] = (int8_t)stocks;
    }
}

int32_t gmRemixTwelveCBPortraitOf(int32_t fkind)
{
    if ((fkind >= 0) && (fkind < GMREMIX_TWELVECB_CHARACTERS))
    {
        return fkind;
    }
    return GMREMIX_TWELVECB_CHARACTERS + (((fkind < 0) ? -fkind : fkind) % GMREMIX_TWELVECB_CHARACTERS);
}

int32_t gmRemixTwelveCBGetPortraitStocks(const GMRemixTwelveCBState *state, int32_t portrait)
{
    if (gmRemixTwelveCBPortraitIsValid(portrait) == 0)
    {
        return 0;
    }
    return state->stocks_by_portrait[portrait] + 1;
}

void gmRemixTwelveCBSetNextPortrait(GMRemixTwelveCBState *state, int32_t port, int32_t portrait)
{
    int32_t next_game = state->current_game + 1;

    if ((gmRemixTwelveCBPortIsValid(port) == 0) || (gmRemixTwelveCBPortraitIsValid(portrait) == 0))
    {
        return;
    }
    if (next_game >= GMREMIX_TWELVECB_GAMES)
    {
        return;
    }
    state->players[port].matches[next_game].portrait = (uint8_t)portrait;
}

int32_t gmRemixTwelveCBGetInitialStocks(const GMRemixTwelveCBState *state, int32_t port)
{
    const GMRemixTwelveCBPlayer *player = &state->players[port];
    int32_t next_game = state->current_game + 1;
    int32_t previous_end;

    if (next_game >= GMREMIX_TWELVECB_GAMES)
    {
        return 0;
    }
    if (state->current_game >= 0)
    {
        previous_end = (int8_t)player->matches[state->current_game].stocks_end;

        if (previous_end > 0)
        {
            return previous_end;
        }
    }
    return state->stocks_by_portrait[player->matches[next_game].portrait];
}

void gmRemixTwelveCBStartGame(GMRemixTwelveCBState *state, const int32_t fkinds[GMREMIX_TWELVECB_PLAYERS],
                              const int32_t stocks[GMREMIX_TWELVECB_PLAYERS],
                              const int32_t was_random[GMREMIX_TWELVECB_PLAYERS])
{
    int32_t port;
    GMRemixTwelveCBMatch *match;

    if ((state->current_game + 1) >= GMREMIX_TWELVECB_GAMES)
    {
        return;
    }
    state->status = GMREMIX_TWELVECB_STATUS_STARTED;
    state->current_game++;

    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        match = &state->players[port].matches[state->current_game];
        match->fkind = (uint8_t)fkinds[port];
        match->stocks_start = (uint8_t)stocks[port];
        match->stocks_end = (uint8_t)stocks[port];
        match->was_random = (uint8_t)((was_random[port] != 0) ? 1 : 0);
    }
}

void gmRemixTwelveCBRecordKo(GMRemixTwelveCBState *state, int32_t port, int32_t stocks_left)
{
    GMRemixTwelveCBPlayer *player;
    GMRemixTwelveCBMatch *match;

    if (gmRemixTwelveCBPortIsValid(port) == 0)
    {
        return;
    }
    player = &state->players[port];

    if (player->stocks_remaining <= 0)
    {
        return;
    }
    player->stocks_remaining--;

    if (player->stocks_remaining == 0)
    {
        state->status = GMREMIX_TWELVECB_STATUS_COMPLETE;
    }
    if (state->current_game < 0)
    {
        return;
    }
    match = &player->matches[state->current_game];
    match->stocks_end = (uint8_t)stocks_left;

    if (gmRemixTwelveCBPortraitIsValid(match->portrait) != 0)
    {
        state->stocks_by_portrait[match->portrait] = (int8_t)stocks_left;
    }
}

static int32_t gmRemixTwelveCBStocksLost(const GMRemixTwelveCBMatch *match)
{
    int32_t end_count = (match->stocks_end == GMREMIX_TWELVECB_NO_STOCKS) ? 0 : (match->stocks_end + 1);

    return (match->stocks_start + 1) - end_count;
}

static int32_t gmRemixTwelveCBRunKey(const GMRemixTwelveCBMatch *match)
{
    return ((int32_t)match->fkind << GMREMIX_TWELVECB_KEY_FKIND_SHIFT) + match->portrait;
}

static GMRemixTwelveCBRun* gmRemixTwelveCBCurrentRun(GMRemixTwelveCBRuns *runs, int32_t key)
{
    GMRemixTwelveCBRun *run;

    if ((runs->count > 0) && (runs->runs[runs->count - 1].key == key))
    {
        return &runs->runs[runs->count - 1];
    }
    run = &runs->runs[runs->count++];
    run->key = key;
    runs->own_lost = 0;
    runs->opponent_lost = 0;
    return run;
}

static void gmRemixTwelveCBAddGame(GMRemixTwelveCBRuns *runs, const GMRemixTwelveCBMatch *own,
                                   const GMRemixTwelveCBMatch *opponent)
{
    GMRemixTwelveCBRun *run = gmRemixTwelveCBCurrentRun(runs, gmRemixTwelveCBRunKey(own));

    runs->own_lost += gmRemixTwelveCBStocksLost(own);
    runs->opponent_lost += gmRemixTwelveCBStocksLost(opponent);
    run->tkos_for = runs->opponent_lost;
    run->tkos_against = runs->own_lost;
}

static void gmRemixTwelveCBPickBest(const GMRemixTwelveCBRuns *runs, GMRemixTwelveCBPlayer *player)
{
    int32_t i;

    player->best_fkind = GMREMIX_TWELVECB_NO_FKIND;
    player->best_tkos_for = -1;
    player->best_tkos_against = 0;

    for (i = 0; i < runs->count; i++)
    {
        const GMRemixTwelveCBRun *run = &runs->runs[i];
        int32_t is_better = (run->tkos_for > player->best_tkos_for) ||
                            ((run->tkos_for == player->best_tkos_for) && (run->tkos_against < player->best_tkos_against));

        if (is_better != 0)
        {
            player->best_fkind = run->key >> GMREMIX_TWELVECB_KEY_FKIND_SHIFT;
            player->best_tkos_for = run->tkos_for;
            player->best_tkos_against = run->tkos_against;
        }
    }
}

void gmRemixTwelveCBCalculateBest(GMRemixTwelveCBState *state)
{
    GMRemixTwelveCBRuns runs[GMREMIX_TWELVECB_PLAYERS];
    int32_t game;
    int32_t port;

    if (state->status == GMREMIX_TWELVECB_STATUS_NOT_STARTED)
    {
        return;
    }
    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        runs[port].count = 0;
    }
    for (game = 0; game <= state->current_game; game++)
    {
        gmRemixTwelveCBAddGame(&runs[0], &state->players[0].matches[game], &state->players[1].matches[game]);
        gmRemixTwelveCBAddGame(&runs[1], &state->players[1].matches[game], &state->players[0].matches[game]);
    }
    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        gmRemixTwelveCBPickBest(&runs[port], &state->players[port]);
    }
}

int32_t gmRemixTwelveCBGetWinnerPortraitOf(const GMRemixTwelveCBState *state, int32_t port)
{
    const GMRemixTwelveCBMatch *last;

    if ((port < 0) || (port >= GMREMIX_TWELVECB_PLAYERS) || (state->status != GMREMIX_TWELVECB_STATUS_STARTED) ||
        (state->current_game < 0) || (state->current_game >= GMREMIX_TWELVECB_GAMES))
    {
        return -1;
    }
    last = &state->players[port].matches[state->current_game];

    return (last->stocks_end != GMREMIX_TWELVECB_NO_STOCKS) ? (int32_t)last->portrait : -1;
}

void gmRemixTwelveCBUndoGame(GMRemixTwelveCBState *state)
{
    int32_t port;
    int32_t game = state->current_game;

    if (game < 0)
    {
        return;
    }
    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        GMRemixTwelveCBMatch *match = &state->players[port].matches[game];

        state->players[port].stocks_remaining += gmRemixTwelveCBStocksLost(match);

        if (gmRemixTwelveCBPortraitIsValid(match->portrait) != 0)
        {
            state->stocks_by_portrait[match->portrait] = (int8_t)match->stocks_start;
        }
        match->fkind = GMREMIX_TWELVECB_NO_STOCKS;
        match->stocks_start = 0;
        match->stocks_end = 0;
    }
    if (game == 0)
    {
        state->status = GMREMIX_TWELVECB_STATUS_NOT_STARTED;
    }
    state->current_game = game - 1;
}

int32_t gmRemixTwelveCBResetControllerPort(int32_t pausing_port, int32_t opponent_is_human)
{
    return (opponent_is_human != 0) ? (pausing_port ^ 1) : pausing_port;
}

char gmRemixTwelveCBResetRequesterDigit(int32_t port)
{
    return (char)('1' + port);
}

#endif
