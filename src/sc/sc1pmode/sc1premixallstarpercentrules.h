#ifndef _SC1PREMIXALLSTARPERCENTRULES_H_
#define _SC1PREMIXALLSTARPERCENTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the percent the All-Star run carries between matches and of the afk exit of the Rest Area (SinglePlayerModes.asm
 * player_percent, record_percent, rest_area_routine), kept apart so the headless unit tests (tests/remix) run them. A port is named
 * by its index 0 to 3; the enemy port variable of the Single Player Enemy option is 0 for "off" and the port index plus one otherwise.
 */

/** Frames the Rest Area lasts for the "afk" automation before it walks out regardless of where the fighter stands (rest_area_routine). */
#define SC1PREMIX_ALLSTAR_AFK_REST_FRAMES 120

/**
 * Port the Single Player Enemy option hands to the enemy human (player_percent): the port after the playing one, port 1 after port 4.
 *
 * @param man_port Index of the playing port.
 * @return Port index 0 to 3.
 */
extern int sc1PRemixAllstarEnemyPort(int man_port);

/**
 * Whether a human fighter starts the match with the carried percent (player_percent): the enemy human of the Single Player Enemy
 * option starts at zero, everyone else gets the percent.
 *
 * @param enemy_port Enemy port variable, 0 to 4.
 * @param man_port Index of the playing port.
 * @param port Index of the port of the fighter being made.
 * @return Non-zero when the fighter takes the carried percent.
 */
extern int sc1PRemixAllstarTakesPercent(int enemy_port, int man_port, int port);

/**
 * Percent kept at the end of a match (record_percent): the fighter's percent, or zero while the run is in limbo.
 *
 * @param percent Percent of the human fighter.
 * @param is_limbo Non-zero after the human lost its last stock and until the next fighter is made.
 * @return The percent the next match starts with.
 */
extern int sc1PRemixAllstarRecordedPercent(int percent, int is_limbo);

/**
 * Whether the Rest Area is left this frame (rest_area_routine): the fighter has to be on the ground, and then it either stands on
 * the exit clipping or, with the "afk" automation on, the Rest Area has lasted SC1PREMIX_ALLSTAR_AFK_REST_FRAMES frames.
 *
 * @param is_grounded Non-zero while the fighter stands on the ground.
 * @param is_on_exit Non-zero while the surface under the fighter is the exit clipping.
 * @param is_afk Non-zero while the "afk" automation is on.
 * @param frames_elapsed Frames since the Rest Area began.
 * @return Non-zero when the run leaves the Rest Area.
 */
extern int sc1PRemixAllstarRestIsLeft(int is_grounded, int is_on_exit, int is_afk, unsigned int frames_elapsed);

#endif

#endif
