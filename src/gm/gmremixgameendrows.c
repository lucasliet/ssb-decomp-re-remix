/**
 * gmremixgameendrows.c - reads the King of the Hill and Smashketball scores for the extra results row, see gmremixgameendrows.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixgameendrows.h"
#include "gm/gmremixgameendrowsrules.h"
#include "remix/remix_hud_text.h"
#include "gm/gmremixkoth.h"
#include "gm/gmremixmode.h"
#include "gm/gmremixsmashketballstate.h"
#include "mn/mnvsmode/mnremixvsmode.h"

sb32 gmRemixGameEndRowsHasRow(void)
{
    return (gmRemixModeIs(MNREMIX_VSMODE_KOTH) || gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL)) ? TRUE : FALSE;
}

s32 gmRemixGameEndRowsGetValue(s32 port)
{
    if (gmRemixModeIs(MNREMIX_VSMODE_KOTH))
    {
        return gmRemixKothGetScore(port);
    }
    if (gmRemixModeIs(MNREMIX_VSMODE_SMASHKETBALL))
    {
        return gGMRemixSmashketball.scores[port];
    }
    return 0;
}

#define GMREMIX_GAMEEND_ROWS_TEXT_SLOT_FIRST 20
#define GMREMIX_GAMEEND_ROWS_TEXT_SLOT_LABEL 30
#define GMREMIX_GAMEEND_ROWS_PORTS 4
#define GMREMIX_GAMEEND_ROWS_LABEL_X 26.0F
#define GMREMIX_GAMEEND_ROWS_TEXT_SCALE 0.75F
#define GMREMIX_GAMEEND_ROWS_TEXT_COLOR 0xFFFFFFFF

void gmRemixGameEndRowsDraw(f32 y, const f32 *column_x, const sb32 *is_present)
{
    sb32 is_koth = gmRemixModeIs(MNREMIX_VSMODE_KOTH) ? TRUE : FALSE;
    s32 port;

    /* REMIX-TODO(GameEnd): VsRemixMenu.asm:1106-1232 draws the "Times"/"FGs" label images and the result number sprites here,
     * not Kabel text. */
    remix_hud_text_put(GMREMIX_GAMEEND_ROWS_TEXT_SLOT_LABEL, (is_koth != FALSE) ? "Time" : "Goals", GMREMIX_GAMEEND_ROWS_LABEL_X, y,
                       GMREMIX_GAMEEND_ROWS_TEXT_COLOR, GMREMIX_GAMEEND_ROWS_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);

    for (port = 0; port < GMREMIX_GAMEEND_ROWS_PORTS; port++)
    {
        s32 value = gmRemixGameEndRowsGetValue(port);
        s32 slot = GMREMIX_GAMEEND_ROWS_TEXT_SLOT_FIRST + port;

        if (is_present[port] == FALSE)
        {
            continue;
        }
        if (is_koth != FALSE)
        {
            remix_hud_text_putf(slot, column_x[port], y, GMREMIX_GAMEEND_ROWS_TEXT_COLOR, GMREMIX_GAMEEND_ROWS_TEXT_SCALE,
                                REMIX_HUD_ALIGN_CENTER, "%d:%02d", gmRemixGameEndRowsMinutes(value),
                                gmRemixGameEndRowsSecondsDigits(value));
            continue;
        }
        remix_hud_text_putf(slot, column_x[port], y, GMREMIX_GAMEEND_ROWS_TEXT_COLOR, GMREMIX_GAMEEND_ROWS_TEXT_SCALE,
                            REMIX_HUD_ALIGN_CENTER, "%d", gmRemixGameEndRowsClampCount(value));
    }
}

#endif
