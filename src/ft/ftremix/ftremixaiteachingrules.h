#ifndef _FTREMIXAITEACHINGRULES_H_
#define _FTREMIXAITEACHINGRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure teching and Z-cancel rules of the Remix AI (AI.asm random_teching_, z_cancel_), so the headless unit tests
 * (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_TECH_LEVEL_ADVANCED 10
#define FTREMIX_AI_TECH_ROLL_MAX 100
#define FTREMIX_AI_TECH_CHANCE_FORWARD 30
#define FTREMIX_AI_TECH_CHANCE_BACKWARD 30
#define FTREMIX_AI_TECH_CHANCE_IN_PLACE 30
#define FTREMIX_AI_TECH_OUTCOME_FORWARD 0
#define FTREMIX_AI_TECH_OUTCOME_BACKWARD 1
#define FTREMIX_AI_TECH_OUTCOME_IN_PLACE 2
#define FTREMIX_AI_TECH_OUTCOME_FAIL 3
#define FTREMIX_AI_ZCANCEL_CHANCE 95

/* Training.entry_tech_behavior values above Random (string_table_tech_options). */
#define FTREMIX_AI_TECH_MENU_BACKWARD 1
#define FTREMIX_AI_TECH_MENU_FORWARD 2
#define FTREMIX_AI_TECH_MENU_IN_PLACE 3
#define FTREMIX_AI_TECH_MENU_LEFT 4
#define FTREMIX_AI_TECH_MENU_RIGHT 5
#define FTREMIX_AI_TECH_MENU_NONE 6

/**
 * TRUE when a CPU uses the improved random tech roll instead of the vanilla tech checks (AI.asm random_teching_).
 * Training always uses it. Level 10 CPUs and Remix 1P use it on every screen except the excluded ones. Other CPUs
 * need the improved AI toggle and not vanilla 1P/RTTF.
 */
extern int ftRemixAiTechUsesImprovedRoll(int is_training, int is_advanced_cpu, int is_classic, int improved_ai,
                                         int is_excluded_screen);

/**
 * TRUE when the Single Button Mode allows a CPU to tech: off (0), 'R' (3) or 'R+C' (6). A, B, A+C and B+C block it.
 */
extern int ftRemixAiTechSingleButtonAllows(int single_button_mode);

/**
 * Maps a roll 0 to FTREMIX_AI_TECH_ROLL_MAX - 1 to a tech outcome. Off stage only the in-place roll and the fail
 * remain, forward and backward are never chosen.
 */
extern int ftRemixAiTechRollOutcome(int roll, int is_off_stage);

/**
 * TRUE when the CPU may roll for a Z-cancel: the improved path applies, the screen is not excluded, the Single Button
 * Mode allows it, the fighter is a CPU and it is not vanilla 1P/RTTF (AI.asm z_cancel_).
 */
extern int ftRemixAiZCancelGateOpen(int improved_path, int is_excluded_screen, int single_button_allows, int is_cpu,
                                    int is_classic);

/** TRUE when a Z-cancel roll succeeds: roll below FTREMIX_AI_ZCANCEL_CHANCE. */
extern int ftRemixAiZCancelRollSucceeds(int roll);

#endif

#endif
