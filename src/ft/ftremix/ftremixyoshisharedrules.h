#ifndef _FTREMIXYOSHISHAREDRULES_H_
#define _FTREMIXYOSHISHAREDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of yoshishared.asm (Yoshi clones hard-coded in the vanilla shield, grab, throw and egg routines), JYoshi.asm
 * (joshi_egg_timer, joshi_mash) and the yoshi_egg rows of Bowser.asm, Dedede.asm, GBowser.asm, NBowser.asm, NDedede.asm and
 * Piano.asm, so the headless unit tests (tests/remix) can run them without game state. Numbers in, numbers out. */

/* JYoshi.asm joshi_egg_timer (210 frames instead of 250) and joshi_mash (630 base inputs instead of 750) */
#define FTREMIX_YOSHI_J_EGG_ESCAPE_WAIT_MAX 210
#define FTREMIX_YOSHI_J_EGG_BREAKOUT_INPUTS_MIN 630

/* Floats of one Character.yoshi_egg row: effect size, hurtbox offset x y z, hurtbox size x y z (ftCommonYoshiEggDesc) */
#define FTREMIX_YOSHI_EGG_ROW_FLOATS 7

/**
 * yoshi_shield_fix_1..8, ftparam.c egg escape effect: Yoshi and J Yoshi shield, roll and parry as an egg. Polygon Yoshi
 * keeps the normal shield.
 */
extern int ftRemixYoshiSharedRulesHasEggShield(int fkind);

/**
 * yoshi_grab_fix_1/2: the grabber puts its tongue part (catch wait) and swallows its victim (capture wait). Yoshi,
 * Polygon Yoshi and J Yoshi.
 */
extern int ftRemixYoshiSharedRulesIsGrabSwallower(int fkind);

/**
 * yoshi_throw_fix_1: the thrown victim stays invisible and intangible after the throw. Yoshi, Polygon Yoshi, J Yoshi and
 * Mad Piano.
 */
extern int ftRemixYoshiSharedRulesIsThrowSwallower(int fkind);

/**
 * joshi_egg_timer and joshi_mash read the fighter that laid the egg: the fighter itself, or the copied power of a Kirby or
 * J Kirby. Returns TRUE when that is J Yoshi. `copy_fkind` is only read for the Kirby kinds.
 */
extern int ftRemixYoshiSharedRulesIsJapaneseEggLayer(int capture_fkind, int copy_fkind);

/** Frames before the egg breaks by itself: JYoshi.asm joshi_egg_timer. */
extern int ftRemixYoshiSharedRulesEggEscapeWaitMax(int is_japanese_layer, int vanilla_wait);

/** Base inputs needed to break the egg: JYoshi.asm joshi_mash. */
extern int ftRemixYoshiSharedRulesEggBreakoutInputsMin(int is_japanese_layer, int vanilla_inputs);

/**
 * Character.yoshi_egg rows that Remix sets for Bowser, Giga Bowser, Polygon Bowser, Dedede, Polygon Dedede and Mad Piano
 * (DK sized, Giga Bowser larger). Returns TRUE and fills the FTREMIX_YOSHI_EGG_ROW_FLOATS floats of `row` for those ids,
 * FALSE for every other id (they keep the row of the parent descriptor).
 */
extern int ftRemixYoshiSharedRulesGetEggRow(int fkind, float *row);

#endif

#endif
