#ifndef _FTREMIXAICPUINPUTRULES_H_
#define _FTREMIXAICPUINPUTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the CPU-only inputs that the Remix AI adds inside shared fighter statuses (AI.asm auto_jab_grab_,
 * GBowser/gbowser.asm prevent_dash), so the headless unit tests (tests/remix) can run them without game state. */

#define FTREMIX_AI_JAB_GRAB_LEVEL 10
#define FTREMIX_AI_JAB_GRAB_MIN_STATUS_TICS 3

/**
 * TRUE when a jab (Attack11) interrupt grabs without the Z tap: the fighter is a CPU of level 10 or more, the jab has run
 * for at least 3 frames (status_total_tics) and Single Button Mode allows it (off, R or R+C), as in AI.asm auto_jab_grab_.
 * Every other fighter keeps the Z tap requirement.
 */
extern int ftRemixAiJabGrabRulesApplies(int is_cpu, int level, int status_total_tics, int single_button_mode);

/** TRUE when the dash of this fighter is replaced by the fast walk: a CPU Giga Bowser (gbowser.asm prevent_dash). */
extern int ftRemixAiDashRulesUsesWalk(int is_giga_bowser, int is_cpu);

#endif

#endif
