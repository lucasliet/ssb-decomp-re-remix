#ifndef _FTREMIXAIROLLRULES_H_
#define _FTREMIXAIROLLRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix AI for the level 10 routines that roll, repeat or drop (AI.asm doublejump, repeated input,
 * shield drop, stop_roll_spam_), so the headless unit tests (tests/remix) can run them without game state. Numbers in,
 * numbers out. */

#define FTREMIX_AI_ROLL_LEVEL_TEN 10
#define FTREMIX_AI_ROLL_SHIELD_DROP_INPUT 0x3C

/* stop_roll_spam_ (AI.asm 6039-6307). Input ids are AI.ROUTINE values; 0x00 is nFTComputerInputStickN and the
 * custom ids follow add_cpu_input_routine order (AI.asm 1603-1913). */
#define FTREMIX_AI_ROLL_SPAM_RANDOM_RANGE 16
#define FTREMIX_AI_ROLL_SPAM_SKIP (-1)
#define FTREMIX_AI_ROLL_SPAM_RESET_STICK_INPUT 0x00
#define FTREMIX_AI_ROLL_SPAM_SHORT_HOP_TOWARDS_INPUT 0x31
#define FTREMIX_AI_ROLL_SPAM_SHORT_HOP_AWAY_INPUT 0x32
#define FTREMIX_AI_ROLL_SPAM_SHORT_HOP_IN_PLACE_INPUT 0x33
#define FTREMIX_AI_ROLL_SPAM_PIVOT_AWAY_INPUT 0x37
/* Action.asm values, which are the status ids read from fp->status_id. */
#define FTREMIX_AI_ROLL_SPAM_ACTION_TURN_RUN 0x13
#define FTREMIX_AI_ROLL_SPAM_ACTION_DAMAGE_HIGH1 0x25
#define FTREMIX_AI_ROLL_SPAM_ACTION_DAMAGE_FLY_ROLL 0x37
#define FTREMIX_AI_ROLL_SPAM_ACTION_DOWN_BOUNCE_D 0x43
#define FTREMIX_AI_ROLL_SPAM_ACTION_DOWN_BOUNCE_U 0x44
#define FTREMIX_AI_ROLL_SPAM_ACTION_STUN 0xA4
#define FTREMIX_AI_ROLL_SPAM_ACTION_SLEEP 0xA5

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

/**
 * TRUE when stop_roll_spam_ applies: Giga Bowser at any level, or any CPU at level 10 or more (AI.asm checks the
 * character before the level, and `slti 10` skips the rest below level 10).
 */
extern int ftRemixAiRollSpamApplies(int is_giga_bowser, int level);

/**
 * TRUE when the CPU is moving or in an action above TurnRun: the routine resets the stick instead of rolling
 * (AI.asm `slt TurnRun, action`).
 */
extern int ftRemixAiRollSpamResetsStick(int action);

/**
 * TRUE when the opponent's action gives the CPU the advantage: any damage action from DamageHigh1 to DamageFlyRoll,
 * DownBounceD, DownBounceU, Stun or Sleep (AI.asm advantage_check_*).
 */
extern int ftRemixAiRollSpamIsAdvantageAction(int action);

/**
 * Command for an advantage CPU from one draw of 0 to 15: 0 short hop in place, 1 short hop towards, others skip.
 */
extern int ftRemixAiRollSpamAdvantageCommand(int roll_command, int random);

/**
 * Command for a CPU without advantage from one draw of 0 to 15: 0 the roll, 1 short hop in place, 2 towards,
 * 3 away, 4 and 5 pivot away, others skip.
 */
extern int ftRemixAiRollSpamNeutralCommand(int roll_command, int random);

#endif

#endif
