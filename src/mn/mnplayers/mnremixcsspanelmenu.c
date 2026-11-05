/**
 * mnremixcsspanelmenu.c - per-port panel menu of the Remix character select screens, see mnremixcsspanelmenu.h.
 *
 * CharacterSelectDebugMenu.asm: init_debug_menu_ 27 and update_debug_menu_ 79 (the button and reset button sprites, one control
 * object that runs every frame), set_panel_display_ 317 (what the open menu hides), setup_menu_page_ 554, render_menu_page_ 644,
 * create_item_arrows_ 929 and render_pagination_ 1073 (arrows and blink). Text is in mnremixcsspanelmenutext.c, presses in
 * mnremixcsspanelmenuinput.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <string.h>
#include <reloc_data.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include "gm/gmremixtoggle.h"
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnremixcsshighscores.h"
#include "mn/mnplayers/mnremixcsspanelmenustate.h"

/* The links of the panel card of every CSS (the 12-Character Battle stock arrows use the same ones). The control object takes the
 * last GObj link, so its process runs after the ones of the CSS and what it hides stays hidden when the frame is drawn. */
#define MNREMIX_PANELMENU_GOBJ_LINK 22
#define MNREMIX_PANELMENU_DL_LINK 28
#define MNREMIX_PANELMENU_CONTROLLER_LINK 32
/* Remix CSS images file: the gold button with the Smash logo (footer 0x1208) and the reset to defaults button (0x1828). */
#define MNREMIX_PANELMENU_BUTTON_SPRITE 0x1208
#define MNREMIX_PANELMENU_RESET_SPRITE 0x1828

static MNRemixPanelMenuState sState;

MNRemixPanelMenuState *mnRemixPanelMenuGetState(void)
{
    return &sState;
}

s32 mnRemixPanelMenuGetMaxPage(const MNRemixPanelMenuPanel *panel)
{
    return mnRemixPanelMenuRulesMaxPage(panel->item_count);
}

sb32 mnRemixPanelMenuIsOpen(s32 index)
{
    if ((index < 0) || (index >= MNREMIX_PANELMENU_PANELS))
    {
        return FALSE;
    }
    return ((sState.panels[index].info.is_present != FALSE) && (sState.panels[index].is_open != FALSE)) ? TRUE : FALSE;
}

void mnRemixPanelMenuSetPanel(s32 index, const MNRemixPanelMenuPanelInfo *info)
{
    MNRemixPanelMenuPanel *panel;

    if ((index < 0) || (index >= MNREMIX_PANELMENU_PANELS))
    {
        return;
    }
    panel = &sState.panels[index];
    panel->info = *info;
    panel->item_count = mnRemixPanelMenuRulesListItems(sState.mode, info->is_cpu, panel->items, MNREMIX_PANELMENU_ITEM_COUNT);
    panel->page = mnRemixPanelMenuRulesClampPage(panel->page, mnRemixPanelMenuGetMaxPage(panel));
}

void mnRemixPanelMenuSetHideTargets(s32 index, GObj *const *targets, s32 count)
{
    MNRemixPanelMenuPanel *panel;
    s32 i;

    if ((index < 0) || (index >= MNREMIX_PANELMENU_PANELS))
    {
        return;
    }
    panel = &sState.panels[index];
    panel->hide_count = (count < MNREMIX_PANELMENU_HIDE_MAX) ? count : MNREMIX_PANELMENU_HIDE_MAX;

    for (i = 0; i < MNREMIX_PANELMENU_HIDE_MAX; i++)
    {
        panel->hide_targets[i] = (i < panel->hide_count) ? targets[i] : NULL;
    }
}

void mnRemixPanelMenuSetGateSprite(s32 index, SObj *sobj)
{
    if ((index >= 0) && (index < MNREMIX_PANELMENU_PANELS))
    {
        sState.panels[index].gate_sobj = sobj;
    }
}

static void mnRemixPanelMenuSetHidden(GObj *gobj, sb32 is_hidden)
{
    if (gobj != NULL)
    {
        gobj->flags = (is_hidden != FALSE) ? GOBJ_FLAG_HIDDEN : GOBJ_FLAG_NONE;
    }
}

static void mnRemixPanelMenuSetPosition(GObj *gobj, f32 x, f32 y)
{
    SObj *sobj;

    if (gobj == NULL)
    {
        return;
    }
    sobj = SObjGetStruct(gobj);
    sobj->pos.x = x;
    sobj->pos.y = y;
}

static GObj *mnRemixPanelMenuMakeButton(u32 sprite_offset, f32 x)
{
    return ifRemixSpriteMakeFromFile(sState.images_file, sprite_offset, MNREMIX_PANELMENU_GOBJ_LINK, MNREMIX_PANELMENU_DL_LINK, x,
                                     MNREMIX_PANELMENU_BUTTON_Y, 0, 0, MNREMIX_PANELMENU_BUTTON_SCALE);
}

static GObj *mnRemixPanelMenuMakeArrow(sb32 is_right)
{
    u32 sprite_offset = (is_right != FALSE) ? llMNPlayersCommonArrowRSprite : llMNPlayersCommonArrowLSprite;

    return ifRemixSpriteMakeFromFile(sState.arrows_file, sprite_offset, MNREMIX_PANELMENU_GOBJ_LINK, MNREMIX_PANELMENU_DL_LINK, 0.0F,
                                     0.0F, MNREMIX_PANELMENU_ARROW_TINT, MNREMIX_PANELMENU_ARROW_PALETTE, MNREMIX_PANELMENU_ARROW_SCALE);
}

/** Makes the sprites of a panel the first time it exists; the arrows start hidden. */
static void mnRemixPanelMenuMakeSprites(MNRemixPanelMenuPanel *panel)
{
    s32 row;

    if (panel->button != NULL)
    {
        return;
    }
    panel->button = mnRemixPanelMenuMakeButton(MNREMIX_PANELMENU_BUTTON_SPRITE, panel->info.x + MNREMIX_PANELMENU_BUTTON_X);
    panel->reset = mnRemixPanelMenuMakeButton(MNREMIX_PANELMENU_RESET_SPRITE, panel->info.x + MNREMIX_PANELMENU_RESET_X);
    mnRemixPanelMenuSetHidden(panel->reset, TRUE);

    for (row = 0; row < MNREMIX_PANELMENU_ROWS; row++)
    {
        panel->item_arrows[row][0] = mnRemixPanelMenuMakeArrow(FALSE);
        panel->item_arrows[row][1] = mnRemixPanelMenuMakeArrow(TRUE);
        mnRemixPanelMenuSetHidden(panel->item_arrows[row][0], TRUE);
        mnRemixPanelMenuSetHidden(panel->item_arrows[row][1], TRUE);
    }
    panel->page_arrows[0] = mnRemixPanelMenuMakeArrow(FALSE);
    panel->page_arrows[1] = mnRemixPanelMenuMakeArrow(TRUE);
    mnRemixPanelMenuSetHidden(panel->page_arrows[0], TRUE);
    mnRemixPanelMenuSetHidden(panel->page_arrows[1], TRUE);
}

static void mnRemixPanelMenuPlaceItemArrows(const MNRemixPanelMenuPanel *panel, sb32 is_blink_on)
{
    s32 rows = mnRemixPanelMenuRulesRowsOnPage(panel->page, panel->item_count);
    s32 row;

    for (row = 0; row < MNREMIX_PANELMENU_ROWS; row++)
    {
        sb32 is_shown = ((panel->is_open != FALSE) && (row < rows) && (is_blink_on != FALSE)) ? TRUE : FALSE;
        f32 y = panel->info.y + mnRemixPanelMenuRulesRowArrowY(row);

        mnRemixPanelMenuSetPosition(panel->item_arrows[row][0], mnRemixPanelMenuRulesItemArrowX(panel->info.x, panel->info.width, 0), y);
        mnRemixPanelMenuSetPosition(panel->item_arrows[row][1], mnRemixPanelMenuRulesItemArrowX(panel->info.x, panel->info.width, 1), y);
        mnRemixPanelMenuSetHidden(panel->item_arrows[row][0], !is_shown);
        mnRemixPanelMenuSetHidden(panel->item_arrows[row][1], !is_shown);
    }
}

static void mnRemixPanelMenuPlacePageArrows(const MNRemixPanelMenuPanel *panel, sb32 is_blink_on)
{
    sb32 is_shown = ((panel->is_open != FALSE) && (mnRemixPanelMenuGetMaxPage(panel) > 0) && (is_blink_on != FALSE)) ? TRUE : FALSE;
    f32 y = panel->info.y + MNREMIX_PANELMENU_PAGE_ARROW_Y;

    mnRemixPanelMenuSetPosition(panel->page_arrows[0], mnRemixPanelMenuRulesPageArrowX(panel->info.x, panel->info.width, 0), y);
    mnRemixPanelMenuSetPosition(panel->page_arrows[1], mnRemixPanelMenuRulesPageArrowX(panel->info.x, panel->info.width, 1), y);
    mnRemixPanelMenuSetHidden(panel->page_arrows[0], !is_shown);
    mnRemixPanelMenuSetHidden(panel->page_arrows[1], !is_shown);
}

static void mnRemixPanelMenuPlaceButtons(const MNRemixPanelMenuPanel *panel)
{
    sb32 is_button_shown = ((gmRemixToggle(css_panel_menu) != 0) && (panel->info.is_enabled != FALSE)) ? TRUE : FALSE;

    mnRemixPanelMenuSetPosition(panel->button, panel->info.x + MNREMIX_PANELMENU_BUTTON_X, MNREMIX_PANELMENU_BUTTON_Y);
    mnRemixPanelMenuSetPosition(panel->reset, panel->info.x + MNREMIX_PANELMENU_RESET_X, MNREMIX_PANELMENU_BUTTON_Y);
    mnRemixPanelMenuSetHidden(panel->button, !is_button_shown);
    mnRemixPanelMenuSetHidden(panel->reset, (panel->is_open == FALSE) ? TRUE : FALSE);
}

/**
 * Hides what the open menu covers and gives it back when the menu closes. The flags a CSS sets again while the menu is open are
 * kept as the ones to give back.
 */
static void mnRemixPanelMenuApplyHiding(MNRemixPanelMenuPanel *panel)
{
    s32 i;

    for (i = 0; i < MNREMIX_PANELMENU_HIDE_MAX; i++)
    {
        GObj *target = panel->hide_targets[i];
        MNRemixPanelMenuHidden *hidden = &panel->hidden[i];

        if ((target == NULL) || (panel->is_open == FALSE))
        {
            if ((hidden->gobj != NULL) && (hidden->gobj == target))
            {
                target->flags = hidden->saved_flags;
            }
            hidden->gobj = NULL;
            continue;
        }
        if ((hidden->gobj != target) || ((target->flags & GOBJ_FLAG_HIDDEN) == 0))
        {
            hidden->gobj = target;
            hidden->saved_flags = target->flags;
        }
        target->flags |= GOBJ_FLAG_HIDDEN;
    }
}

/** Hides the player number image of the 1P and Bonus panel by setting its width to 0, and gives the width back on close. */
static void mnRemixPanelMenuApplyGateWidth(MNRemixPanelMenuPanel *panel)
{
    SObj *sobj = panel->gate_sobj;

    if (sobj == NULL)
    {
        return;
    }
    if (panel->is_open != FALSE)
    {
        if (sobj->sprite.width != 0)
        {
            panel->gate_width = sobj->sprite.width;
            sobj->sprite.width = 0;
        }
        return;
    }
    if (panel->gate_width != 0)
    {
        sobj->sprite.width = panel->gate_width;
        panel->gate_width = 0;
    }
}

static void mnRemixPanelMenuUpdatePanel(s32 index)
{
    MNRemixPanelMenuPanel *panel = &sState.panels[index];
    sb32 is_blink_on;

    if (panel->info.is_present == FALSE)
    {
        panel->is_open = FALSE;
    }
    if ((panel->is_open != FALSE) && (panel->info.is_enabled == FALSE))
    {
        panel->is_open = FALSE;
    }
    if (panel->info.is_present != FALSE)
    {
        mnRemixPanelMenuMakeSprites(panel);
    }
    panel->blink_timer = (panel->is_open != FALSE) ? mnRemixPanelMenuRulesBlinkNext(panel->blink_timer) : 0;
    is_blink_on = (mnRemixPanelMenuRulesBlinkIsVisible(panel->blink_timer) != 0) ? TRUE : FALSE;

    if ((panel->button == NULL) || (sState.images_file == NULL))
    {
        return;
    }
    mnRemixPanelMenuPlaceButtons(panel);
    mnRemixPanelMenuPlaceItemArrows(panel, is_blink_on);
    mnRemixPanelMenuPlacePageArrows(panel, is_blink_on);
    mnRemixPanelMenuApplyHiding(panel);
    mnRemixPanelMenuApplyGateWidth(panel);

    if (panel->is_open != FALSE)
    {
        mnRemixPanelMenuDrawText(index, panel);
        return;
    }
    mnRemixPanelMenuHideText(index);
}

/** Keeps the 1P high score flag in step with the options of the human port (SinglePlayer.high_score_enabled). */
void mnRemixPanelMenuRefreshHighScores(s32 port)
{
    s32 values[MNREMIX_PANELMENU_ITEM_COUNT];
    s32 item;

    if ((sState.mode == MNREMIX_PANELMENU_MODE_VS) || (sState.mode == MNREMIX_PANELMENU_MODE_TRAINING))
    {
        return;
    }
    for (item = 0; item < MNREMIX_PANELMENU_ITEM_COUNT; item++)
    {
        values[item] = mnRemixPanelMenuGetValue(item, port);
    }
    sState.are_high_scores_enabled = mnRemixCssHighScoresUpdate(sState.mode, values);
}

static void mnRemixPanelMenuProcUpdate(GObj *gobj)
{
    s32 index;

    if (sState.sound_buffer > 0)
    {
        sState.sound_buffer--;
    }
    for (index = 0; index < MNREMIX_PANELMENU_PANELS; index++)
    {
        mnRemixPanelMenuUpdatePanel(index);
    }
    if (sState.panels[0].info.is_present != FALSE)
    {
        mnRemixPanelMenuRefreshHighScores(sState.panels[0].info.port);
    }
}

void mnRemixPanelMenuStart(s32 mode, void *images_file, void *arrows_file)
{
    memset(&sState, 0, sizeof(sState));
    sState.mode = mode;
    sState.images_file = images_file;
    sState.arrows_file = arrows_file;
    sState.are_high_scores_enabled = TRUE;

    gcAddGObjProcess(gcMakeGObjSPAfter(0, NULL, MNREMIX_PANELMENU_CONTROLLER_LINK, GOBJ_PRIORITY_DEFAULT), mnRemixPanelMenuProcUpdate,
                     nGCProcessKindFunc, 1);
}

#endif
