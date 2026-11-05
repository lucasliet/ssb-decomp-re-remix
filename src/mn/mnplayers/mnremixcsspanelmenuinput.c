/**
 * mnremixcsspanelmenuinput.c - presses of the Remix CSS panel menu, see mnremixcsspanelmenu.h.
 *
 * CharacterSelectDebugMenu.asm: press_a_handler_ 1272 (order of the checks), handle_debug_button_press_ 1574,
 * handle_reset_button_press_ 1621, handle_item_arrow_press_ 1761, handle_page_arrow_press_ 1949, hold_a_handler_ 1432.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/audio.h>
#include "gm/gmremixmode.h"
#include "gm/gmremixstamina.h"
#include "gm/gmremixtoggle.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixcsspanelmenustate.h"
#include "mn/mnvsmode/mnremixvsmode.h"
#include "remix/remix_player_panel.h"

#include <port_log.h>

/* FGM.asm menu.TOGGLE: the sound of the button, the reset button and every arrow. */
#define MNREMIX_PANELMENU_FGM_TOGGLE 163
#define MNREMIX_PANELMENU_BUTTON_SIZE 16
#define MNREMIX_PANELMENU_ARROW_SIZE 13
#define MNREMIX_PANELMENU_DIRECTION_LEFT (-1)
#define MNREMIX_PANELMENU_DIRECTION_RIGHT 1

/** Plays the toggle sound unless it played in the last frames, so a held A does not play it every frame (fgm_buffer). */
static void mnRemixPanelMenuPlayRepeatSound(MNRemixPanelMenuState *state)
{
    if (state->sound_buffer != 0)
    {
        return;
    }
    state->sound_buffer = MNREMIX_PANELMENU_SOUND_BUFFER;
    syAudioPlayFGM(MNREMIX_PANELMENU_FGM_TOGGLE);
}

static sb32 mnRemixPanelMenuIsPressing(f32 cursor_x, f32 cursor_y, f32 x, f32 y, s32 size, s32 left_pad, s32 right_pad)
{
    return (mnRemixCssCursorPressesImage(cursor_x, cursor_y, x - (f32)left_pad, y, size + left_pad + right_pad, size) != 0) ? TRUE : FALSE;
}

sb32 mnRemixPanelMenuIsPanelActive(const MNRemixPanelMenuPanel *panel)
{
    return ((panel->info.is_present != FALSE) && (panel->is_open != FALSE)) ? TRUE : FALSE;
}

sb32 mnRemixPanelMenuAreHighScoresEnabled(void)
{
    return mnRemixPanelMenuGetState()->are_high_scores_enabled;
}

/** TRUE in the VS modes where the Stock option keeps its value (StockMode.asm onchange_handler). */
static sb32 mnRemixPanelMenuDoesModeKeepStock(void)
{
    return ((gmRemixModeIs(MNREMIX_VSMODE_DEFAULT) != FALSE) || (gmRemixModeIs(MNREMIX_VSMODE_TWELVE_CB) != FALSE) ||
            (gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR) != FALSE) || (gmRemixModeIs(MNREMIX_VSMODE_TAG_TEAM) != FALSE)) ? TRUE : FALSE;
}

/** Applies what the asm onchange handlers do to the new value that the registry can show: Stock and Damage in Stamina. */
static s32 mnRemixPanelMenuApplyChangeRules(const MNRemixPanelMenuState *state, s32 item, s32 value, s32 direction)
{
    if (state->mode != MNREMIX_PANELMENU_MODE_VS)
    {
        return value;
    }
    if (item == MNREMIX_PANELMENU_ITEM_STOCK)
    {
        return mnRemixPanelMenuRulesStockValue(mnRemixPanelMenuDoesModeKeepStock(), value);
    }
    if ((item == MNREMIX_PANELMENU_ITEM_DAMAGE) && (gGMRemixStaminaRule != FALSE))
    {
        return mnRemixPanelMenuRulesStaminaDamage(value, direction, gmRemixStaminaGetTotalHP());
    }
    return value;
}

/** Value after an arrow: wraps over the range, and a Player Tag with no name is passed over (handle_item_arrow_press_ _skip_value). */
static s32 mnRemixPanelMenuStepItem(s32 item, s32 port, s32 direction)
{
    s32 min = mnRemixPanelMenuRulesItemMin(item);
    s32 max = mnRemixPanelMenuRulesItemMax(item);
    s32 value = mnRemixPanelMenuRulesStepValue(mnRemixPanelMenuGetValue(item, port), direction, min, max);
    s32 attempts;

    if (item != MNREMIX_PANELMENU_ITEM_PLAYER_TAG)
    {
        return value;
    }
    for (attempts = 0; attempts <= max; attempts++)
    {
        if ((value == 0) || (remix_panel_tag_name(value)[0] != '\0'))
        {
            return value;
        }
        value = mnRemixPanelMenuRulesStepValue(value, direction, min, max);
    }
    return 0;
}

static void mnRemixPanelMenuChangeItem(MNRemixPanelMenuState *state, const MNRemixPanelMenuPanel *panel, s32 row, s32 direction)
{
    s32 item = panel->items[(panel->page * MNREMIX_PANELMENU_ROWS) + row];
    s32 value = mnRemixPanelMenuStepItem(item, panel->info.port, direction);

    value = mnRemixPanelMenuApplyChangeRules(state, item, value, direction);
    mnRemixPanelMenuSetValue(item, panel->info.port, value);
    port_log("css panel menu: port %d item %d = %d\n", (int)panel->info.port, (int)item, (int)value);
    mnRemixPanelMenuPlayRepeatSound(state);
}

/** Direction of the arrow of a row that the cursor presses: -1, 1, or 0. The left one is tested first, as the asm does. */
static s32 mnRemixPanelMenuGetItemArrowPressed(const MNRemixPanelMenuPanel *panel, s32 row, f32 cursor_x, f32 cursor_y)
{
    f32 y = panel->info.y + mnRemixPanelMenuRulesRowArrowY(row);
    f32 left_x = mnRemixPanelMenuRulesItemArrowX(panel->info.x, panel->info.width, 0);
    f32 right_x = mnRemixPanelMenuRulesItemArrowX(panel->info.x, panel->info.width, 1);

    if (mnRemixPanelMenuIsPressing(cursor_x, cursor_y, left_x, y, MNREMIX_PANELMENU_ARROW_SIZE, 0, MNREMIX_PANELMENU_ITEM_ARROW_LEFT_PAD) != FALSE)
    {
        return MNREMIX_PANELMENU_DIRECTION_LEFT;
    }
    if (mnRemixPanelMenuIsPressing(cursor_x, cursor_y, right_x, y, MNREMIX_PANELMENU_ARROW_SIZE, MNREMIX_PANELMENU_ITEM_ARROW_LEFT_PAD, 0) != FALSE)
    {
        return MNREMIX_PANELMENU_DIRECTION_RIGHT;
    }
    return 0;
}

static sb32 mnRemixPanelMenuPressItemArrows(MNRemixPanelMenuState *state, const MNRemixPanelMenuPanel *panel, f32 cursor_x, f32 cursor_y)
{
    s32 rows = mnRemixPanelMenuRulesRowsOnPage(panel->page, panel->item_count);
    s32 row;

    for (row = 0; row < rows; row++)
    {
        s32 direction = mnRemixPanelMenuGetItemArrowPressed(panel, row, cursor_x, cursor_y);

        if (direction != 0)
        {
            mnRemixPanelMenuChangeItem(state, panel, row, direction);
            return TRUE;
        }
    }
    return FALSE;
}

/** Direction of the page arrow that the cursor presses: -1, 1, or 0 (the left one has 3 units of padding on its right, the right one 4 on its left). */
static s32 mnRemixPanelMenuGetPageArrowPressed(const MNRemixPanelMenuPanel *panel, f32 cursor_x, f32 cursor_y)
{
    f32 y = panel->info.y + MNREMIX_PANELMENU_PAGE_ARROW_Y;
    f32 left_x = mnRemixPanelMenuRulesPageArrowX(panel->info.x, panel->info.width, 0);
    f32 right_x = mnRemixPanelMenuRulesPageArrowX(panel->info.x, panel->info.width, 1);

    if (mnRemixPanelMenuIsPressing(cursor_x, cursor_y, left_x, y, MNREMIX_PANELMENU_ARROW_SIZE, 0, MNREMIX_PANELMENU_PAGE_ARROW_LEFT_PAD) != FALSE)
    {
        return MNREMIX_PANELMENU_DIRECTION_LEFT;
    }
    if (mnRemixPanelMenuIsPressing(cursor_x, cursor_y, right_x, y, MNREMIX_PANELMENU_ARROW_SIZE, MNREMIX_PANELMENU_PAGE_ARROW_RIGHT_PAD, 0) != FALSE)
    {
        return MNREMIX_PANELMENU_DIRECTION_RIGHT;
    }
    return 0;
}

static sb32 mnRemixPanelMenuPressPageArrows(MNRemixPanelMenuState *state, MNRemixPanelMenuPanel *panel, MNRemixPanelMenuHold *hold,
                                            f32 cursor_x, f32 cursor_y)
{
    s32 max_page = mnRemixPanelMenuGetMaxPage(panel);
    s32 direction;

    if (max_page == 0)
    {
        return FALSE;
    }
    direction = mnRemixPanelMenuGetPageArrowPressed(panel, cursor_x, cursor_y);

    if (direction == 0)
    {
        return FALSE;
    }
    if (hold != NULL)
    {
        mnRemixPanelMenuRulesHoldRestart(hold);
    }
    panel->page = mnRemixPanelMenuRulesStepPage(panel->page, direction, max_page);
    mnRemixPanelMenuPlayRepeatSound(state);
    return TRUE;
}

/** Item arrows and page arrows of every open panel (_arrow_checks of press_a_handler_). */
static sb32 mnRemixPanelMenuPressArrows(MNRemixPanelMenuState *state, MNRemixPanelMenuHold *hold, f32 cursor_x, f32 cursor_y)
{
    s32 index;

    for (index = 0; index < MNREMIX_PANELMENU_PANELS; index++)
    {
        if ((mnRemixPanelMenuIsPanelActive(&state->panels[index]) != FALSE) &&
            (mnRemixPanelMenuPressItemArrows(state, &state->panels[index], cursor_x, cursor_y) != FALSE))
        {
            return TRUE;
        }
    }
    for (index = 0; index < MNREMIX_PANELMENU_PANELS; index++)
    {
        if ((mnRemixPanelMenuIsPanelActive(&state->panels[index]) != FALSE) &&
            (mnRemixPanelMenuPressPageArrows(state, &state->panels[index], hold, cursor_x, cursor_y) != FALSE))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** Opens or closes the menu of a panel when the cursor presses its button. */
static sb32 mnRemixPanelMenuPressButton(MNRemixPanelMenuPanel *panel, f32 cursor_x, f32 cursor_y)
{
    if ((panel->info.is_present == FALSE) || (panel->info.is_enabled == FALSE))
    {
        return FALSE;
    }
    if (mnRemixPanelMenuIsPressing(cursor_x, cursor_y, panel->info.x + MNREMIX_PANELMENU_BUTTON_X, MNREMIX_PANELMENU_BUTTON_Y,
                                   MNREMIX_PANELMENU_BUTTON_SIZE, 0, 0) == FALSE)
    {
        return FALSE;
    }
    panel->is_open = (panel->is_open == FALSE) ? TRUE : FALSE;
    port_log("css panel menu: port %d %s, card x=%d y=%d width=%d\n", (int)panel->info.port, (panel->is_open != FALSE) ? "open" : "closed",
             (int)panel->info.x, (int)panel->info.y, (int)panel->info.width);
    syAudioPlayFGM(MNREMIX_PANELMENU_FGM_TOGGLE);
    return TRUE;
}

/** Sets every item of the mode to its default for the port of the panel (handle_reset_button_press_). */
static void mnRemixPanelMenuResetPanel(MNRemixPanelMenuState *state, const MNRemixPanelMenuPanel *panel)
{
    s32 items[MNREMIX_PANELMENU_ITEM_COUNT];
    s32 count = mnRemixPanelMenuRulesListResetItems(state->mode, items, MNREMIX_PANELMENU_ITEM_COUNT);
    s32 index;

    for (index = 0; index < count; index++)
    {
        mnRemixPanelMenuSetValue(items[index], panel->info.port, mnRemixPanelMenuRulesItemDefault(items[index]));
    }
    syAudioPlayFGM(MNREMIX_PANELMENU_FGM_TOGGLE);
}

static sb32 mnRemixPanelMenuPressReset(MNRemixPanelMenuState *state, const MNRemixPanelMenuPanel *panel, f32 cursor_x, f32 cursor_y)
{
    if (mnRemixPanelMenuIsPanelActive(panel) == FALSE)
    {
        return FALSE;
    }
    if (mnRemixPanelMenuIsPressing(cursor_x, cursor_y, panel->info.x + MNREMIX_PANELMENU_RESET_X, MNREMIX_PANELMENU_BUTTON_Y,
                                   MNREMIX_PANELMENU_BUTTON_SIZE, 0, 0) == FALSE)
    {
        return FALSE;
    }
    mnRemixPanelMenuResetPanel(state, panel);
    port_log("css panel menu: port %d reset\n", (int)panel->info.port);
    return TRUE;
}

sb32 mnRemixPanelMenuPress(f32 cursor_x, f32 cursor_y)
{
    MNRemixPanelMenuState *state = mnRemixPanelMenuGetState();
    s32 index;

    if (gmRemixToggle(css_panel_menu) == 0)
    {
        return FALSE;
    }
    for (index = 0; index < MNREMIX_PANELMENU_PANELS; index++)
    {
        if (mnRemixPanelMenuPressButton(&state->panels[index], cursor_x, cursor_y) != FALSE)
        {
            return TRUE;
        }
    }
    for (index = 0; index < MNREMIX_PANELMENU_PANELS; index++)
    {
        if (mnRemixPanelMenuPressReset(state, &state->panels[index], cursor_x, cursor_y) != FALSE)
        {
            return TRUE;
        }
    }
    return mnRemixPanelMenuPressArrows(state, NULL, cursor_x, cursor_y);
}

void mnRemixPanelMenuHoldPress(s32 cursor_port, sb32 is_a_held, f32 cursor_x, f32 cursor_y)
{
    MNRemixPanelMenuState *state = mnRemixPanelMenuGetState();
    MNRemixPanelMenuHold *hold;

    if ((cursor_port < 0) || (cursor_port >= MNREMIX_PANELMENU_PANELS))
    {
        return;
    }
    hold = &state->holds[cursor_port];

    if (mnRemixPanelMenuRulesHoldAdvance(hold, (is_a_held != FALSE) ? 1 : 0) == 0)
    {
        return;
    }
    mnRemixPanelMenuPressArrows(state, hold, cursor_x, cursor_y);
}

#endif
