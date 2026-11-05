/**
 * gmremixgameendrowsrules.c - pure arithmetic of the extra results rows, see gmremixgameendrowsrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgameendrowsrules.h"

int gmRemixGameEndRowsSeconds(int frames)
{
    return frames / GMREMIX_GAMEEND_ROWS_FRAMES_PER_SECOND;
}

int gmRemixGameEndRowsSecondsDigits(int frames)
{
    return gmRemixGameEndRowsSeconds(frames) % GMREMIX_GAMEEND_ROWS_SECONDS_PER_MINUTE;
}

int gmRemixGameEndRowsMinutes(int frames)
{
    return gmRemixGameEndRowsSeconds(frames) / GMREMIX_GAMEEND_ROWS_SECONDS_PER_MINUTE;
}

int gmRemixGameEndRowsClampCount(int value)
{
    if (value > GMREMIX_GAMEEND_ROWS_COUNT_MAX)
    {
        return GMREMIX_GAMEEND_ROWS_COUNT_MAX;
    }
    return value;
}

int gmRemixGameEndRowsPlaceY(int place_y, int has_row)
{
    if (has_row)
    {
        return place_y + GMREMIX_GAMEEND_ROWS_PLACE_PUSH_Y;
    }
    return place_y;
}

int gmRemixGameEndRowsForcesTensDigit(int is_koth_time_row)
{
    return (is_koth_time_row != 0) ? 1 : 0;
}

int gmRemixGameEndRowsRowTics(int layout)
{
    if (layout == GMREMIX_GAMEEND_ROWS_LAYOUT_NO_CONTEST)
    {
        return GMREMIX_GAMEEND_ROWS_NO_CONTEST_NEW_ROW_TICS;
    }
    if (layout == GMREMIX_GAMEEND_ROWS_LAYOUT_STOCK)
    {
        return GMREMIX_GAMEEND_ROWS_NO_ROW_TICS;
    }
    return GMREMIX_GAMEEND_ROWS_NEW_ROW_TICS;
}

int gmRemixGameEndRowsRowY(int layout)
{
    if (layout == GMREMIX_GAMEEND_ROWS_LAYOUT_NO_CONTEST)
    {
        return GMREMIX_GAMEEND_ROWS_NO_CONTEST_ROW_Y;
    }
    return GMREMIX_GAMEEND_ROWS_NEW_ROW_Y;
}

#endif
