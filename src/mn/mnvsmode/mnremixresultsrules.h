#ifndef _MNREMIXRESULTSRULES_H_
#define _MNREMIXRESULTSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Smash Remix VS results winner screen (src/resultsscreen.asm) that need no game state, kept apart so the headless
 * unit tests (tests/remix) run them. Only int, float and char arguments, no port or libultraship include.
 */

/* First fkind whose winner data comes from the Remix tables (Character.*.table): Master Hand, Metal Mario, the polygons, Giant DK
 * and every Remix fighter. Fkinds below it are the 12 playable fighters, which keep the vanilla tables of the results screen. */
#define MNREMIX_RESULTS_FIRST_EXTENDED_FKIND 12

/* Joypad.CR / CL / CU / CD: the C buttons of the held mask (victory_animations_ checks the low nibble) */
#define MNREMIX_RESULTS_C_BUTTONS_MASK 0x000F
#define MNREMIX_RESULTS_C_RIGHT 0x0001
#define MNREMIX_RESULTS_C_LEFT 0x0002

/* add_ampersand_: glyph index of "&", its sprite offset in the announce letters file (0x8348 + 0x10) and its width (0x42080000) */
#define MNREMIX_RESULTS_AMPERSAND_INDEX 0x1D
#define MNREMIX_RESULTS_AMPERSAND_SPRITE_OFFSET 0x8358
#define MNREMIX_RESULTS_AMPERSAND_WIDTH 34.0F

/* Longest winner name copied out of the Remix image, with the terminator ("BAN1JO2&2KAZOOI1E" is 18 characters) */
#define MNREMIX_RESULTS_NAME_BUFFER_SIZE 32

/** Reads one byte of the Remix image, 0 outside it. */
typedef unsigned int (*MNRemixResultsReadByteFunc)(unsigned int ram_addr);

/**
 * Whether the winner data of a fighter kind comes from the Remix tables instead of the 12-entry vanilla tables.
 *
 * @param fkind Fighter kind of the winner.
 * @return Non-zero for fkind 12 and above.
 */
extern int mnRemixResultsRulesIsExtendedFkind(int fkind);

/**
 * Victory animation index of the winner (victory_animations_, pick_victory_animation_). Holding a C button on the controller of
 * the winning port overrides the random pick: C-Right gives 2 (victory animation 3), C-Left gives 1 (animation 2), C-Up or
 * C-Down gives 0 (animation 1). Without a C button the random pick stays.
 *
 * @param random_index Index drawn by the vanilla code.
 * @param held_buttons Held button mask of the winning port.
 * @return Index into the three victory statuses.
 */
extern int mnRemixResultsRulesPickVictoryIndex(int random_index, unsigned int held_buttons);

/**
 * Whether the winner line reads "WIN!" instead of "WINS!" (get_str_wins_lx_): the fighters with the Japanese sound type and Banjo
 * say "WIN!", Dragon King keeps "WINS!" whatever its sound type.
 *
 * @param fkind Fighter kind of the winner.
 * @param is_japanese_sound Non-zero when Character.sound_type.table holds the J sound type for the fighter.
 * @return Non-zero when the singular text is used.
 */
extern int mnRemixResultsRulesUsesSingularWin(int fkind, int is_japanese_sound);

/**
 * Float stored in a Remix table word (the asm macro `float32` writes the IEEE bits).
 *
 * @param bits The 32 bit word.
 * @return The float with those bits.
 */
extern float mnRemixResultsRulesBitsToFloat(unsigned int bits);

/**
 * Copies a zero terminated string of the Remix image (the winner name of str_winner_ptr.table).
 *
 * @param read_byte Reader of one image byte.
 * @param ram_addr Remix RAM address of the first character.
 * @param buffer Destination, always terminated.
 * @param buffer_size Size of the destination in bytes, 1 or more.
 * @return Number of characters copied; 0 for an empty string or an unusable argument.
 */
extern int mnRemixResultsRulesCopyName(MNRemixResultsReadByteFunc read_byte, unsigned int ram_addr, char *buffer, int buffer_size);

#endif

#endif
