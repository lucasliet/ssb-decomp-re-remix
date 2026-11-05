#ifndef _MNREMIXCSSPANELMENUSTATE_H_
#define _MNREMIXCSSPANELMENUSTATE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcsspanelmenu.h"

/*
 * State shared by the files of the panel menu (mnremixcsspanelmenu.c, mnremixcsspanelmenutext.c, mnremixcsspanelmenuinput.c).
 * Not for the CSS files: they use mnremixcsspanelmenu.h.
 */

#define MNREMIX_PANELMENU_HIDE_MAX 8

typedef struct MNRemixPanelMenuHidden
{
    GObj *gobj;
    u32 saved_flags;

} MNRemixPanelMenuHidden;

typedef struct MNRemixPanelMenuPanel
{
    MNRemixPanelMenuPanelInfo info;
    sb32 is_open;
    s32 page;
    s32 blink_timer;
    s32 item_count;
    s32 items[MNREMIX_PANELMENU_ITEM_COUNT];
    GObj *button;
    GObj *reset;
    GObj *item_arrows[MNREMIX_PANELMENU_ROWS][2];
    GObj *page_arrows[2];
    GObj *hide_targets[MNREMIX_PANELMENU_HIDE_MAX];
    s32 hide_count;
    MNRemixPanelMenuHidden hidden[MNREMIX_PANELMENU_HIDE_MAX];
    SObj *gate_sobj;
    s32 gate_width;

} MNRemixPanelMenuPanel;

typedef struct MNRemixPanelMenuState
{
    s32 mode;
    void *images_file;
    void *arrows_file;
    MNRemixPanelMenuPanel panels[MNREMIX_PANELMENU_PANELS];
    MNRemixPanelMenuHold holds[MNREMIX_PANELMENU_PANELS];
    s32 sound_buffer;
    sb32 are_high_scores_enabled;

} MNRemixPanelMenuState;

/** State of the screen in progress. */
extern MNRemixPanelMenuState *mnRemixPanelMenuGetState(void);

/** Highest page of a panel, 0 based. */
extern s32 mnRemixPanelMenuGetMaxPage(const MNRemixPanelMenuPanel *panel);

/** TRUE when a panel shows its menu and takes presses. */
extern sb32 mnRemixPanelMenuIsPanelActive(const MNRemixPanelMenuPanel *panel);

/** Text slot of a row label (is_value FALSE), a row value (TRUE) or the page number (row -1) of a panel. */
extern s32 mnRemixPanelMenuGetTextSlot(s32 panel_index, s32 row, sb32 is_value);

/** Hides the labels, values and page number of a panel at once (a label otherwise stays one more frame). */
extern void mnRemixPanelMenuHideText(s32 panel_index);

/** Puts the labels, values and page number of an open panel on screen for this frame. */
extern void mnRemixPanelMenuDrawText(s32 panel_index, const MNRemixPanelMenuPanel *panel);

/** Text the value of an item shows for a port: the option name, a number, or the player tag name. */
extern void mnRemixPanelMenuGetValueText(s32 item, s32 port, char *text, s32 text_size);

/** Value of an item for a port from the registry. */
extern s32 mnRemixPanelMenuGetValue(s32 item, s32 port);

/** Sets the value of an item for a port in the registry. */
extern void mnRemixPanelMenuSetValue(s32 item, s32 port, s32 value);

/** Recomputes the high score flag from the registry, for the human port of the screen (1P, Bonus and All-Star only). */
extern void mnRemixPanelMenuRefreshHighScores(s32 port);

#endif

#endif
