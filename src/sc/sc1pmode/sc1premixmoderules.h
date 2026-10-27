#ifndef _SC1PREMIXMODERULES_H_
#define _SC1PREMIXMODERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Remix 1P modes that need no game state (SinglePlayerModes.asm bonus2_css_2_1p), kept apart so the headless
 * unit tests (tests/remix) run them.
 */

/**
 * Whether the Bonus 2 CSS of a Remix 1P mode hands over to the 1P game instead of the bonus stage: Bonus 3, Multi-Man, Cruel
 * Multi-Man and the Home-Run Contest. Remix 1P and All-Star use the 1P CSS and never reach the Bonus 2 CSS; the standard
 * bonus stages keep their own flow.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @return Non-zero when the Bonus 2 CSS starts the 1P game.
 */
extern int sc1PRemixModeLeavesBonusCSSFor1P(int mode);

/**
 * Times the RNG is stepped when a mode leaves the Bonus 2 CSS, so the polygon fighters of the first match differ between runs:
 * one to 64 steps by the frame count of the screen.
 *
 * @param frame_count Frames the CSS ran.
 * @return Steps to run, 1 to 64.
 */
extern unsigned int sc1PRemixModeSeedAdvanceCount(unsigned int frame_count);

/** KOs of a Multi-Man run: the enemy KOs the player scored, which stop counting once the player was KO'd (multiman_ko_count_stop). */
typedef struct SC1PRemixMultimanKos
{
    int kos;
    int is_player_ko;
} SC1PRemixMultimanKos;

/**
 * Whether the mode is one of the horde modes (Multi-Man and Cruel Multi-Man), which fight endless Fighting Polygons in the
 * Duel Zone with one stock, no items and no team attack.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 */
extern int sc1PRemixModeIsMultiman(int mode);

/**
 * Knockback ratio (the "handicap" of the 1P setup) of the polygons of a horde mode: easy for Multi-Man, very hard for Cruel.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @return The ratio, or 0 when the mode keeps the stage value.
 */
extern int sc1PRemixModeEnemyHandicap(int mode);

/**
 * Knockback ratio the player takes: raised in Cruel Multi-Man only.
 *
 * @param mode Mode id (SC1PREMIX_MODE_*).
 * @return The ratio, or 0 when the mode keeps the default.
 */
extern int sc1PRemixModeHumanHandicap(int mode);

/**
 * Whether a horde run that ended starts over: the reset button restarts the run, any other end (the player KO'd) goes back to
 * the character select (_end_match_screen).
 *
 * @param is_reset Non-zero when the run ended by the reset button.
 */
extern int sc1PRemixMultimanRestartsRun(int is_reset);

/** Starts a run with no KOs and the player alive. */
extern void sc1PRemixMultimanKosReset(SC1PRemixMultimanKos *counter);

/** Counts one enemy KO unless the player was already KO'd. */
extern void sc1PRemixMultimanKosAddEnemy(SC1PRemixMultimanKos *counter);

/** Marks the player as KO'd, so later enemy KOs are not counted. */
extern void sc1PRemixMultimanKosSetPlayerKo(SC1PRemixMultimanKos *counter);

#endif

#endif
