/**
 * mnremixstagegridrules.c - pure rules of the Remix stage select grid, see mnremixstagegridrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnmaps/mnremixstagegridrules.h"

#define MNREMIX_GRID_ICON_ORIGIN_X 30
#define MNREMIX_GRID_ICON_ORIGIN_Y 20
#define MNREMIX_GRID_CURSOR_ORIGIN_X 0x18
#define MNREMIX_GRID_CURSOR_ORIGIN_Y 0x0E

int mnRemixStageGridRulesGetCell(int row, int column)
{
    return (row * MNREMIX_GRID_COLUMNS) + column;
}

int mnRemixStageGridRulesGetRow(int cell)
{
    return cell / MNREMIX_GRID_COLUMNS;
}

int mnRemixStageGridRulesGetColumn(int cell)
{
    return cell % MNREMIX_GRID_COLUMNS;
}

int mnRemixStageGridRulesMoveRight(int cell)
{
    int row = mnRemixStageGridRulesGetRow(cell);
    int column = mnRemixStageGridRulesGetColumn(cell);

    return mnRemixStageGridRulesGetCell(row, (column < (MNREMIX_GRID_COLUMNS - 1)) ? (column + 1) : 0);
}

int mnRemixStageGridRulesMoveLeft(int cell)
{
    int row = mnRemixStageGridRulesGetRow(cell);
    int column = mnRemixStageGridRulesGetColumn(cell);

    return mnRemixStageGridRulesGetCell(row, (column != 0) ? (column - 1) : (MNREMIX_GRID_COLUMNS - 1));
}

int mnRemixStageGridRulesMoveDown(int cell)
{
    int row = mnRemixStageGridRulesGetRow(cell);
    int column = mnRemixStageGridRulesGetColumn(cell);

    return mnRemixStageGridRulesGetCell((row < (MNREMIX_GRID_ROWS - 1)) ? (row + 1) : 0, column);
}

int mnRemixStageGridRulesMoveUp(int cell)
{
    int row = mnRemixStageGridRulesGetRow(cell);
    int column = mnRemixStageGridRulesGetColumn(cell);

    return mnRemixStageGridRulesGetCell((row != 0) ? (row - 1) : (MNREMIX_GRID_ROWS - 1), column);
}

int mnRemixStageGridRulesNextPage(int page, int page_count)
{
    return ((page + 1) == page_count) ? 0 : (page + 1);
}

int mnRemixStageGridRulesPreviousPage(int page, int page_count)
{
    return (page == 0) ? (page_count - 1) : (page - 1);
}

int mnRemixStageGridRulesClampPage(int page, int page_count)
{
    return (page_count > page) ? page : 0;
}

int mnRemixStageGridRulesGetTableIndex(int page, int cell)
{
    return (page * MNREMIX_GRID_CELLS) + cell;
}

int mnRemixStageGridRulesRouletteRerolls(int row, int column)
{
    return ((row == (MNREMIX_GRID_ROWS - 1)) && (column == (MNREMIX_GRID_COLUMNS - 1))) ? 1 : 0;
}

int mnRemixStageGridRulesGetIconX(int column)
{
    return MNREMIX_GRID_ICON_ORIGIN_X + (column * MNREMIX_GRID_STEP_X);
}

int mnRemixStageGridRulesGetIconY(int row)
{
    return MNREMIX_GRID_ICON_ORIGIN_Y + (row * MNREMIX_GRID_STEP_Y);
}

int mnRemixStageGridRulesGetCursorX(int column)
{
    return (MNREMIX_GRID_STEP_X * column) + MNREMIX_GRID_CURSOR_ORIGIN_X;
}

int mnRemixStageGridRulesGetCursorY(int row)
{
    return (MNREMIX_GRID_STEP_Y * row) + MNREMIX_GRID_CURSOR_ORIGIN_Y;
}

int mnRemixStageGridRulesGetPageNumber(int page)
{
    return page + 1;
}

#endif
