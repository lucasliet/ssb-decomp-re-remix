#ifndef _SC1PTRAININGREMIXACTION_H_
#define _SC1PTRAININGREMIXACTION_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Show Action of the Remix Training (Training.asm run_ 1969-1978 L toggle, setup_ 1634-1637 legend, create_action_strings_ 1475-1565,
 * update_frame_counts_ 2097-2152, update_actions_ 2156-2277): L in the pause turns on, for every fighter, its action name and the
 * frames spent in that action, drawn down the left of the screen. Action names are read from the Remix RAM image (shared table for
 * action ids below 0xDC, the character table from there on).
 */

/** Scene start (setup_ clears the previous action, frame count and string of every port). The Show Action flag is kept like the asm variable. */
extern void sc1PTrainingRemixActionStartScene(void);

/** Scene start: makes the L glyph of the "Show Action" legend (group 0x15, the custom pause group). */
extern void sc1PTrainingRemixActionMakeLegend(void);

/** After a game frame ran (the object routine of group 0xD, update_frame_counts_): counts the frames of every fighter's action. */
extern void sc1PTrainingRemixActionCountFrame(void);

/** Every frame: L in the pause flips Show Action, the legend shows with the pause and the action rows show while the flag is on. */
extern void sc1PTrainingRemixActionUpdate(void);

#endif

#endif
