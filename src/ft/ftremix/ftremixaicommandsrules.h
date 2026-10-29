#ifndef _FTREMIXAICOMMANDSRULES_H_
#define _FTREMIXAICOMMANDSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix CPU stick and turn commands (AI.asm extend_stick_x_commands and
 * CUSTOM_COMMANDS.TURNAROUND_WAIT), so the headless unit tests (tests/remix) can run them.
 */

#define FTREMIX_AI_STICK_FULL 80
#define FTREMIX_AI_STICK_HALF 40

#define FTREMIX_AI_STICK_AUTOFULL_AWAY 0x81
#define FTREMIX_AI_STICK_AUTOHALF_AWAY 0x82
#define FTREMIX_AI_STICK_FORWARDS 0x83
#define FTREMIX_AI_STICK_BACKWARDS 0x84

#define FTREMIX_AI_ACTION_TURN 0x012
#define FTREMIX_AI_ACTION_TURN_RUN 0x013
#define FTREMIX_AI_ACTION_JUMP_SQUAT 0x014

/**
 * Stick X value of an extended stick code (0x81 to 0x84). `player_x` and `target_x` are the CPU's top joint and its
 * target; `facing_lr` is FTStruct.lr (1 right, -1 left). AUTO*_AWAY points away from the target (ties count as the
 * player being on the target's right), FORWARDS and BACKWARDS follow or oppose the facing. Returns 0 for other codes.
 */
extern int ftRemixAiRulesStickXFromCode(int code, float player_x, float target_x, int facing_lr);

/** TRUE when `status_id` is a turn or a turn run, the actions TURNAROUND_WAIT waits out. */
extern int ftRemixAiRulesIsTurnAction(int status_id);

#endif

#endif
