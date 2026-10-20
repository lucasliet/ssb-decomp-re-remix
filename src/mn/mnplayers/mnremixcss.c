/**
 * mnremixcss.c - Smash Remix character select grid shared by the VS, 1P, training and bonus screens
 * (src/CharacterSelect.asm, data from port/remix/remix_css_data.h): 3 rows x 10 columns of 24x24 cells holding the
 * Remix character ids, which equal port fkinds.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/ftdef.h>
#include "remix/remix_css_data.h"
#include "mn/mnplayers/mnremixcss.h"
#include "mn/mnplayers/mnremixvariants.h"

#define MNREMIX_CSS_GRID_LEFT (REMIX_CSS_START_VISUAL + REMIX_CSS_START_X)
#define MNREMIX_CSS_GRID_TOP (REMIX_CSS_START_VISUAL + REMIX_CSS_START_Y)
#define MNREMIX_CSS_GRID_WIDTH (REMIX_CSS_PORTRAIT_WIDTH * REMIX_CSS_NUM_COLUMNS)
#define MNREMIX_CSS_GRID_HEIGHT (REMIX_CSS_PORTRAIT_HEIGHT * REMIX_CSS_NUM_ROWS)

f32 mnRemixCssCellX(s32 slot)
{
    return (f32)(MNREMIX_CSS_GRID_LEFT + (REMIX_CSS_PORTRAIT_WIDTH * (slot % REMIX_CSS_NUM_COLUMNS)));
}

f32 mnRemixCssCellY(s32 slot)
{
    return (f32)(MNREMIX_CSS_GRID_TOP + (REMIX_CSS_PORTRAIT_HEIGHT * (slot / REMIX_CSS_NUM_COLUMNS)));
}

/** The slot whose cell contains (pos_x, pos_y), or -1 outside the grid. */
s32 mnRemixCssSlotAtPoint(s32 pos_x, s32 pos_y)
{
    s32 col = pos_x - MNREMIX_CSS_GRID_LEFT;
    s32 row = pos_y - MNREMIX_CSS_GRID_TOP;

    if ((col < 0) || (col >= MNREMIX_CSS_GRID_WIDTH) || (row < 0) || (row >= MNREMIX_CSS_GRID_HEIGHT))
    {
        return -1;
    }
    return ((row / REMIX_CSS_PORTRAIT_HEIGHT) * REMIX_CSS_NUM_COLUMNS) + (col / REMIX_CSS_PORTRAIT_WIDTH);
}

s32 mnRemixCssGetFighterKind(s32 slot)
{
    if ((slot < 0) || (slot >= REMIX_CSS_NUM_SLOTS))
    {
        return nFTKindNull;
    }
    return kRemixCssSlotIds[slot];
}

/** The cell of `fkind`; a D-pad variant maps to the cell of the fighter it varies, any other fkind to slot 0. */
s32 mnRemixCssGetPortrait(s32 fkind)
{
    s32 slot, variant_slot;

    for (slot = 0; slot < REMIX_CSS_NUM_SLOTS; slot++)
    {
        if (kRemixCssSlotIds[slot] == fkind)
        {
            return slot;
        }
    }
    for (slot = 0; slot < REMIX_CSS_NUM_SLOTS; slot++)
    {
        for (variant_slot = 1; variant_slot <= MNREMIX_VARIANT_SLOT_COUNT; variant_slot++)
        {
            if (mnRemixVariantLookup(kRemixCssSlotIds[slot], variant_slot) == fkind)
            {
                return slot;
            }
        }
    }
    return 0;
}

#endif
