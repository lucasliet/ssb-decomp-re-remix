#ifndef _FTREMIXAITABLESRULES_H_
#define _FTREMIXAITABLESRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix CPU attack tables (AI.asm add_attack_behaviour), so the headless unit tests (tests/remix) can run them.
 * Only int and float: the walk works on u32 words of the RAM image, one entry is 7 words (input, start, end, four f32 detect bounds).
 */

#define FTREMIX_AI_TABLES_WORDS_PER_ENTRY 7
#define FTREMIX_AI_TABLES_TERMINATOR_INPUT 0xFFFFFFFFu
#define FTREMIX_AI_TABLES_TERMINATORS_PER_TABLE 2
#define FTREMIX_AI_TABLES_FIRST_CUSTOM_INPUT 0x31
#define FTREMIX_AI_TABLES_CHARACTER_COUNT 97
#define FTREMIX_AI_TABLES_LV10_LEVEL 10

/** TRUE for the END_ATTACKS input word (0xFFFFFFFF), which closes the grounded and then the aerial group. */
extern int ftRemixAiTablesRulesIsTerminator(unsigned int input_word);

/** TRUE for a custom CPU input routine id (AI.NUM_VANILLA_CPU_INPUT_ROUTINES = 0x31 and up). */
extern int ftRemixAiTablesRulesIsCustomInput(int input_kind);

/**
 * Number of entries of a table walked from its first entry up to and including the second terminator.
 * words holds entry_capacity entries of FTREMIX_AI_TABLES_WORDS_PER_ENTRY words each.
 * Returns -1 when the second terminator is not inside the capacity.
 */
extern int ftRemixAiTablesRulesCountEntries(const unsigned int *words, int entry_capacity);

/** Bit-cast of a word of the image to the f32 it holds (memcpy, not a numeric conversion). */
extern float ftRemixAiTablesRulesBitsToFloat(unsigned int bits);

/** TRUE for the CPU level that selects the lv10 attack table (Character.lv10_ai_behaviour.table). */
extern int ftRemixAiTablesRulesIsLv10Level(int level);

/** TRUE when fkind can index Character.ai_behaviour.table (one pointer per Character.id). */
extern int ftRemixAiTablesRulesIsTableIndex(int fkind);

#endif

#endif
