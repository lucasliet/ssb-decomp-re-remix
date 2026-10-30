#ifndef _FTREMIXAIROLLRULES_H_
#define _FTREMIXAIROLLRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix AI for the level 10 routines that roll, repeat or drop (AI.asm doublejump, repeated input,
 * shield drop, stop_roll_spam_), so the headless unit tests (tests/remix) can run them without game state. Numbers in,
 * numbers out. */

#define FTREMIX_AI_ROLL_LEVEL_TEN 10
#define FTREMIX_AI_ROLL_SHIELD_DROP_INPUT 0x3C

/**
 * TRUE for level 10, the only level that gets the advanced routines (AI.asm checks `level - 10 == 0`).
 */
extern int ftRemixAiRollIsLevelTen(int level);

/**
 * TRUE when a CPU below the ledge may try another jump: level 10 while its proc update is the aerial jump proc, or
 * the status is FallSpecial (AI.asm fix_multi_doublejump_usage).
 */
extern int ftRemixAiRollCanJumpWhileFalling(int level, int proc_is_aerial_jump, int status_is_fall_special);

/**
 * TRUE when the platform drop becomes a shield drop: level 10 and the CPU is Idle (AI.asm shield_drop_).
 */
extern int ftRemixAiRollShieldDrops(int level, int is_idle);

#endif

#endif
