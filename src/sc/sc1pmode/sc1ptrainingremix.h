#ifndef _SC1PTRAININGREMIX_H_
#define _SC1PTRAININGREMIX_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Remix additions of the Training mode (Training.asm advance_frame_): the D-pad of any controller toggles the freeze (up), advances one
 * frame (right), cycles the model display (down) and resets the match (left), and the reset count is drawn on screen.
 */

/** Starts a training scene: the match is not frozen, and the reset count restarts unless the scene was loaded by a reset. */
extern void sc1PTrainingRemixStartScene(sb32 is_reset);

/** Counts a reset of the match (the D-pad shortcut and the pause menu option). */
extern void sc1PTrainingRemixOnReset(void);

/**
 * Reads the D-pad of every controller for one frame of the match. Up and right change the freeze, down changes the model display.
 *
 * @return TRUE when the D-pad asks for a reset (left), which the training scene carries out.
 */
extern sb32 sc1PTrainingRemixPollDpad(void);

/** TRUE when this frame of the match must not run (frozen, and no frame advance was asked for). Consumes a frame advance. */
extern sb32 sc1PTrainingRemixIsFrameHeld(void);

/** Draws the reset count. */
extern void sc1PTrainingRemixDrawHud(void);

#endif

#endif
