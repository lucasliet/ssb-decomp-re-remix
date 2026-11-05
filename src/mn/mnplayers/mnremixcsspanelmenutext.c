/**
 * mnremixcsspanelmenutext.c - labels, values and page number of the Remix CSS panel menu, see mnremixcsspanelmenu.h.
 *
 * The asm draws them with Render.draw_string_ (create_item_label_, create_item_value_, render_pagination_); the port puts them
 * through remix_hud_text every frame.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcsspanelmenustate.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_player_panel.h"

#include <stdio.h>

/* The menu items are numbered like the registry options (the asm add_menu_item order). */
_Static_assert((int)REMIX_PANEL_SHIELD == (int)MNREMIX_PANELMENU_ITEM_SHIELD, "panel menu item order");
_Static_assert((int)REMIX_PANEL_STOCK_MODE == (int)MNREMIX_PANELMENU_ITEM_STOCK, "panel menu item order");
_Static_assert((int)REMIX_PANEL_HANDICAP == (int)MNREMIX_PANELMENU_ITEM_HANDICAP, "panel menu item order");
_Static_assert((int)REMIX_PANEL_DPAD_CONTROL == (int)MNREMIX_PANELMENU_ITEM_DPAD_CONTROL, "panel menu item order");
_Static_assert((int)REMIX_PANEL_DAMAGE == (int)MNREMIX_PANELMENU_ITEM_DAMAGE, "panel menu item order");
_Static_assert((int)REMIX_PANEL_PRACTICE_1P == (int)MNREMIX_PANELMENU_ITEM_PRACTICE, "panel menu item order");

/* Text slots of the CSS scenes: 7 per panel (3 labels, 3 values, the page number). The 12-Character Battle CSS uses 32..48 and
 * 100..103 and the 1P CSS enemy label 120..121, so 56..83 is free in every CSS. */
#define MNREMIX_PANELMENU_TEXT_SLOT_FIRST 56
#define MNREMIX_PANELMENU_TEXT_SLOTS_PER_PANEL 7
#define MNREMIX_PANELMENU_VALUE_SLOT_OFFSET 3
#define MNREMIX_PANELMENU_PAGE_SLOT_OFFSET 6
#define MNREMIX_PANELMENU_TEXT_PAGE_ROW (-1)

s32 mnRemixPanelMenuGetTextSlot(s32 panel_index, s32 row, sb32 is_value)
{
    s32 base = MNREMIX_PANELMENU_TEXT_SLOT_FIRST + (panel_index * MNREMIX_PANELMENU_TEXT_SLOTS_PER_PANEL);

    if (row == MNREMIX_PANELMENU_TEXT_PAGE_ROW)
    {
        return base + MNREMIX_PANELMENU_PAGE_SLOT_OFFSET;
    }
    return base + row + ((is_value != FALSE) ? MNREMIX_PANELMENU_VALUE_SLOT_OFFSET : 0);
}

s32 mnRemixPanelMenuGetValue(s32 item, s32 port)
{
    return remix_panel_get(item, port);
}

void mnRemixPanelMenuSetValue(s32 item, s32 port, s32 value)
{
    remix_panel_set(item, port, value);
}

/** Text of Input Delay (css/InputDelay.asm string_table): "None", "1 frame", "N frames". */
static void mnRemixPanelMenuGetDelayText(s32 value, char *text, s32 text_size)
{
    if (value == 0)
    {
        snprintf(text, text_size, "None");
        return;
    }
    snprintf(text, text_size, (value == 1) ? "%d frame" : "%d frames", (int)value);
}

/** Text of Player Tag (css/PlayerTag.asm string_table): "Default", then the name typed for the tag. */
static void mnRemixPanelMenuGetTagText(s32 value, char *text, s32 text_size)
{
    if (value == 0)
    {
        snprintf(text, text_size, "Default");
        return;
    }
    snprintf(text, text_size, "%s", remix_panel_tag_name(value));
}

void mnRemixPanelMenuGetValueText(s32 item, s32 port, char *text, s32 text_size)
{
    s32 value = mnRemixPanelMenuGetValue(item, port);
    const char *name;

    if (mnRemixPanelMenuRulesItemIsNumeric(item) != 0)
    {
        snprintf(text, text_size, "%d", (int)value);
        return;
    }
    if (item == MNREMIX_PANELMENU_ITEM_DELAY)
    {
        mnRemixPanelMenuGetDelayText(value, text, text_size);
        return;
    }
    if (item == MNREMIX_PANELMENU_ITEM_PLAYER_TAG)
    {
        mnRemixPanelMenuGetTagText(value, text, text_size);
        return;
    }
    name = remix_panel_value_name(item, value);
    snprintf(text, text_size, "%s", (name != NULL) ? name : "");
}

static void mnRemixPanelMenuDrawRow(s32 panel_index, const MNRemixPanelMenuPanel *panel, s32 row)
{
    char value_text[REMIX_HUD_TEXT_LENGTH];
    s32 item = panel->items[(panel->page * MNREMIX_PANELMENU_ROWS) + row];
    f32 center_x = panel->info.x + ((f32)panel->info.width * 0.5F);

    mnRemixPanelMenuGetValueText(item, panel->info.port, value_text, sizeof(value_text));
    remix_hud_text_put_fit(mnRemixPanelMenuGetTextSlot(panel_index, row, FALSE), mnRemixPanelMenuRulesItemLabel(item),
                           panel->info.x + MNREMIX_PANELMENU_LABEL_X, panel->info.y + mnRemixPanelMenuRulesRowTextY(row, 0),
                           MNREMIX_PANELMENU_LABEL_RGBA, MNREMIX_PANELMENU_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT,
                           (f32)(panel->info.width - MNREMIX_PANELMENU_LABEL_INSET));
    remix_hud_text_put_fit(mnRemixPanelMenuGetTextSlot(panel_index, row, TRUE), value_text, center_x,
                           panel->info.y + mnRemixPanelMenuRulesRowTextY(row, 1), MNREMIX_PANELMENU_VALUE_RGBA,
                           MNREMIX_PANELMENU_TEXT_SCALE, REMIX_HUD_ALIGN_CENTER, (f32)(panel->info.width - MNREMIX_PANELMENU_VALUE_INSET));
}

void mnRemixPanelMenuHideText(s32 panel_index)
{
    s32 row;

    for (row = 0; row < MNREMIX_PANELMENU_ROWS; row++)
    {
        remix_hud_text_hide(mnRemixPanelMenuGetTextSlot(panel_index, row, FALSE));
        remix_hud_text_hide(mnRemixPanelMenuGetTextSlot(panel_index, row, TRUE));
    }
    remix_hud_text_hide(mnRemixPanelMenuGetTextSlot(panel_index, MNREMIX_PANELMENU_TEXT_PAGE_ROW, FALSE));
}

void mnRemixPanelMenuDrawText(s32 panel_index, const MNRemixPanelMenuPanel *panel)
{
    s32 rows = mnRemixPanelMenuRulesRowsOnPage(panel->page, panel->item_count);
    s32 row;

    mnRemixPanelMenuHideText(panel_index);

    for (row = 0; row < rows; row++)
    {
        mnRemixPanelMenuDrawRow(panel_index, panel, row);
    }
    if (mnRemixPanelMenuGetMaxPage(panel) == 0)
    {
        return;
    }
    remix_hud_text_putf(mnRemixPanelMenuGetTextSlot(panel_index, MNREMIX_PANELMENU_TEXT_PAGE_ROW, FALSE),
                        panel->info.x + (f32)panel->info.width - MNREMIX_PANELMENU_PAGE_NUMBER_INSET,
                        panel->info.y + MNREMIX_PANELMENU_PAGE_NUMBER_Y, MNREMIX_PANELMENU_VALUE_RGBA, MNREMIX_PANELMENU_TEXT_SCALE,
                        REMIX_HUD_ALIGN_CENTER, "%d", (int)(panel->page + 1));
}

#endif
