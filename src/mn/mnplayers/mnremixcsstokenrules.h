#ifndef _MNREMIXCSSTOKENRULES_H_
#define _MNREMIXCSSTOKENRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix character select tokens (src/CharacterSelect.asm): where a token rests on its portrait, the box
 * the placed token is held in, the offset a held token keeps from the hand, and the limits of the cursor. Slots are the
 * portrait numbering of the asm: 0 to 29 the grid, 30 the bonus bookend, 31 the Random bookend.
 */

#define MNREMIX_CSS_TOKEN_GRID_COLUMNS 10
#define MNREMIX_CSS_TOKEN_GRID_SLOTS 30
#define MNREMIX_CSS_TOKEN_BONUS_SLOT 30
#define MNREMIX_CSS_TOKEN_RANDOM_SLOT 31

/** Offset of a held token from the hand sprite (adjust_scaled_token_position_: TOKEN_X_OFFSET 12, TOKEN_Y_OFFSET -10). */
#define MNREMIX_CSS_TOKEN_FOLLOW_X 12.0F
#define MNREMIX_CSS_TOKEN_FOLLOW_Y (-10.0F)

/** Lowest the cursor goes, so the Reset button of the 12CB and the bottom rows of the panel menu are reachable (CharacterSelect.asm:992-1004). */
#define MNREMIX_CSS_CURSOR_MAX_Y 212.0F

/** Distance under which two placed tokens of the same portrait push each other on the VS CSS (CharacterSelect.asm:1076, 15 in vanilla). */
#define MNREMIX_CSS_TOKEN_OVERLAP_RADIUS 10.0F

/**
 * Position of a token put on a portrait by code, for example a Random roll (place_token_from_id_): x = START_X + 12 + 24 column,
 * y = START_Y + 14 + 24 row; the bonus bookend is (21, 70) and the Random bookend (278, 70).
 *
 * @param slot Portrait slot, 0 to 31; any other value reads as slot 0.
 * @param pos_x Receives the x of the token sprite.
 * @param pos_y Receives the y of the token sprite.
 */
extern void mnRemixCssTokenRestPosition(int slot, float *pos_x, float *pos_y);

/**
 * Box a placed token is held in (token_autoposition_, fix_right_boundary_check_, fix_bottom_boundary_check_): x = START_X + 12 +
 * 24 column, y = START_Y + 12 + 24 row, 24 x 24; the bonus bookend starts at x 24 and the Random one at 279, both 15 x 72 from y 46.
 *
 * @param slot Portrait slot, 0 to 31; any other value reads as slot 0.
 * @param edge_x Receives the left edge.
 * @param edge_y Receives the top edge.
 * @param width Receives the width.
 * @param height Receives the height.
 */
extern void mnRemixCssTokenEdgeBox(int slot, float *edge_x, float *edge_y, float *width, float *height);

#endif

#endif
