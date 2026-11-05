#ifndef _GRREMIXSTAGERULES_H_
#define _GRREMIXSTAGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure stage class rules of Stages.asm (id_fix_5_ and id_fix_6_), so the headless unit tests (tests/remix) can run them.
 * The callers read the Stages.class_table and Stages.function_table entries and pass them in.
 */

/* Stages.asm id_fix_5_ and id_fix_6_ `slti ... 0x0011`: ids below this are battle stages without a table lookup. */
#define GRREMIX_STAGERULES_FIRST_NON_BATTLE_ID 0x11

/* Stages.class.BATTLE */
#define GRREMIX_STAGERULES_CLASS_BATTLE 0

/**
 * Tells whether the end of a match on the stage shows GAME SET (id_fix_5_, RAM 0x80114C9C). Ids below 0x11 are battles; HRC,
 * BTT_CRASH and BTP_CRASH show FAILURE; any other id is a battle when its Stages.function_table entry is not 0 (0 = bonus stage).
 *
 * @param gkind Stage id.
 * @param ground_function Stages.function_table entry of the stage.
 * @return 1 for GAME SET, 0 for FAILURE.
 */
extern int grRemixStageRulesEndsWithGameSet(int gkind, unsigned int ground_function);

/**
 * Tells whether a fighter KO on the stage plays the small explosion SFX (id_fix_6_, RAM 0x8013C2BC). Ids below 0x11 play the large
 * one; any other id plays the small one when its Stages.class_table entry is not BATTLE.
 *
 * @param gkind Stage id.
 * @param stage_class Stages.class_table entry of the stage.
 * @return 1 for the small explosion, 0 for the large one.
 */
extern int grRemixStageRulesHasSmallExplosion(int gkind, int stage_class);

#endif
#endif
