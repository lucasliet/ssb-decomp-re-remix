#ifndef _SC1PREMIXRETRY_H_
#define _SC1PREMIXRETRY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Whether the run is one of the modes that show the "L: RETRY" legend and retry on L: Bonus 3, Multi-Man, Cruel Multi-Man and the
 * Home-Run Contest (the fake screen 0x77 of SinglePlayerModes.asm _reset_button).
 */
extern sb32 sc1PRemixRetryIsModeAvailable(void);

/**
 * Starts a retry from the pause menu when L is pressed in a Bonus 3, Multi-Man, Cruel Multi-Man or Home-Run Contest run
 * (SinglePlayerModes.asm _reset, `_multiman`: reset_flag is set). The caller ends the scene; sc1PRemixModeLeaveRun then restarts
 * the run instead of going back to the Bonus 2 CSS.
 *
 * @param button_tap Buttons pressed this frame by the pausing player.
 * @return TRUE when the retry started.
 */
extern sb32 sc1PRemixRetryTryByPause(u16 button_tap);

/**
 * Starts a retry while GAME SET shows when L is held on any controller (Practice_1P.asm hold_l_to_retry_): in a Bonus 1 or Bonus 2
 * practice run (the Bonus stage restarts, sc1PBonusStageStartScene consumes the retry) and in Bonus 3, Multi-Man, Cruel Multi-Man
 * and the Home-Run Contest (sc1PRemixModeLeaveRun consumes it). The caller ends the scene.
 *
 * @return TRUE when the retry started.
 */
extern sb32 sc1PRemixRetryTryByGameSet(void);

/**
 * TRUE once per retry (reset_flag of the asm), and clears the flag so the next run ends normally.
 */
extern sb32 sc1PRemixRetryConsume(void);

#endif

#endif
