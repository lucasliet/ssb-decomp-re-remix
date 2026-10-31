#ifndef _FTREMIXTRAINING_H_
#define _FTREMIXTRAINING_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fttypes.h>
#include <ssb_types.h>

/*
 * Fighter side of the Remix Training custom menu (Training.asm shield_break_mode_, spam_practice_, spam_practice_timer_,
 * di_practice_mode_). The entries come from remix_training_get (port menu Remix > Training); every hook is a no-op outside Training.
 */

/** Clears the per-port shield break status and spam timers (Training.asm load_from_sss_ and load_from_reset_). */
extern void ftRemixTrainingStartScene(void);

/**
 * Shield break CPU: refills or holds the shield and presses the OOS option on the controller of a CPU fighter. Called after the
 * player input of the fighter is built (Training.asm hook 0x800E142C).
 */
extern void ftRemixTrainingShieldBreakInput(FTStruct *fp);

/**
 * Spam Practice: replaces the objective of a CPU fighter with the chosen move when its timer ran out (Training.asm hook 0x8013A3D0).
 *
 * @return TRUE when the CPU script was set and the normal objective must be skipped.
 */
extern sb32 ftRemixTrainingSpamSetObjective(FTStruct *fp);

/** One frame of the Training objects: counts the spam timers down and applies DI Practice Mode (hitlag 2 on every fighter). */
extern void ftRemixTrainingUpdateFighters(void);

#endif

#endif
