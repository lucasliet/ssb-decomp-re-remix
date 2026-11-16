#ifndef _WPMARIOREMIXFIREBALLRULES_H_
#define _WPMARIOREMIXFIREBALLRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of Mad Piano's book (src/fireball.asm Fireball.Book.create_), so the headless unit tests (tests/remix) can
 * run them without game state. */

#define WPREMIX_PIANO_BOOK_SPREAD_STEP 0.25F

/**
 * Books one neutral special spawns. The asm loop makes a book, then repeats while its counter differs from the bonus
 * ammo, so the normal book always spawns and every bonus ammo adds one more.
 *
 * @param bonus_ammo Piano's bonus ammo truncated to an integer (0 for Kirby with the Piano power).
 * @return The book count, at least 1.
 */
extern int wpRemixPianoBookRulesCount(int bonus_ammo);

/**
 * Spread multiplier of the book made on the given loop iteration: 25% wider per iteration.
 *
 * @param iteration Loop iteration, 0 for the normal book.
 * @return The multiplier applied to the random spread.
 */
extern float wpRemixPianoBookRulesSpreadIntensity(int iteration);

#endif

#endif
