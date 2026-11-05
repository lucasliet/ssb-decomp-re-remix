#ifndef _FTREMIXJABRULES_H_
#define _FTREMIXJABRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix jab 3 and rapid jab tables (src/Character.asm: set_jab_3_timer_, get_jab_3_action_, check_jab_3_,
 * get_rapid_jab_begin/loop/ending_action_, fix_rapid_jab_unknown_, check_rapid_jab_, table setup at :2063-2144 and
 * define_character at :216-240), so the headless unit tests (tests/remix) can run them without game state.
 *
 * Every table is indexed by the fighter id and holds the address of the vanilla code path that fighter takes: a case of the
 * vanilla switch, the DISABLED default of that switch, or a Remix routine (address >= 0x80400000). The rules classify a word
 * read from the RAM image; ftremixjab.c reads the words. */

#define FTREMIX_JAB_3_GATE_ENABLED 0x8014F054u
#define FTREMIX_JAB_3_TIMER_DISABLED 0x8014EBA4u
#define FTREMIX_JAB_3_ACTION_DISABLED 0x8014EC30u
#define FTREMIX_RAPID_JAB_GATE_ENABLED 0x8014F55Cu

/* First address of the Remix code in the RAM image: a table word at or above it is a Remix routine, not a vanilla case. */
#define FTREMIX_JAB_CUSTOM_ROUTINE_MIN 0x80400000u

/* Id of nFTCommonStatusSpecialStart: the vanilla jab 3 status of every character and the rapid jab begin status of Fox, Kirby
 * and Jigglypuff (ftremixjab.c asserts it). */
#define FTREMIX_JAB_SPECIAL_STATUS_START 0xDC

/* Lanky.asm set_jab_3_timer_: `lui at, 0x4228`, stored as the followup window. */
#define FTREMIX_JAB_3_LONG_FOLLOWUP_FRAMES 42.0F

#define FTREMIX_JAB_STATUS_NONE (-1)

/** What the jab 3 timer word does to the followup window of jab 2. */
typedef enum FTRemixJabFollowup
{
    nFTRemixJabFollowupKeep,        /* DISABLED: the window left by jab 1 stays */
    nFTRemixJabFollowupDefault,     /* a vanilla case: FTCOMMON_ATTACK1_FOLLOWUP_FRAMES_DEFAULT */
    nFTRemixJabFollowupLong         /* Remix routine (Lanky): FTREMIX_JAB_3_LONG_FOLLOWUP_FRAMES */

} FTRemixJabFollowup;

/** Rapid jab flavour of a table word: the vanilla fighter whose case the word points to. */
typedef enum FTRemixJabRapidStyle
{
    nFTRemixJabRapidStyleNone,
    nFTRemixJabRapidStyleFox,
    nFTRemixJabRapidStyleLink,
    nFTRemixJabRapidStyleKirby,
    nFTRemixJabRapidStylePurin,
    nFTRemixJabRapidStyleCaptain,
    nFTRemixJabRapidStyleEnd = nFTRemixJabRapidStyleCaptain

} FTRemixJabRapidStyle;

#define FTREMIX_RAPID_JAB_STYLE_COUNT nFTRemixJabRapidStyleEnd

/** Order of the vanilla rows a rapid jab word is compared with (style - 1 indexes the array). */
extern const int dFTRemixJabRapidStyleFkinds[FTREMIX_RAPID_JAB_STYLE_COUNT];

typedef enum FTRemixJabRapidPhase
{
    nFTRemixJabRapidPhaseBegin,
    nFTRemixJabRapidPhaseLoop,
    nFTRemixJabRapidPhaseEnd

} FTRemixJabRapidPhase;

/** TRUE when the jab_3 table word enables the third jab (check_jab_3_ jumps to ENABLED). */
extern int ftRemixJabRulesIsJab3Gate(unsigned int gate_word);

/** TRUE when the rapid_jab table word enables the rapid jab (check_rapid_jab_ jumps to ENABLED). */
extern int ftRemixJabRulesIsRapidGate(unsigned int gate_word);

/**
 * Effect of the jab_3_timer word on the followup window of jab 2 (set_jab_3_timer_).
 *
 * @param timer_word jab_3_timer table word of the fighter.
 */
extern int ftRemixJabRulesFollowupKind(unsigned int timer_word);

/**
 * Status id of the third jab (get_jab_3_action_). A vanilla case gives FTREMIX_JAB_SPECIAL_STATUS_START; the Remix routines
 * of Banjo, Dedede, Bowser and Giga Bowser give the id of their own jab 3. Returns FTREMIX_JAB_STATUS_NONE for DISABLED or
 * a routine of another fighter.
 *
 * @param fkind Fighter id.
 * @param action_word jab_3_action table word of the fighter.
 */
extern int ftRemixJabRulesJab3Status(int fkind, unsigned int action_word);

/**
 * Style of a rapid jab table word: the first vanilla row (Fox, Link, Kirby, Jigglypuff, Captain) with the same word. A zero
 * word (image not loaded) and every other word, DISABLED included, give nFTRemixJabRapidStyleNone.
 *
 * @param word Table word of the fighter.
 * @param vanilla_words The words of the vanilla rows in dFTRemixJabRapidStyleFkinds order.
 */
extern int ftRemixJabRulesRapidStyle(unsigned int word, const unsigned int *vanilla_words);

/**
 * Status id of one phase of the rapid jab (get_rapid_jab_begin/loop/ending_action_). Mewtwo and N Mewtwo have their own
 * routines (0xE7, 0xE8, 0xE9); the other words are matched with the vanilla rows. Returns FTREMIX_JAB_STATUS_NONE when the
 * word is DISABLED or unknown.
 *
 * @param phase FTRemixJabRapidPhase.
 * @param fkind Fighter id.
 * @param word Table word of the phase.
 * @param vanilla_words The words of the vanilla rows of the same table in dFTRemixJabRapidStyleFkinds order.
 */
extern int ftRemixJabRulesRapidStatus(int phase, int fkind, unsigned int word, const unsigned int *vanilla_words);

/**
 * Number of A presses that start the rapid jab (fix_rapid_jab_unknown_ switch): 4 for Fox, Kirby and Jigglypuff, 5 for Link,
 * 6 for Captain. 0 for nFTRemixJabRapidStyleNone.
 *
 * @param style FTRemixJabRapidStyle of the rapid_jab_unknown word.
 */
extern int ftRemixJabRulesRapidInputsMin(int style);

/** TRUE when jab 2 of the fighter starts the rapid jab instead (Mewtwo.asm rapid_jab_patch_: Mewtwo and N Mewtwo). */
extern int ftRemixJabRulesStartsRapidJabAtJab2(int fkind);

/** TRUE when the rapid jab of the fighter restarts jab 1 (Slippy.asm unique_jab_loop_). */
extern int ftRemixJabRulesRapidJabRestartsJab1(int fkind);

#endif

#endif
