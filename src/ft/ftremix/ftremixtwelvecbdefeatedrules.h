#ifndef _FTREMIXTWELVECBDEFEATEDRULES_H_
#define _FTREMIXTWELVECBDEFEATEDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Defeated pose of the 12-Character Battle character select (TwelveCharBattle.asm defeated_action_override_ and defeated_array),
 * free of game state so the headless unit tests (tests/remix) can run it. A character with no stocks left shows its DownStandU
 * animation on the select screen: one 12 byte row per character id holds the animation file, the motion script address and the
 * animation flags.
 */

/** Size in bytes of one defeated_array row: animation file id, motion script address, animation flags. */
#define FTREMIX_TWELVECB_DEFEATED_ROW_SIZE 12

/** Highest character id with a row in defeated_array (Polygon Crash). */
#define FTREMIX_TWELVECB_DEFEATED_LAST_FKIND 0x60

/**
 * Address of the defeated_array row of a character.
 *
 * @param array_address Address of defeated_array in the Remix RAM image.
 * @param fkind Character id.
 * @return Address of the row, or 0 when the character has none.
 */
extern unsigned int ftRemixTwelveCBDefeatedRulesRowAddress(unsigned int array_address, int fkind);

/**
 * Whether the defeated pose replaces the motion of a fighter of the character select: the scene is the VS character select, the
 * fighter belongs to a port of the battle and the character it shows has no stocks left.
 *
 * @param is_vs_css Non-zero while the scene is the VS character select.
 * @param is_defeated Non-zero when the character of the port has no stocks left.
 * @param animation_file Animation file id of the row, 0 when the row is empty.
 * @return Non-zero when the pose replaces the motion.
 */
extern int ftRemixTwelveCBDefeatedRulesReplacesMotion(int is_vs_css, int is_defeated, unsigned int animation_file);

#endif

#endif
