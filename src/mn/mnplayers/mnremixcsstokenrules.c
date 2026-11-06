/**
 * mnremixcsstokenrules.c - pure rules of the Remix character select tokens, see mnremixcsstokenrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcsstokenrules.h"

#define MNREMIX_CSS_TOKEN_START_X 29
#define MNREMIX_CSS_TOKEN_START_Y 34
#define MNREMIX_CSS_TOKEN_CELL_SIZE 24
#define MNREMIX_CSS_TOKEN_REST_OFFSET_X 12
#define MNREMIX_CSS_TOKEN_REST_OFFSET_Y 14
#define MNREMIX_CSS_TOKEN_EDGE_OFFSET 12
#define MNREMIX_CSS_TOKEN_BONUS_X 21.0F
#define MNREMIX_CSS_TOKEN_RANDOM_X 278.0F
#define MNREMIX_CSS_TOKEN_BOOKEND_Y 70.0F
#define MNREMIX_CSS_TOKEN_BONUS_EDGE_X 24.0F
#define MNREMIX_CSS_TOKEN_RANDOM_EDGE_X 279.0F
#define MNREMIX_CSS_TOKEN_BOOKEND_WIDTH 15.0F
#define MNREMIX_CSS_TOKEN_BOOKEND_HEIGHT 72.0F

static int mnRemixCssTokenNormalizeSlot(int slot)
{
    if ((slot < 0) || (slot > MNREMIX_CSS_TOKEN_RANDOM_SLOT))
    {
        return 0;
    }
    return slot;
}

void mnRemixCssTokenRestPosition(int slot, float *pos_x, float *pos_y)
{
    slot = mnRemixCssTokenNormalizeSlot(slot);

    if (slot == MNREMIX_CSS_TOKEN_BONUS_SLOT)
    {
        *pos_x = MNREMIX_CSS_TOKEN_BONUS_X;
        *pos_y = MNREMIX_CSS_TOKEN_BOOKEND_Y;
        return;
    }
    if (slot == MNREMIX_CSS_TOKEN_RANDOM_SLOT)
    {
        *pos_x = MNREMIX_CSS_TOKEN_RANDOM_X;
        *pos_y = MNREMIX_CSS_TOKEN_BOOKEND_Y;
        return;
    }
    *pos_x = (float)(MNREMIX_CSS_TOKEN_START_X + MNREMIX_CSS_TOKEN_REST_OFFSET_X + (MNREMIX_CSS_TOKEN_CELL_SIZE * (slot % MNREMIX_CSS_TOKEN_GRID_COLUMNS)));
    *pos_y = (float)(MNREMIX_CSS_TOKEN_START_Y + MNREMIX_CSS_TOKEN_REST_OFFSET_Y + (MNREMIX_CSS_TOKEN_CELL_SIZE * (slot / MNREMIX_CSS_TOKEN_GRID_COLUMNS)));
}

void mnRemixCssTokenEdgeBox(int slot, float *edge_x, float *edge_y, float *width, float *height)
{
    slot = mnRemixCssTokenNormalizeSlot(slot);

    if (slot >= MNREMIX_CSS_TOKEN_BONUS_SLOT)
    {
        *edge_x = (slot == MNREMIX_CSS_TOKEN_BONUS_SLOT) ? MNREMIX_CSS_TOKEN_BONUS_EDGE_X : MNREMIX_CSS_TOKEN_RANDOM_EDGE_X;
        *edge_y = (float)(MNREMIX_CSS_TOKEN_START_Y + MNREMIX_CSS_TOKEN_EDGE_OFFSET);
        *width = MNREMIX_CSS_TOKEN_BOOKEND_WIDTH;
        *height = MNREMIX_CSS_TOKEN_BOOKEND_HEIGHT;
        return;
    }
    *edge_x = (float)(MNREMIX_CSS_TOKEN_START_X + MNREMIX_CSS_TOKEN_EDGE_OFFSET + (MNREMIX_CSS_TOKEN_CELL_SIZE * (slot % MNREMIX_CSS_TOKEN_GRID_COLUMNS)));
    *edge_y = (float)(MNREMIX_CSS_TOKEN_START_Y + MNREMIX_CSS_TOKEN_EDGE_OFFSET + (MNREMIX_CSS_TOKEN_CELL_SIZE * (slot / MNREMIX_CSS_TOKEN_GRID_COLUMNS)));
    *width = (float)MNREMIX_CSS_TOKEN_CELL_SIZE;
    *height = (float)MNREMIX_CSS_TOKEN_CELL_SIZE;
}

#endif
