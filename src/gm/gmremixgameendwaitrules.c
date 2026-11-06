/**
 * gmremixgameendwaitrules.c - state-free exit wait of the VS results screen, see gmremixgameendwaitrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgameendwaitrules.h"

int gmRemixGameEndRulesStockExitWait(int is_full_results)
{
    return (is_full_results != 0) ? GMREMIX_GAMEEND_WAIT_TIME : GMREMIX_GAMEEND_WAIT_STOCK;
}

#endif
