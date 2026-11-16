#ifndef _FTREMIXBURYRULES_H_
#define _FTREMIXBURYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure bury timer of Smash Remix (Damage.asm bury_or_plunge_initial_ and bury_initial_), free of game globals so the
 * headless unit tests (tests/remix) can run it.
 */

/* Damage.asm: initial_buried_time 99, buried_percent_multiplier 0.7F */
#define FTREMIX_BURY_RULES_BASE_TICS 99
#define FTREMIX_BURY_RULES_PERCENT_MUL 0.7F

/**
 * Buried timer for a damage percent: 99 plus 0.7 times the percent, rounded to nearest (ties to even) as `cvt.w.s` does
 * under the libultra FCSR rounding mode.
 *
 * @param percent Damage percent of the buried fighter.
 * @return Tics the fighter stays buried before the mash inputs shorten it.
 */
int ftRemixBuryRulesTimer(int percent);

#endif

#endif
