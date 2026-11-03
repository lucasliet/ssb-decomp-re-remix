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

#endif

#endif
