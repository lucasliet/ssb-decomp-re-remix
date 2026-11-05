#ifndef _MNREMIXTWELVECBLAYOUT_H_
#define _MNREMIXTWELVECBLAYOUT_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Layout of the 12-Character Battle character select (TwelveCharBattle.asm lines 79-357, docs/remix/css_reference.md): 24
 * portraits in 3 rows of 8 columns. Player 1 owns the columns 0 to 3 and player 2 the columns 4 to 7, so every character shows up
 * twice and each player has 12 portraits of his own. Pure so the headless unit tests (tests/remix) run it.
 */

#define MNREMIX_TCB_SLOTS 24
#define MNREMIX_TCB_COLUMNS 8
#define MNREMIX_TCB_ROWS 3
#define MNREMIX_TCB_HALF_COLUMNS 4
#define MNREMIX_TCB_SETS 4
#define MNREMIX_TCB_NO_SLOT (-1)

/** Portrait size in screen units (a 32 pixel image drawn at scale 1.0 on a 30 unit step, so the cells overlap by 2). */
#define MNREMIX_TCB_CELL_SIZE 30
#define MNREMIX_TCB_PORTRAIT_SIZE 32

/** Fighter kind of a slot in a character set (0 Default, 1 Japanese, 2 Polygon, 3 Remix, 4 Custom); -1 for an invalid set or slot. */
extern int mnRemixTwelveCBLayoutFkind(int set, int slot);

/** Final X and Y of the portrait of a slot, in screen units (the asm portrait_x_position and _get_y). */
extern float mnRemixTwelveCBLayoutCellX(int slot);
extern float mnRemixTwelveCBLayoutCellY(int slot);

/** X where the portrait of a slot starts before it slides in: left of the screen for columns 0 to 3, right for 4 to 7. */
extern float mnRemixTwelveCBLayoutStartX(int slot);

/**
 * One step of the slide-in (the asm portrait update routine).
 *
 * @param slot Portrait slot.
 * @param current_x Current X of the portrait.
 * @return The next X, never past the final X; the final X itself when it is already there.
 */
extern float mnRemixTwelveCBLayoutNextX(int slot, float current_x);

/**
 * Slot a token points at (TwelveCharBattle.asm hit test, lines 1023-1118). Player 1 only reaches the left half and player 2
 * the right half.
 *
 * @param player 0 or 1.
 * @param point_x X of the token point in screen units.
 * @param point_y Y of the token point in screen units.
 * @return The slot, or MNREMIX_TCB_NO_SLOT when the point is off the player's portraits.
 */
extern int mnRemixTwelveCBLayoutSlotAtPoint(int player, int point_x, int point_y);

/** Slot of a player holding a fighter kind in a set, MNREMIX_TCB_NO_SLOT when the player's half has none. */
extern int mnRemixTwelveCBLayoutSlotOfFkind(int set, int player, int fkind);

/** Player that owns a slot: 0 for the columns 0 to 3, 1 for 4 to 7; -1 for an invalid slot. */
extern int mnRemixTwelveCBLayoutOwner(int slot);

/**
 * Padding between the right edge of the panel card and the right edge of the stock number (update_stock_icon_and_count_indicators_):
 * -4 normally, and when the Manual stock arrows sit at the card edge -10 for a one digit number and -7 for two digits.
 *
 * @param has_arrows Non-zero when the Manual stock arrows are shown.
 * @param stocks_shown Number the indicator shows (1-based).
 * @return Padding in screen units, negative.
 */
extern int mnRemixTwelveCBLayoutStockPadding(int has_arrows, int stocks_shown);

#endif

#endif
