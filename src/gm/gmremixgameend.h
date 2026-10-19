#ifndef _GMREMIXGAMEEND_H_
#define _GMREMIXGAMEEND_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Smash Remix end of a VS match (GameEnd.asm update_screen_, TimedStock.asm show_full_results_). Settings:
 * Toggles.entry_skip_results_screen, entry_salty_runback and entry_full_results. Preserve Song is read by the random
 * music pick (gr/grremix/grremixrandommusic.c).
 *
 * @return The scene that follows the battle: nSCKindVSBattle (salty runback), nSCKindPlayersVS (results skipped) or
 * nSCKindVSResults.
 */
s32 gmRemixGameEndSelectScene(void);

/**
 * GameEnd.is_salty_runback: TRUE from the salty runback restart until the next VS match ends.
 *
 * @return TRUE when the current match restarts a salty runback.
 */
sb32 gmRemixGameEndIsSaltyRunback(void);

/**
 * Maps the results kind to the draw procedure kind: Always Show Full Results draws the time layout (KOs and TKOs) for
 * the stock kinds, as the Remix branch of mnVSResultsMakeLabel does.
 *
 * @param kind sMNVSResultsKind (nMNVSResultsKind*).
 * @return The kind whose draw procedure is used.
 */
s32 gmRemixGameEndResultsDrawKind(s32 kind);

#endif

#endif
