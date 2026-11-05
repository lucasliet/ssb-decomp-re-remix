#ifndef _FTREMIXJUMPAERIALRULES_H_
#define _FTREMIXJUMPAERIALRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix double jump patches (yoshishared.asm yoshi_dj_fix_1, JYoshi.asm joshi_armor, nessshared.asm
 * ness_jump1_/ness_jump2_ with the Character.ness_jump table), so the headless unit tests (tests/remix) can run them without
 * game state. Numbers in, numbers out. */

/* JYoshi.asm joshi_armor: 0x42DC0000, the Japanese Yoshi value */
#define FTREMIX_JUMPAERIAL_J_YOSHI_KNOCKBACK_RESIST 110.0F

/**
 * yoshi_dj_fix_1: fighters that get the Yoshi double jump (Yoshi physics and knockback resist). Yoshi, Polygon Yoshi and
 * J Yoshi.
 */
extern int ftRemixJumpAerialRulesUsesYoshiPhysics(int fkind);

/**
 * joshi_armor: knockback resist of the Yoshi double jump. J Yoshi gets the Japanese value, every other fighter keeps
 * `vanilla_resist`.
 */
extern float ftRemixJumpAerialRulesYoshiKnockbackResist(int fkind, float vanilla_resist);

/**
 * Character.ness_jump: fighters that double jump like Ness (x drift held in the status vars). Ness, Polygon Ness, J Ness,
 * Lucas, Polygon Lucas, Mewtwo, Polygon Mewtwo, Peach and Polygon Peach.
 */
extern int ftRemixJumpAerialRulesUsesNessPhysics(int fkind);

#endif

#endif
