#ifndef _FTREMIXAILEVELRULES_H_
#define _FTREMIXAILEVELRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure level rules of the Remix AI (AI.asm clamp_level_9_1 .. clamp_level_9_9), so the headless unit tests
 * (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_LEVEL_CLAMP_MAX 9
#define FTREMIX_AI_LEVEL_COLL_SIZE_LV10 0.875F
#define FTREMIX_AI_LEVEL_COLL_SIZE_DEFAULT 1.0F

/**
 * Level 10 is read as level 9 by the formulas that would go negative at level 10 (FTCOMPUTER_LEVEL_MAX - level).
 */
extern int ftRemixAiLevelClamp(int level);

/**
 * Walk-stop timer threshold, (-level * 2) + 18, floored at 0 (AI.asm clamp_level_9_9: level 10 gives -2, clamped to 0).
 */
extern int ftRemixAiLevelWalkStopThreshold(int level);

/**
 * Collision size base used by the near-opponent check: 0.875 for level 10 (underestimate), 1.0 otherwise
 * (AI.asm clamp_level_9_7).
 */
extern float ftRemixAiLevelCollisionScale(int level);

#endif

#endif
