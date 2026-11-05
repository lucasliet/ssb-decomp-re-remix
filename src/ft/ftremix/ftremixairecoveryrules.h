#ifndef _FTREMIXAIRECOVERYRULES_H_
#define _FTREMIXAIRECOVERYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure recovery rules of the Remix AI (AI.asm recovery_fix_, fix_target_pos_x), so the headless unit tests (tests/remix)
 * can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_RECOVERY_BLASTZONE_PADDING 200.0F
#define FTREMIX_AI_RECOVERY_CPU_LEVEL_MAX 10

/**
 * TRUE when the CPU may repeat its up-B recovery (the vanilla "already used special up" check is skipped).
 * Remix 1P (automatic improved AI) and a level 10 CPU always repeat. Vanilla 1P/RTTF (classic mode) never repeats.
 * Otherwise the improved AI decides. The Remix 1P test comes first because Remix 1P also reports the 1P game type.
 */
extern int ftRemixAiRecoveryUpBRepeatAllowed(int cpu_level, int is_classic_mode, int improved_ai, int is_remix_1p);

/**
 * Clamps a target X inside the blastzone shrunk by FTREMIX_AI_RECOVERY_BLASTZONE_PADDING, so a CPU does not walk off
 * stages with walk-offs (AI.asm fix_target_pos_x). The left bound is checked first and wins over the right bound.
 */
extern float ftRemixAiRecoveryClampTargetX(float target_x, float left_blastzone, float right_blastzone);

/** TRUE for the Wolf USP actions (E3, E4, E6, E8) that aim the recovery target straight up (fix_remix_recovery). */
extern int ftRemixAiRecoveryIsWolfUspAction(int action);

/** TRUE when the CPU is in its NSP charge action of charge_action (a negative charge_action never matches). */
extern int ftRemixAiRecoveryIsNspChargeAction(int action, int charge_action);

/** TRUE when a Banjo fighter is in Banjo Flight (0xEF) at frame 21, the frame that presses B (fix_remix_recovery). */
extern int ftRemixAiRecoveryIsBanjoFlapFrame(int action, int frame);

/** TRUE when the clipping below is -1 (no ground), so Banjo presses B by command (fix_remix_recovery, _banjo_flight). */
extern int ftRemixAiRecoveryBanjoNeedsPressB(int floor_line_id);

/** TRUE when the vanilla Pikachu recovery branch runs: vanilla 1P/RTTF (classic mode) or the improved AI is off. */
extern int ftRemixAiRecoveryKeepsPikachuVanilla(int is_classic_mode, int improved_ai);

#endif

#endif
