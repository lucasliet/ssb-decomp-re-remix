#ifndef _SC1PREMIXHIGHSCORESRULES_H_
#define _SC1PREMIXHIGHSCORESRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the 1P record switch of the Remix (SinglePlayer.asm high_score_enabled, SinglePlayerModes.asm _ko_save and _time_save):
 * every record writer of the 1P menu modes checks the switch that the character select panel keeps, and all but the Multi-Man KO
 * save also leave the record alone while another port plays the enemy (SinglePlayerEnemy.enemy_port). Kept apart so the headless
 * unit tests (tests/remix) run them.
 */

/**
 * Whether the record of a finished run may be saved by the 1P game, Break the Targets, Board the Platforms, Bonus 3 and the Home-Run
 * Contest (extend_high_score_1p_write_, extend_high_score_btt/btp_*_write_, _time_save).
 *
 * @param high_scores_enabled Non-zero while no panel option, toggle or enemy port turns the high scores off.
 * @param enemy_port 0 when the CPU plays the enemy, otherwise the port index plus one.
 */
extern int sc1PRemixHighScoresAllowSave(int high_scores_enabled, int enemy_port);

/**
 * Whether the Multi-Man and Cruel Multi-Man KO count may be saved (_ko_save checks only the switch, not the enemy port).
 *
 * @param high_scores_enabled Non-zero while no panel option, toggle or enemy port turns the high scores off.
 */
extern int sc1PRemixHighScoresAllowKoSave(int high_scores_enabled);

#endif

#endif
