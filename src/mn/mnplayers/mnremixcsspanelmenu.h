#ifndef _MNREMIXCSSPANELMENU_H_
#define _MNREMIXCSSPANELMENU_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objtypes.h>
#include "mn/mnplayers/mnremixcsspanelmenurules.h"

/*
 * Per-port panel menu of the Remix character select screens (src/CharacterSelectDebugMenu.asm): a gold button at the top-left of
 * each panel opens a menu over the panel with the options of the player panel registry (port/remix/remix_player_panel.h), three
 * per page, with item arrows, page arrows and a reset button. docs/remix/notes/f16_css_panel_menu.md has the asm map.
 *
 * Each CSS tells the menu where its panels are once per frame (mnRemixPanelMenuSetPanel), forwards the A button of its cursors
 * (mnRemixPanelMenuPress, mnRemixPanelMenuHoldPress) and asks mnRemixPanelMenuIsOpen where the asm hooks hide or disable
 * something of a panel while its menu is open.
 */

#define MNREMIX_PANELMENU_PANELS 4

typedef struct MNRemixPanelMenuPanelInfo
{
    sb32 is_present;    /* the panel exists (the asm: the token object exists) */
    sb32 is_enabled;    /* the button may be pressed (VS: the doors are open all the way) */
    sb32 is_cpu;        /* VS and Training: the panel is not a human one, so it takes the CPU list */
    s32 port;           /* port whose options the menu edits */
    f32 x;              /* left edge of the panel card */
    f32 y;              /* top edge of the panel card */
    s32 width;          /* width of the panel card */

} MNRemixPanelMenuPanelInfo;

/**
 * Starts the menu for a scene: forgets the objects of the previous visit and makes the controller object that runs the menu every
 * frame (after the processes of the CSS, so what it hides stays hidden).
 *
 * @param mode MNRemixPanelMenuMode of the screen.
 * @param images_file Loaded Remix CSS images file (File.CSS_IMAGES), where the button sprites are.
 * @param arrows_file Loaded MNPlayersCommon file, where the arrow sprites are.
 */
extern void mnRemixPanelMenuStart(s32 mode, void *images_file, void *arrows_file);

/**
 * Tells the menu about one panel for this frame.
 *
 * @param index Panel index of the screen, 0 to MNREMIX_PANELMENU_PANELS - 1.
 * @param info Where the panel is and which port it edits.
 */
extern void mnRemixPanelMenuSetPanel(s32 index, const MNRemixPanelMenuPanelInfo *info);

/**
 * Gives the objects of a panel that the menu hides while it is open (prevent_player_model_display_, set_panel_display_: the
 * fighter, the name and series logo, the HMN/CPU/NA button, the team button, the handicap and CPU level objects). They come back
 * as they were when the menu closes. Call every frame, the GObjs of a panel change during the screen.
 *
 * @param index Panel index of the screen.
 * @param targets GObjs, NULL entries are skipped.
 * @param count Number of entries.
 */
extern void mnRemixPanelMenuSetHideTargets(s32 index, GObj *const *targets, s32 count);

/**
 * Gives the player number image of the 1P and Bonus panel, which the menu hides by setting its width to 0 (update_debug_menu_
 * _run_every_frame_1p). Call every frame.
 *
 * @param index Panel index of the screen.
 * @param sobj Second SObj of the panel GObj, NULL when there is none.
 */
extern void mnRemixPanelMenuSetGateSprite(s32 index, SObj *sobj);

/** TRUE when the menu of the panel is open. FALSE for an index outside the panels. */
extern sb32 mnRemixPanelMenuIsOpen(s32 index);

/**
 * Handles a fresh A press of a cursor (press_a_handler_): the panel button, the reset button, the item arrows and the page
 * arrows, in that order. Call it where the CSS handles an A press that nothing else took.
 *
 * @param cursor_x Position X of the cursor sprite.
 * @param cursor_y Position Y of the cursor sprite.
 * @return TRUE when the press was used.
 */
extern sb32 mnRemixPanelMenuPress(f32 cursor_x, f32 cursor_y);

/**
 * One frame of the hold A repeat of a cursor (hold_a_handler_): after the wait, the item and page arrows repeat while A is held.
 * Call every frame for every cursor, with A_BUTTON of its button_hold.
 *
 * @param cursor_port Port of the cursor.
 * @param is_a_held TRUE while A is held.
 * @param cursor_x Position X of the cursor sprite.
 * @param cursor_y Position Y of the cursor sprite.
 */
extern void mnRemixPanelMenuHoldPress(s32 cursor_port, sb32 is_a_held, f32 cursor_x, f32 cursor_y);

/** TRUE while the 1P high scores stay on (SinglePlayer.high_score_enabled): no option that disables them is off its default. */
extern sb32 mnRemixPanelMenuAreHighScoresEnabled(void);

#endif

#endif
