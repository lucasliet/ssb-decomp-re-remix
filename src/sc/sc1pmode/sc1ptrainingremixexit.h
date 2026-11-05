#ifndef _SC1PTRAININGREMIXEXIT_H_
#define _SC1PTRAININGREMIXEXIT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Hold to Exit of the Remix Training (Toggles.asm entry_hold_to_exit_training, Training.asm hold_to_exit_ 751-809, setup_ 1590-1611,
 * run_ 2002-2038): with the toggle on, the Exit option of the pause menu needs A held for 33 frames. While the cursor is on Exit the
 * text "(Hold A)" shows with a blue bar that fills in 32 frames. With the toggle off Exit runs on a tap of A like the vanilla menu.
 */

/** Scene start: the hold counter and the bar are empty. */
extern void sc1PTrainingRemixExitStartScene(void);

/** Scene start: makes the A glyph of the "(Hold --)" text (hidden until the cursor is on Exit). */
extern void sc1PTrainingRemixExitMakeLegend(void);

/** TRUE when the Exit option must run on this frame (hold_to_exit_ replaces the A tap test of the vanilla option). */
extern sb32 sc1PTrainingRemixExitIsRequested(void);

/** The cursor of the pause menu is not on Exit: the held frames restart (hold_to_exit_._reset). */
extern void sc1PTrainingRemixExitResetHold(void);

/**
 * Every frame: shows the text and the bar while the vanilla pause menu has the cursor on Exit with Hold to Exit on.
 *
 * @param is_cursor_on_exit The pause menu cursor is on the Exit option.
 */
extern void sc1PTrainingRemixExitUpdate(sb32 is_cursor_on_exit);

#endif

#endif
