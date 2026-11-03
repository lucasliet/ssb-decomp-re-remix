/**
 * mnremixtwelvecblayout.c - pure layout of the 12-Character Battle character select, see mnremixtwelvecblayout.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixtwelvecblayout.h"

/* Fighter kinds of a set: three rows of the four characters a half shows (TwelveCharBattle.asm layout.slot_N, 81-197). */
static const int sLayoutRows[MNREMIX_TCB_SETS][MNREMIX_TCB_ROWS][MNREMIX_TCB_HALF_COLUMNS] =
{
    { { 0x04, 0x00, 0x02, 0x05 }, { 0x03, 0x07, 0x0B, 0x06 }, { 0x08, 0x01, 0x09, 0x0A } }, /* Default: Luigi, Mario, DK, Link ... */
    { { 0x2B, 0x2A, 0x2C, 0x27 }, { 0x24, 0x28, 0x25, 0x31 }, { 0x30, 0x29, 0x32, 0x2E } }, /* Japanese */
    { { 0x12, 0x0E, 0x10, 0x13 }, { 0x11, 0x15, 0x19, 0x14 }, { 0x16, 0x0F, 0x17, 0x18 } }, /* Polygon */
    { { 0x20, 0x1E, 0x1F, 0x1D }, { 0x22, 0x21, 0x26, 0x34 }, { 0x37, 0x38, 0x39, 0x3A } }, /* Remix */
};

/* TwelveCharBattle.asm portrait_x_position: START_VISUAL 10 + START_X 30 + 30 per column, 8 left of it for the left half and 8
 * right of it for the right half. */
static const float sLayoutCellX[MNREMIX_TCB_COLUMNS] = { 32.0F, 62.0F, 92.0F, 122.0F, 168.0F, 198.0F, 228.0F, 258.0F };
static const float sLayoutRowY[MNREMIX_TCB_ROWS] = { 35.0F, 65.0F, 95.0F };
/* TwelveCharBattle.asm 231-239: slide speed of each column, negative for the right half. */
static const float sLayoutSlideSpeed[MNREMIX_TCB_COLUMNS] = { 1.9F, 3.9F, 7.8F, 11.8F, -11.8F, -7.8F, -3.8F, -1.8F };
#define LAYOUT_LEFT_START_X (-20.0F)
#define LAYOUT_RIGHT_START_X 310.0F

/* Hit test extents (TwelveCharBattle.asm 1023-1118): the first row starts at y 25, each half is 4 cells of 30 wide. */
#define LAYOUT_HIT_TOP 25
#define LAYOUT_HIT_BOTTOM 110
#define LAYOUT_HIT_LEFT_X 19
#define LAYOUT_HIT_RIGHT_X 155

static int slotIsValid(int slot)
{
    return (slot >= 0) && (slot < MNREMIX_TCB_SLOTS);
}

int mnRemixTwelveCBLayoutFkind(int set, int slot)
{
    if ((set < 0) || (set >= MNREMIX_TCB_SETS) || !slotIsValid(slot))
    {
        return -1;
    }
    return sLayoutRows[set][slot / MNREMIX_TCB_COLUMNS][slot % MNREMIX_TCB_HALF_COLUMNS];
}

float mnRemixTwelveCBLayoutCellX(int slot)
{
    return slotIsValid(slot) ? sLayoutCellX[slot % MNREMIX_TCB_COLUMNS] : 0.0F;
}

float mnRemixTwelveCBLayoutCellY(int slot)
{
    return slotIsValid(slot) ? sLayoutRowY[slot / MNREMIX_TCB_COLUMNS] : 0.0F;
}

float mnRemixTwelveCBLayoutStartX(int slot)
{
    return ((slot % MNREMIX_TCB_COLUMNS) < MNREMIX_TCB_HALF_COLUMNS) ? LAYOUT_LEFT_START_X : LAYOUT_RIGHT_START_X;
}

float mnRemixTwelveCBLayoutNextX(int slot, float current_x)
{
    float target = mnRemixTwelveCBLayoutCellX(slot);
    float speed = sLayoutSlideSpeed[slot % MNREMIX_TCB_COLUMNS];
    float next;

    if (current_x == target)
    {
        return target;
    }
    next = current_x + speed;
    if (((speed > 0.0F) && (next >= target)) || ((speed < 0.0F) && (next <= target)))
    {
        return target;
    }
    return next;
}

int mnRemixTwelveCBLayoutSlotAtPoint(int player, int point_x, int point_y)
{
    int origin_x = (player == 0) ? LAYOUT_HIT_LEFT_X : LAYOUT_HIT_RIGHT_X;
    int row;
    int column;

    if ((player < 0) || (player > 1) || (point_y < LAYOUT_HIT_TOP) || (point_y >= LAYOUT_HIT_BOTTOM) || (point_x < origin_x))
    {
        return MNREMIX_TCB_NO_SLOT;
    }
    column = (point_x - origin_x) / MNREMIX_TCB_CELL_SIZE;
    row = (point_y - LAYOUT_HIT_TOP) / MNREMIX_TCB_CELL_SIZE;
    if ((column >= MNREMIX_TCB_HALF_COLUMNS) || (row >= MNREMIX_TCB_ROWS))
    {
        return MNREMIX_TCB_NO_SLOT;
    }
    return (row * MNREMIX_TCB_COLUMNS) + column + (player * MNREMIX_TCB_HALF_COLUMNS);
}

int mnRemixTwelveCBLayoutOwner(int slot)
{
    if (!slotIsValid(slot))
    {
        return -1;
    }
    return ((slot % MNREMIX_TCB_COLUMNS) < MNREMIX_TCB_HALF_COLUMNS) ? 0 : 1;
}

int mnRemixTwelveCBLayoutSlotOfFkind(int set, int player, int fkind)
{
    int row;
    int column;

    for (row = 0; row < MNREMIX_TCB_ROWS; row++)
    {
        for (column = 0; column < MNREMIX_TCB_HALF_COLUMNS; column++)
        {
            if (mnRemixTwelveCBLayoutFkind(set, (row * MNREMIX_TCB_COLUMNS) + column) == fkind)
            {
                return (row * MNREMIX_TCB_COLUMNS) + column + (player * MNREMIX_TCB_HALF_COLUMNS);
            }
        }
    }
    return MNREMIX_TCB_NO_SLOT;
}

#endif
