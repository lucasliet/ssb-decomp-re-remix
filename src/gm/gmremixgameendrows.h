#ifndef _GMREMIXGAMEENDROWS_H_
#define _GMREMIXGAMEENDROWS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * TRUE when the results screen of the current match gets the extra row: King of the Hill (time on the hill) or Smashketball
 * (baskets). VsRemixMenu.asm add_row_to_results_ checks vs_mode_flag for the same two modes.
 *
 * @return TRUE for King of the Hill and Smashketball matches.
 */
extern sb32 gmRemixGameEndRowsHasRow(void);

/**
 * Value of the extra row for a port: frames on the hill for King of the Hill (KingOfTheHill.scores), baskets for Smashketball
 * (Smashketball.scores). Read the rows through gmremixgameendrowsrules.h.
 *
 * @param port Port index (0..3).
 * @return The raw value, 0 when the match has no extra row.
 */
extern s32 gmRemixGameEndRowsGetValue(s32 port);

/**
 * Draws the extra results row (label and one value per present port) with the Remix HUD text. The label is text, the asm uses a
 * sprite of the Remix results file (offsets 0xE648 "Times", 0xE9E0 "FGs") that the port does not load.
 *
 * @param y Top of the row in N64 screen units (124).
 * @param column_x Centre x of each port column, as mnVSResultsGetColumnX returns it.
 * @param is_present TRUE for each port that took part.
 */
extern void gmRemixGameEndRowsDraw(f32 y, const f32 *column_x, const sb32 *is_present);

#endif

#endif
