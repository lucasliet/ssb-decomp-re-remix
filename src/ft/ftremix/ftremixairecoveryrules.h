#ifndef _FTREMIXAIRECOVERYRULES_H_
#define _FTREMIXAIRECOVERYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure recovery rules of the Remix AI (AI.asm recovery_fix_, fix_target_pos_x), so the headless unit tests (tests/remix)
 * can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_RECOVERY_BLASTZONE_PADDING 200.0F
#define FTREMIX_AI_RECOVERY_CPU_LEVEL_MAX 10

/**
 * TRUE when the CPU may repeat its up-B recovery (the vanilla "already used special up" check is skipped).
 * A level 10 CPU always repeats. Vanilla 1P/RTTF (classic mode) never repeats. Otherwise the improved AI decides.
 * The Remix 1P automatic improved AI is not part of this rule yet (REMIX-TODO(AI)).
 */
extern int ftRemixAiRecoveryUpBRepeatAllowed(int cpu_level, int is_classic_mode, int improved_ai);

/**
 * Clamps a target X inside the blastzone shrunk by FTREMIX_AI_RECOVERY_BLASTZONE_PADDING, so a CPU does not walk off
 * stages with walk-offs (AI.asm fix_target_pos_x). The left bound is checked first and wins over the right bound.
 */
extern float ftRemixAiRecoveryClampTargetX(float target_x, float left_blastzone, float right_blastzone);

#endif

#endif
