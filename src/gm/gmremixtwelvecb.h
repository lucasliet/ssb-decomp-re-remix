#ifndef _GMREMIXTWELVECB_H_
#define _GMREMIXTWELVECB_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>
#include "gm/gmremixtwelvecbrules.h"

/*
 * 12-Character Battle (TwelveCharBattle.asm, ROADMAP T6.1f) on the match: every character has stocks that last across the
 * games of the battle, a winner keeps what is left and the battle ends when the stock pool of a player is empty. The rules
 * live in gmremixtwelvecbrules.c; this file ties them to the VS match.
 */

/** TRUE when the current VS match is a 12-Character Battle. */
extern sb32 gmRemixTwelveCBIsActive(void);

/**
 * Starts a game of the battle at the start of the VS match, before the fighters are made: takes the portraits of both
 * players from their characters, records the game and works out the stocks each fighter starts with. A finished battle
 * starts over. Does nothing outside the mode.
 */
extern void gmRemixTwelveCBBeginMatch(void);

/**
 * Stocks (0-based) a fighter starts the game with, valid after gmRemixTwelveCBBeginMatch.
 *
 * @param player Port index of the fighter.
 * @param match_stocks Stock count of the match rules, returned for a port that is not in the battle.
 */
extern s32 gmRemixTwelveCBGetStocks(s32 player, s32 match_stocks);

/**
 * Takes the stock a fighter just lost out of its pool (ftCommonDeadUpdateScore, after the stock was taken).
 *
 * @param fp Fighter that lost a stock.
 */
extern void gmRemixTwelveCBOnKo(FTStruct *fp);

/**
 * Whether one CSS slot lets the 12-Character Battle start: only the ports 1 and 2 play, a character with no stocks left
 * cannot be taken into another game and a player whose pool is empty (the battle is over) blocks the start until RESET
 * (treat_selected_defeated_chars_as_unselected_). Always TRUE outside the mode.
 *
 * @param slot Port index of the CSS slot (0..3).
 * @param pkind Player kind of the slot (nFTPlayerKind*).
 * @param fkind Fighter the slot selected.
 */
extern sb32 gmRemixTwelveCBCheckSlot(s32 slot, s32 pkind, s32 fkind);

/**
 * Whether the token of a port can be picked up on the CSS (prevent_token_pickup_): once the battle started, the token of a CPU that
 * ended the last game with stocks left stays where it is. Always TRUE outside the mode.
 *
 * @param port Port index of the token (0 or 1).
 * @param is_human TRUE when the player of the token is a human.
 */
extern sb32 gmRemixTwelveCBTokenIsFree(s32 port, sb32 is_human);

/**
 * Remembers the CSS portrait slot (0..23) a player's token sits on; the game takes its portrait stocks from it. -1 forgets it,
 * and the portrait is then worked out from the fighter (the default character set).
 *
 * @param port Player port (0 or 1).
 * @param slot Portrait slot, or -1.
 */
extern void gmRemixTwelveCBSetCssPortrait(s32 port, s32 slot);

/** Character set (0 Default, 1 Japanese, 2 Polygon, 3 Remix) a player shows in his half of the CSS, and its setter. */
extern s32 gmRemixTwelveCBGetCssSet(s32 port);
extern void gmRemixTwelveCBSetCssSet(s32 port, s32 set);

/**
 * Applies a stock count change of the CSS selector (update_stock_fields_): every portrait and both pools restart from it.
 *
 * @param stocks New stock count, 0-based (0..98).
 * @return FALSE when the battle already started (the count is locked, the CSS plays the denied sound), else TRUE.
 */
extern sb32 gmRemixTwelveCBTrySetStocks(s32 stocks);

/**
 * Portrait slot a player must keep for the next game: the portrait he won the last game with (CharacterSelect.asm 1155-1228,
 * TwelveCharBattle.asm 2860-2929). A winner keeps his character until it loses a stock.
 *
 * @param port Player port (0 or 1).
 * @return The slot, or -1 when the player is free to choose (no game played, he lost it, or the battle is over).
 */
extern s32 gmRemixTwelveCBGetWinnerPortrait(s32 port);

/** TRUE when the stock pool of a player is empty, which ends the battle. */
extern sb32 gmRemixTwelveCBIsComplete(void);

/**
 * The battle in progress, with the best character of each player worked out, for the CSS and results text.
 *
 * @return The state (a fresh battle before the first game), or NULL when the mode is not active.
 */
extern const GMRemixTwelveCBState* gmRemixTwelveCBGetState(void);

/**
 * Whether the Manual stock mode of the CSS lets a player change the stocks of a portrait: the panel is in Manual, the battle is not over
 * and the player has not played the portrait yet (TwelveCharBattle.asm check_input_ 4098-4125).
 *
 * @param port Player port (0 or 1).
 * @param slot Portrait slot (0..23).
 */
extern sb32 gmRemixTwelveCBCanEditStocks(s32 port, s32 slot);

/**
 * Moves the stocks of a portrait one step (CharacterSelect.asm update_remaining_stocks_).
 *
 * @param port Player port (0 or 1) whose pool follows.
 * @param slot Portrait slot (0..23).
 * @param delta -1 or +1.
 */
extern void gmRemixTwelveCBStepStocks(s32 port, s32 slot, s32 delta);

/** Takes back the game in progress when both players accepted a Gentlemen's Reset of the pause (gmRemixTwelveCBUndoGame). */
extern void gmRemixTwelveCBUndoCurrentGame(void);

/** Forgets the battle (the CSS goes back to the VS menu). */
extern void gmRemixTwelveCBReset(void);

#endif

#endif
