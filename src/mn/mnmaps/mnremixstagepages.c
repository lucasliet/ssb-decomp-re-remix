/**
 * mnremixstagepages.c - pages of the Remix stage select grid (src/Stages.asm stage_table, stage_table_pages, stage_table_normal,
 * stage_table_tournament), read from the Remix image.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "gr/grremix/grremixids.h"
#include "gr/grremix/grremixstage.h"
#include "mn/mnmaps/mnremixstagepages.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <port_log.h>

#define MNREMIX_STAGE_PAGES_COUNT_WORD_SIZE 4

/** Pages of a layout (Stages.stage_table_pages), kept inside the grid arrays. */
static s32 mnRemixStagePagesReadCount(sb32 is_tournament)
{
    u32 count_addr = REMIX_SYM_Stages__stage_table_pages + ((is_tournament != FALSE) ? MNREMIX_STAGE_PAGES_COUNT_WORD_SIZE : 0);
    s32 count = (s32)remix_ram_image_read_word(count_addr);

    if (count < 1)
    {
        return 1;
    }
    return (count > MNREMIX_GRID_PAGES_MAX) ? MNREMIX_GRID_PAGES_MAX : count;
}

static s32 mnRemixStagePagesReadCell(u32 table_addr, s32 index)
{
    s32 gkind = (s32)remix_ram_image_read_byte(table_addr + index);

    if ((gkind == nGRRemixStage_RANDOM) || (gkind <= GRREMIX_KIND_MAX))
    {
        return gkind;
    }
    return MNREMIX_STAGE_PAGES_EMPTY;
}

s32 mnRemixStagePagesLoad(sb32 is_tournament, s32 out_gkinds[MNREMIX_GRID_PAGES_MAX][MNREMIX_GRID_CELLS])
{
    u32 table_addr = (is_tournament != FALSE) ? REMIX_SYM_Stages__stage_table_tournament : REMIX_SYM_Stages__stage_table_normal;
    s32 page_count = mnRemixStagePagesReadCount(is_tournament);
    s32 page;
    s32 cell;

    for (page = 0; page < page_count; page++)
    {
        for (cell = 0; cell < MNREMIX_GRID_CELLS; cell++)
        {
            out_gkinds[page][cell] = mnRemixStagePagesReadCell(table_addr, mnRemixStageGridRulesGetTableIndex(page, cell));
        }
    }
    port_log("[mnmaps] remix stage grid: %d pages, tournament %d\n", (int)page_count, (int)is_tournament);

    return page_count;
}

#endif
