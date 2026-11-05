#ifndef _FTREMIXAIMODESRULES_H_
#define _FTREMIXAIMODESRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the CPU hooks of the Remix game modes (Smashketball.asm ai_ball_behavior_ and ai_ball_attack_range,
 * KingOfTheHill.asm ai_hill_behavior_), so the headless unit tests (tests/remix) can run them without game state. */

/* ai_ball_behavior_: the CPU seeks a point this far behind the ball, so its attacks push the ball toward the other hoop. */
#define FTREMIX_AI_MODES_BALL_SEEK_OFFSET 400.0F

/* ai_ball_attack_range: the ball is detected as a 200 x 200 hurtbox. */
#define FTREMIX_AI_MODES_BALL_HURTBOX_SIZE 200.0F

/* Basketball.asm GRAVITY and FALL_SPEED (the same values as ITREMIX_BASKETBALL_GRAVITY and _FALL_SPEED). */
#define FTREMIX_AI_MODES_BALL_GRAVITY 1.65F
#define FTREMIX_AI_MODES_BALL_FALL_SPEED (-90.0F)

/* ai_hill_behavior_: the target is the hill marker moved down by the marker raise. */
#define FTREMIX_AI_MODES_HILL_MARKER_DROP 500.0F

/**
 * X offset from the ball of the point the CPU of `team` seeks: -400 for the home team (it attacks toward +X), +400 otherwise.
 * The asm compares the fighter team with a stale ITStruct word instead of the home team (see docs/remix/ai.md section 10).
 */
float ftRemixAiModesBallSeekOffset(int team, int home_team);

/** Squared distance from (from_x, from_y) to (to_x, to_y). */
float ftRemixAiModesSquaredDistance(float from_x, float from_y, float to_x, float to_y);

/** Y of the point the CPU walks to on the hill: the marker Y minus FTREMIX_AI_MODES_HILL_MARKER_DROP. */
float ftRemixAiModesHillTargetY(float marker_y);

#endif

#endif
