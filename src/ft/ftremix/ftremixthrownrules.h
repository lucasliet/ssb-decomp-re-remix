#ifndef _FTREMIXTHROWNRULES_H_
#define _FTREMIXTHROWNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rule of bowser.asm thrown_mario_bros_fix_, so the headless unit tests (tests/remix) can run it without game state. */

/**
 * thrown_mario_bros_fix_: the thrown fighter keeps its ThrownMarioBros status when the throw animation ends instead of
 * switching to the queued status. Bowser, Giga Bowser and Polygon Bowser.
 */
extern int ftRemixThrownRulesKeepsStatusAtAnimEnd(int fkind);

#endif

#endif
