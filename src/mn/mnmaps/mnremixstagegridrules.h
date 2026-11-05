#ifndef _MNREMIXSTAGEGRIDRULES_H_
#define _MNREMIXSTAGEGRIDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix stage select grid (Stages.asm NUM_ROWS, NUM_COLUMNS, NUM_ICONS, NUM_PAGES, get_index_, check_valid_page_,
 * get_stage_id_, right_, left_, down_, up_, right_check_ page switch, random_stage_roulette), so the headless unit tests
 * (tests/remix) can run them. A cell is row * columns + column inside a page; a page holds MNREMIX_GRID_CELLS cells. */

/* Stages.NUM_COLUMNS, NUM_ROWS, NUM_ICONS and NUM_PAGES (the page count of the normal layout; the tournament layout has 3). */
#define MNREMIX_GRID_COLUMNS 6
#define MNREMIX_GRID_ROWS 3
#define MNREMIX_GRID_CELLS 18
#define MNREMIX_GRID_PAGES_MAX 5

/* Stages.ICON_WIDTH + 2 and ICON_HEIGHT + 2: the cursor step. The icon step is the width of the image (40 and 30) plus the padding 2. */
#define MNREMIX_GRID_STEP_X 42
#define MNREMIX_GRID_STEP_Y 32

/**
 * Cell of the cursor (get_index_).
 *
 * @param row Cursor row, 0 to MNREMIX_GRID_ROWS - 1.
 * @param column Cursor column, 0 to MNREMIX_GRID_COLUMNS - 1.
 * @return row * MNREMIX_GRID_COLUMNS + column.
 */
extern int mnRemixStageGridRulesGetCell(int row, int column);

/** Row of a cell. */
extern int mnRemixStageGridRulesGetRow(int cell);

/** Column of a cell. */
extern int mnRemixStageGridRulesGetColumn(int cell);

/** Cell after right_: the next column, the first column after the last (the row and the page stay). */
extern int mnRemixStageGridRulesMoveRight(int cell);

/** Cell after left_: the previous column, the last column before the first. */
extern int mnRemixStageGridRulesMoveLeft(int cell);

/** Cell after down_: the next row, the first row after the last. */
extern int mnRemixStageGridRulesMoveDown(int cell);

/** Cell after up_: the previous row, the last row before the first. */
extern int mnRemixStageGridRulesMoveUp(int cell);

/** Page after R (right_check_ _page_up): the next page, the first page after the last. */
extern int mnRemixStageGridRulesNextPage(int page, int page_count);

/** Page after Z (right_check_ _page_down): the previous page, the last page before the first. */
extern int mnRemixStageGridRulesPreviousPage(int page, int page_count);

/** check_valid_page_: a page that is not below the page count of the layout goes back to the first page. */
extern int mnRemixStageGridRulesClampPage(int page, int page_count);

/**
 * Index in the stage table of the hovered cell (get_stage_id_): the cell plus MNREMIX_GRID_CELLS per page.
 */
extern int mnRemixStageGridRulesGetTableIndex(int page, int cell);

/**
 * random_stage_roulette `randomize_coords`: the jump is drawn again when it lands on the bottom right cell (row 2, column 5),
 * the RANDOM cell of every page. It looks at the position only, not at the stage id.
 *
 * @return 1 when the roll must be repeated.
 */
extern int mnRemixStageGridRulesRouletteRerolls(int row, int column);

/** X of the icon of a column (Render.draw_texture_grid at x 30, step 40 + 2). */
extern int mnRemixStageGridRulesGetIconX(int column);

/** Y of the icon of a row (Render.draw_texture_grid at y 20, step 30 + 2). */
extern int mnRemixStageGridRulesGetIconY(int row);

/** X of the cursor sprite of a column (Stages.asm 0x80132A58: (ICON_WIDTH + 2) * column + 0x18). */
extern int mnRemixStageGridRulesGetCursorX(int column);

/** Y of the cursor sprite of a row (Stages.asm 0x80132A58: (ICON_HEIGHT + 2) * row + 0x0E). */
extern int mnRemixStageGridRulesGetCursorY(int row);

/** Number shown after "Page" (draw_number_adjusted adds 1 to the 0 based page). */
extern int mnRemixStageGridRulesGetPageNumber(int page);

#endif

#endif
