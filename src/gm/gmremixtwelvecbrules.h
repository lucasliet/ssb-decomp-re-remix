#ifndef _GMREMIXTWELVECBRULES_H_
#define _GMREMIXTWELVECBRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <stdint.h>

/*
 * Rules of the 12-Character Battle (TwelveCharBattle.asm), kept free of the game state so they run in the headless unit
 * tests (tests/remix). Each player has a pool of 12 characters x (stocks + 1) stocks; every character sits in two of the 24
 * portrait slots. The asm keeps 24 game records per player, which a long battle can overrun (each game defeats one portrait
 * of the loser, so up to 47 games), so the records here hold 48. Stock counts are 0-based like the asm (3 means four stocks) and 0xFF in a match record means no stocks left.
 */

#define GMREMIX_TWELVECB_PLAYERS 2
#define GMREMIX_TWELVECB_CHARACTERS 12
#define GMREMIX_TWELVECB_PORTRAITS 24
#define GMREMIX_TWELVECB_GAMES 48
#define GMREMIX_TWELVECB_DEFAULT_STOCKS 3
#define GMREMIX_TWELVECB_NO_STOCKS 0xFF
#define GMREMIX_TWELVECB_NO_FKIND (-1)

typedef enum GMRemixTwelveCBStatus
{
    GMREMIX_TWELVECB_STATUS_NOT_STARTED,
    GMREMIX_TWELVECB_STATUS_STARTED,
    GMREMIX_TWELVECB_STATUS_COMPLETE
} GMRemixTwelveCBStatus;

typedef struct GMRemixTwelveCBMatch
{
    uint8_t fkind;
    uint8_t stocks_start;
    uint8_t stocks_end;
    uint8_t portrait;
    uint8_t was_random;
} GMRemixTwelveCBMatch;

typedef struct GMRemixTwelveCBPlayer
{
    int32_t stocks_remaining;
    GMRemixTwelveCBMatch matches[GMREMIX_TWELVECB_GAMES];
    int32_t best_fkind;
    int32_t best_tkos_for;
    int32_t best_tkos_against;
} GMRemixTwelveCBPlayer;

typedef struct GMRemixTwelveCBState
{
    int32_t status;
    int32_t num_stocks;
    int32_t current_game;
    int8_t stocks_by_portrait[GMREMIX_TWELVECB_PORTRAITS];
    GMRemixTwelveCBPlayer players[GMREMIX_TWELVECB_PLAYERS];
} GMRemixTwelveCBState;

/**
 * Puts a fresh battle in `state`: not started, no game played, four stocks for each of the 12 characters of both players.
 *
 * @param state Battle state to initialize.
 */
extern void gmRemixTwelveCBInit(GMRemixTwelveCBState *state);

/**
 * Handles the stock count of the CSS (update_stock_fields_): when it changed, every portrait and both pools restart from it.
 *
 * @param state Battle state.
 * @param stocks New stock count, 0-based (0..98).
 */
extern void gmRemixTwelveCBUpdateStocks(GMRemixTwelveCBState *state, int32_t stocks);

/**
 * Portrait slot of a character with no character select to place it: the 12 original fighters sit in the slots 0..11, any other
 * fighter in the slots 12..23 by its number. The 24 slot CSS (mnremixtwelvecb*.c) reports the real slot of each token through
 * gmRemixTwelveCBSetCssPortrait, and the match prefers the slot a fighter holds in the layout of his character set, so this
 * mapping is only the last fallback (a fighter that the layout of the set does not hold, where the asm returns slot 0).
 *
 * @param fkind Fighter kind of the character.
 */
extern int32_t gmRemixTwelveCBPortraitOf(int32_t fkind);

/**
 * Stocks left of the character in `portrait`, 1-based (get_stocks_remaining_for_char_); 0 means it is defeated.
 *
 * @param state Battle state.
 * @param portrait Portrait slot (0..23); an out of range slot counts as defeated.
 */
extern int32_t gmRemixTwelveCBGetPortraitStocks(const GMRemixTwelveCBState *state, int32_t portrait);

/**
 * Whether a player already played a portrait in a finished or running game (was_portrait_played_).
 *
 * @param state Battle state.
 * @param port Player port (0 or 1).
 * @param portrait Portrait slot (0..23).
 * @return 1 when one of the games up to the current one used that portrait, else 0.
 */
extern int32_t gmRemixTwelveCBWasPortraitPlayed(const GMRemixTwelveCBState *state, int32_t port, int32_t portrait);

/**
 * Moves the stocks of one portrait by one step, the way the Manual stock mode of the CSS does (update_remaining_stocks_ in the 12-Character
 * Battle): the count runs through no stocks (defeated) up to the CSS stock count and wraps at both ends, and the pool of the player
 * follows the change.
 *
 * @param state Battle state.
 * @param port Player port (0 or 1) whose pool changes.
 * @param portrait Portrait slot (0..23).
 * @param delta -1 or +1.
 * @return 1 when the portrait changed, 0 when the arguments are out of range.
 */
extern int32_t gmRemixTwelveCBStepPortraitStocks(GMRemixTwelveCBState *state, int32_t port, int32_t portrait, int32_t delta);

/**
 * Remembers the portrait a player chose for the next game (the CSS writes it before the match starts).
 *
 * @param state Battle state.
 * @param port Player port (0 or 1).
 * @param portrait Portrait slot (0..23).
 */
extern void gmRemixTwelveCBSetNextPortrait(GMRemixTwelveCBState *state, int32_t port, int32_t portrait);

/**
 * Portrait the winner of the last game must keep (the CSS only lets him pick it again): the portrait of the last game record when
 * the player still had stocks at its end. -1 when there is none to keep: no game yet, the battle is over, or he lost the game.
 *
 * @param state Battle state.
 * @param port Player port (0 or 1).
 */
extern int32_t gmRemixTwelveCBGetWinnerPortraitOf(const GMRemixTwelveCBState *state, int32_t port);

/**
 * Stock count (0-based) a player starts the next game with (set_initial_stock_count_): the stocks the player ended the last
 * game with when any are left to spare, otherwise the stocks of the portrait chosen.
 *
 * @param state Battle state.
 * @param port Player port (0 or 1).
 */
extern int32_t gmRemixTwelveCBGetInitialStocks(const GMRemixTwelveCBState *state, int32_t port);

/**
 * Starts a game (game_setup_): marks the battle started, moves to the next game and records what each player starts with.
 *
 * @param state Battle state.
 * @param fkinds Character of each player.
 * @param stocks Starting stocks of each player, 0-based.
 * @param was_random Non-zero for the player that chose Random.
 */
extern void gmRemixTwelveCBStartGame(GMRemixTwelveCBState *state, const int32_t fkinds[GMREMIX_TWELVECB_PLAYERS],
                                     const int32_t stocks[GMREMIX_TWELVECB_PLAYERS],
                                     const int32_t was_random[GMREMIX_TWELVECB_PLAYERS]);

/**
 * Takes one stock from a player after a KO (update_stocks_remaining_): the pool drops, the battle completes when the pool is
 * empty, and the game record and the portrait keep the stocks left.
 *
 * @param state Battle state.
 * @param port Player port that lost the stock (0 or 1).
 * @param stocks_left Stocks the player has left after the KO, 0-based (-1 when none).
 */
extern void gmRemixTwelveCBRecordKo(GMRemixTwelveCBState *state, int32_t port, int32_t stocks_left);

/**
 * Picks the best character of each player over the games played (set_best_characters_): the one that took the most stocks
 * from the opponent, the one that lost fewer when tied. Does nothing before the battle starts.
 *
 * @param state Battle state; the best_* fields of both players are written.
 */
extern void gmRemixTwelveCBCalculateBest(GMRemixTwelveCBState *state);

/**
 * Takes back the game in progress after a Gentlemen's Reset was accepted (gentlemens_reset_ _reset): every stock the players
 * lost this game goes back to their pools and to the portrait they played, the game record is cleared (the portrait stays) and
 * the battle goes back one game, to not started when it was the first. Does nothing before a game started.
 *
 * @param state Battle state.
 */
extern void gmRemixTwelveCBUndoGame(GMRemixTwelveCBState *state);

/**
 * Port that controls the pause after a reset request (gentlemens_reset_ 3689-3703): the opponent when it is a human, so that he
 * can accept or decline, otherwise the player that asked.
 *
 * @param pausing_port Port that controls the pause (0 or 1).
 * @param opponent_is_human Non-zero when the other player is a human.
 */
extern int32_t gmRemixTwelveCBResetControllerPort(int32_t pausing_port, int32_t opponent_is_human);

/**
 * Digit of the requesting player in "Game reset requested by  P." ('1' for port 0, '2' for port 1).
 *
 * @param port Port of the player that asked.
 */
extern char gmRemixTwelveCBResetRequesterDigit(int32_t port);

#endif

#endif
