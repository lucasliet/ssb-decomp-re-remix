#ifndef _GMREMIXGAMEENDROWSRULES_H_
#define _GMREMIXGAMEENDROWSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Numbers of the extra results rows of King of the Hill and Smashketball (VsRemixMenu.asm add_row_to_results_), pure so the
 * headless unit tests (tests/remix) can run them. Values are the asm constants; the score sources are in gmremixgameendrows.h.
 */

#define GMREMIX_GAMEEND_ROWS_FRAMES_PER_SECOND 60
#define GMREMIX_GAMEEND_ROWS_SECONDS_PER_MINUTE 60
#define GMREMIX_GAMEEND_ROWS_COUNT_MAX 999

/* Results row of the mode, drawn at y 124 (no contest: 96). */
#define GMREMIX_GAMEEND_ROWS_NEW_ROW_Y 124
#define GMREMIX_GAMEEND_ROWS_NO_CONTEST_ROW_Y 96
/* The place line moves down by this much when the extra row is shown (VsRemixMenu.asm _move_place). */
#define GMREMIX_GAMEEND_ROWS_PLACE_PUSH_Y 16
/* The place line is drawn this many tics after the new row (VsRemixMenu.asm _delay_place, addiu at, at, 20). */
#define GMREMIX_GAMEEND_ROWS_PLACE_DELAY_TICS 20
/* Tic of the new row: the vanilla place line tic of the time layouts (_delay_place), 100 on no contest (_no_contest_new_row). */
#define GMREMIX_GAMEEND_ROWS_NEW_ROW_TICS 290
#define GMREMIX_GAMEEND_ROWS_NO_CONTEST_NEW_ROW_TICS 100
#define GMREMIX_GAMEEND_ROWS_NO_ROW_TICS (-1)

/* Results layouts as the extra row sees them: only the time routines and the no contest routine are patched. */
#define GMREMIX_GAMEEND_ROWS_LAYOUT_TIME 0
#define GMREMIX_GAMEEND_ROWS_LAYOUT_STOCK 1
#define GMREMIX_GAMEEND_ROWS_LAYOUT_NO_CONTEST 2

/* Image offsets of the row labels in the results file (VsRemixMenu.asm _draw_alt_texture): "Times" and "FGs". */
#define GMREMIX_GAMEEND_ROWS_KOTH_LABEL_OFFSET 0xE648
#define GMREMIX_GAMEEND_ROWS_SMASHKETBALL_LABEL_OFFSET 0xE9E0

/**
 * Whole seconds of a King of the Hill score (VsRemixMenu.asm _draw_times, divu by 60).
 *
 * @param frames Frames on the hill.
 * @return Seconds on the hill, frames / 60.
 */
extern int gmRemixGameEndRowsSeconds(int frames);

/**
 * Seconds shown after the minutes, 0 to 59 (VsRemixMenu.asm _draw_times, mfhi of divu by 60).
 *
 * @param frames Frames on the hill.
 * @return (frames / 60) % 60.
 */
extern int gmRemixGameEndRowsSecondsDigits(int frames);

/**
 * Minutes shown before the colon (VsRemixMenu.asm _draw_times, second divu by 60).
 *
 * @param frames Frames on the hill.
 * @return Minutes on the hill, frames / 3600.
 */
extern int gmRemixGameEndRowsMinutes(int frames);

/**
 * Value drawn in the goals row, capped like the KOs column (VsRemixMenu.asm _draw_goals, slti 0x3E8).
 *
 * @param value Raw count of baskets.
 * @return value, or GMREMIX_GAMEEND_ROWS_COUNT_MAX when value is above it.
 */
extern int gmRemixGameEndRowsClampCount(int value);

/**
 * Height of the place line: pushed down when the extra row is shown (VsRemixMenu.asm _move_place).
 *
 * @param place_y Vanilla y of the place line (124).
 * @param has_row TRUE for King of the Hill and Smashketball.
 * @return place_y, plus GMREMIX_GAMEEND_ROWS_PLACE_PUSH_Y when has_row.
 */
extern int gmRemixGameEndRowsPlaceY(int place_y, int has_row);

/**
 * TRUE when the tens digit of the seconds is drawn even when it is 0 (VsRemixMenu.asm _show_seconds_tens_digit_, KOTH time row).
 *
 * @param is_koth_time_row TRUE while the King of the Hill time row is drawn.
 * @return is_koth_time_row as a boolean.
 */
extern int gmRemixGameEndRowsForcesTensDigit(int is_koth_time_row);

/**
 * Tic at which the extra row is made (VsRemixMenu.asm _delay_place and _no_contest_new_row; the stock routines are not patched).
 *
 * @param layout GMREMIX_GAMEEND_ROWS_LAYOUT_*.
 * @return GMREMIX_GAMEEND_ROWS_NEW_ROW_TICS, GMREMIX_GAMEEND_ROWS_NO_CONTEST_NEW_ROW_TICS, or GMREMIX_GAMEEND_ROWS_NO_ROW_TICS
 * for the stock layouts.
 */
extern int gmRemixGameEndRowsRowTics(int layout);

/**
 * Height of the extra row (lli a0, 124 in _delay_place, lli a0, 96 in _no_contest_new_row).
 *
 * @param layout GMREMIX_GAMEEND_ROWS_LAYOUT_*.
 * @return GMREMIX_GAMEEND_ROWS_NO_CONTEST_ROW_Y on no contest, otherwise GMREMIX_GAMEEND_ROWS_NEW_ROW_Y.
 */
extern int gmRemixGameEndRowsRowY(int layout);

#endif

#endif
