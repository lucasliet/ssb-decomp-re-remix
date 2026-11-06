#ifndef _GMREMIXGAMEENDWAITRULES_H_
#define _GMREMIXGAMEENDWAITRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Tics before a VS results screen allows leaving it (mnVSResultsInitVars), pure so the headless unit tests (tests/remix)
 * can run it.
 */

#define GMREMIX_GAMEEND_WAIT_STOCK 370
#define GMREMIX_GAMEEND_WAIT_TIME 410

/**
 * Exit wait of a results screen of a stock match (TimedStock.asm show_full_results_): with Always Show Full Results on, the
 * asm jumps into the Time branch of mnVSResultsInitVars, which also sets the Time wait.
 *
 * @param is_full_results Non-zero when Toggles.entry_full_results is on.
 * @return GMREMIX_GAMEEND_WAIT_TIME with full results on, otherwise GMREMIX_GAMEEND_WAIT_STOCK.
 */
extern int gmRemixGameEndRulesStockExitWait(int is_full_results);

#endif

#endif
