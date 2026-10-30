#ifndef _FTREMIXAILV10RULES_H_
#define _FTREMIXAILV10RULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the level 10 CPU hooks (AI.asm level_ten scope), so the headless unit tests (tests/remix) can run them
 * without game state. Numbers in, numbers out. */

#define FTREMIX_AI_LV10_LEVEL 10

/* Custom routine ids: 0x31 + index of add_cpu_input_routine in AI.asm 1603-1913 (CLIFF_LET_GO is the 16th), checked against AI.command_table. */
#define FTREMIX_AI_LV10_ROUTINE_CLIFF_LET_GO 0x40
#define FTREMIX_AI_LV10_ROUTINE_DASH_ATTACK 0x47
#define FTREMIX_AI_LV10_ROUTINE_USP_TOWARDS 0x48
#define FTREMIX_AI_LV10_ROUTINE_JUMPCANCEL_UPSMASH 0x4A
#define FTREMIX_AI_LV10_ROUTINE_JUMPCANCEL_USP 0x4B
#define FTREMIX_AI_LV10_ROUTINE_NULL 0x4C

/* Vanilla attack inputs (AI.asm ATTACK_TABLE.USMASH.INPUT and ATTACK_TABLE.USPG/USPA.INPUT). */
#define FTREMIX_AI_LV10_INPUT_USMASH 0x19
#define FTREMIX_AI_LV10_INPUT_USP 0x0D

/* Frame bit that splits the START mash in 16 frames pressed and 16 released (AI.asm _check_respawn, andi 0x0010). */
#define FTREMIX_AI_LV10_MASH_FRAME_BIT 0x10

/**
 * TRUE for level 10 and above (AI.asm slti 10 checks).
 */
extern int ftRemixAiIsLevelTen(int cpu_level);

/**
 * TRUE when an enemy counts as a Starman for the evade check: the enemy has Starman, or the CPU is level 10 and the enemy
 * still has respawn invincibility (AI.asm check_invulnerable_).
 */
extern int ftRemixAiCountsAsInvulnerableEnemy(int star_invincible, int respawn_tics, int cpu_level);

/**
 * Returns the cliff getup input: CLIFF_LET_GO when the roll shifted right by one is zero, else normal_input
 * (AI.asm extend_cliff_attack_). The roll is Global.get_random_int_(2), so 0 or 1.
 */
extern int ftRemixAiCliffGetupInputFromRoll(int roll, int normal_input);

/**
 * Buttons of a dead CPU for the START mash: START is pressed in the frames whose frame bit is set, released otherwise
 * (AI.asm _check_respawn). START is Joypad.START, 0x1000.
 */
extern int ftRemixAiStartMashButtons(int buttons, int frame_count);

/**
 * TRUE when the dash attack command must be cancelled: the command is the dash attack and the CPU is neither idle nor
 * running (AI.asm _check_if_dash_attack_input_is_possible).
 */
extern int ftRemixAiDashAttackCancelNeeded(int is_dash_attack, int is_idle_or_run);

#endif

#endif
