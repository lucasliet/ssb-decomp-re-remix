#ifndef _MNREMIXSTAGEPAGES_H_
#define _MNREMIXSTAGEPAGES_H_

#include <ssb_types.h>
#include "mn/mnmaps/mnremixstagegridrules.h"

#if defined(PORT) && defined(REGION_REMIX)

/* Value of a cell that holds no stage: a table byte that is neither a stage id nor the RANDOM id. */
#define MNREMIX_STAGE_PAGES_EMPTY 0xDD

/**
 * Reads the pages of the stage select grid from the Remix image: Stages.stage_table_normal, or Stages.stage_table_tournament on
 * the tournament layout, MNREMIX_GRID_CELLS bytes per page, with the page count of Stages.stage_table_pages. The RANDOM id stays
 * in the cells that hold it (it is a cell of the table, not a fixed slot).
 *
 * @param is_tournament TRUE for the tournament layout.
 * @param out_gkinds Receives the stage id of every cell of every page.
 * @return Number of pages, 1 to MNREMIX_GRID_PAGES_MAX.
 */
extern s32 mnRemixStagePagesLoad(sb32 is_tournament, s32 out_gkinds[MNREMIX_GRID_PAGES_MAX][MNREMIX_GRID_CELLS]);

#endif

#endif
