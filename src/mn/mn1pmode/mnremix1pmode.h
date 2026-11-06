#ifndef _MNREMIX1PMODE_H_
#define _MNREMIX1PMODE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/objdef.h>

#define MNREMIX_1PMODE_PAGE_1 0
#define MNREMIX_1PMODE_PAGE_2 1

#define MNREMIX_1PMODE_OPTION_REMIX_MODES 5
#define MNREMIX_1PMODE_OPTION_COUNT_MAX 6

typedef struct MNRemix1PModeRow
{
    sb32 is_small;
    s32 scene;
    s32 mode;
    s32 page;
    f32 box_x;
    f32 box_y;
    f32 text_x;
    f32 text_y;
    u32 text_offset;
} MNRemix1PModeRow;

/**
 * Remix 1P menu (SinglePlayerMenus.asm menu_button_table, remix_menu_button_table): page 1 has the vanilla buttons, Bonus 3 and
 * Remix Modes, page 2 (Remix Modes) has Remix 1P, All-Star, Multiman, Cruel Multiman and Home-Run Contest. The page lives
 * here (SinglePlayerModes.page_flag); the chosen mode is gSC1PRemixMode.
 */
extern s32 gMNRemix1PModePage;

/** Index of the last button of the current page (the cursor wraps between 0 and it). */
extern s32 mnRemix1PModeGetLastOption(void);

/** Row of the button `option` of the current page. */
extern const MNRemix1PModeRow* mnRemix1PModeGetRow(s32 option);

/**
 * Creates the tab and the text of the button `option` of the current page.
 *
 * @param option Button index on the page.
 * @param common_file Common menu file (tab sprites).
 * @param mn1p_file 1P menu file (text sprites).
 * @param is_highlight TRUE when the cursor is on the button.
 * @return The button object.
 */
extern GObj* mnRemix1PModeMakeButton(s32 option, void *common_file, void *mn1p_file, sb32 is_highlight);

/**
 * Sets the colours of a button tab and its text for the state `status` (nMNOptionTabStatus*).
 *
 * @param gobj Button object.
 * @param status Highlight, not selected or selected.
 * @param option Button index on the page.
 */
extern void mnRemix1PModeSetButtonColors(GObj *gobj, s32 status, s32 option);

/**
 * Button the cursor starts on when the menu opens: Remix Modes when the menu came back from the second page, the first button
 * of the second page when it was just opened, the button of the mode when the player returns from a Remix mode, otherwise
 * `vanilla_option`.
 *
 * @param prev_scene Scene the menu came from.
 * @param vanilla_option Button the vanilla code picked from the previous scene.
 */
extern s32 mnRemix1PModeGetStartOption(s32 prev_scene, s32 vanilla_option);

/**
 * A or START on the button `option`: stores its mode and page and points the scene data at the scene it opens.
 *
 * @param option Button index on the page.
 * @param player Controller number (1..4) of the player that pressed.
 */
extern void mnRemix1PModeSelect(s32 option, s32 player);

/** Clears the chosen 1P mode and goes back to the first page; called when a Mode Select button is picked. */
extern void mnRemix1PModeReset(void);

/** B on the menu: from the second page it goes back to the first one, from the first one to the main menu. */
extern void mnRemix1PModeBack(void);

#endif

#endif
