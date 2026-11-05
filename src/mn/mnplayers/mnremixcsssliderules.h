#ifndef _MNREMIXCSSSLIDERULES_H_
#define _MNREMIXCSSSLIDERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the slide-in of the Remix VS character select portraits (CharacterSelect.asm draw_portraits_ and the patched
 * portrait animation routine, 0x80131DC8): the portraits of the left half come from the left of the screen, the ones of the
 * right half from the right, and each column has its own speed. The bonus bookend is the 11th column and comes from the
 * left, the Random bookend is the 12th and comes from the right.
 */

#define MNREMIX_CSS_SLIDE_BONUS_COLUMN 10
#define MNREMIX_CSS_SLIDE_RANDOM_COLUMN 11

/**
 * Column of the slide tables for a portrait.
 *
 * @param slot Portrait number: 0..29 grid slots, 30 bonus bookend, 31 Random bookend.
 * @return Column, 0..11, or -1 for a number that is not a portrait.
 */
extern int mnRemixCssSlideRulesColumn(int slot);

/** X where the portrait of `column` starts: -20 for the left half and the bonus bookend, 310 for the right half and the Random bookend. */
extern float mnRemixCssSlideRulesStartX(int column);

/**
 * X of the portrait on the next frame: `current_x` moved by the speed of its column, stopping at the final x of the column.
 *
 * @param column Column, 0..11.
 * @param current_x X of the portrait now.
 * @return New x.
 */
extern float mnRemixCssSlideRulesNextX(int column, float current_x);

#endif

#endif
