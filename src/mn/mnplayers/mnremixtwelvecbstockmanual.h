#ifndef _MNREMIXTWELVECBSTOCKMANUAL_H_
#define _MNREMIXTWELVECBSTOCKMANUAL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Controls of the Manual stock mode in the 12-Character Battle panels (CharacterSelect.asm update_stock_icon_and_count_indicators_
 * 5640-5960, check_manual_stock_arrow_press_, arrow_state_routine_): with a character selected two blinking arrows at the edges
 * of the card step the stocks of the portrait, with the token still held a D-pad icon (and two yellow marks on its left and right)
 * tells that D-pad left and right do the same.
 */

typedef enum MNRemixTwelveCBStockControls
{
    MNREMIX_TCB_STOCK_CONTROLS_NONE,
    MNREMIX_TCB_STOCK_CONTROLS_ARROWS,
    MNREMIX_TCB_STOCK_CONTROLS_DPAD
} MNRemixTwelveCBStockControls;

/** Forgets the sprites of the previous visit of the CSS (their scene is gone). */
extern void mnRemixTwelveCBStockManualReset(void);

/**
 * Shows the controls of one panel for this frame, or hides them; call once per frame for each of the two panels.
 *
 * @param player Player (0 or 1) of the panel.
 * @param controls Which controls to show.
 * @param panel_x Left edge of the panel card.
 * @param card_width Width of the panel card.
 * @param digits_x Left edge of the first digit of the stock number.
 * @param arrows_file Loaded CSS file that holds the arrow sprites (sMNPlayersVSFiles[0]).
 */
extern void mnRemixTwelveCBStockManualUpdate(s32 player, MNRemixTwelveCBStockControls controls, f32 panel_x, f32 card_width, f32 digits_x,
                                             void *arrows_file);

/**
 * Arrow a cursor press hits (check_manual_stock_arrow_press_): the left arrow takes a margin of 8 on its right, the right one on its left.
 *
 * @param player Player (0 or 1) of the panel.
 * @param hot_x Cursor hot spot X.
 * @param hot_y Cursor hot spot Y.
 * @return -1 for the left arrow, 1 for the right one, 0 for none (also when the arrows are not shown).
 */
extern s32 mnRemixTwelveCBStockManualArrowAt(s32 player, f32 hot_x, f32 hot_y);

#endif

#endif
