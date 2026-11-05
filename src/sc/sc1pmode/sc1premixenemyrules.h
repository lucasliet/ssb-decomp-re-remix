#ifndef _SC1PREMIXENEMYRULES_H_
#define _SC1PREMIXENEMYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Single Player Enemy option (SinglePlayerEnemy.asm) that need no game state, kept apart so the headless unit
 * tests (tests/remix) run them. A port is named by its index 0 to 3; the enemy port variable of the asm is 0 for "off, the
 * CPU plays the enemy" and the port index plus one otherwise.
 */

#define SC1PREMIX_ENEMY_PORT_COUNT 4

/** Enemy port choice made on the 1P character select: the port (0 off, 1 to 4) and whether a port was ever picked since the last reset. */
typedef struct SC1PRemixEnemySelection
{
    int enemy_port;
    int is_selected;
} SC1PRemixEnemySelection;

/**
 * Applies a Z press of the 1P character select (z_button): a port other than the playing one becomes the enemy port. A press
 * of the playing port or of nobody changes nothing.
 *
 * @param selection Enemy port choice, updated.
 * @param presser Port that pressed Z plus one, 0 when nobody did (scSubsysControllerGetPlayerTapButtons).
 * @param man_port Index of the playing port.
 * @return Non-zero on the first choice since the last reset, which plays the laugh.
 */
extern int sc1PRemixEnemySelectOnZ(SC1PRemixEnemySelection *selection, int presser, int man_port);

/**
 * Whether the B press of a port other than the player's clears the enemy port (b_button).
 *
 * @param player Index of the port whose cursor is being updated.
 * @param b_tap_mask Bit n is set when port n pressed B this frame.
 * @return Non-zero when some other port pressed B.
 */
extern int sc1PRemixEnemyIsClearedByB(int player, unsigned int b_tap_mask);

/**
 * Port that controls the first enemy of the 1P game (onep_control_port): the enemy port minus one, except in the Home-Run Contest.
 *
 * @param enemy_port Enemy port variable, 0 to 4.
 * @param mode Remix 1P mode id (SC1PREMIX_MODE_*).
 * @param is_first_enemy Non-zero for the slot of the first enemy (spawn 0x25).
 * @return Port index 0 to 3, or -1 when the CPU keeps the slot.
 */
extern int sc1PRemixEnemyControlPort(int enemy_port, int mode, int is_first_enemy);

/**
 * Port that controls the first enemy when its team respawns (onep_control_respawn): the same choice without the contest check.
 *
 * @param enemy_port Enemy port variable, 0 to 4.
 * @param is_first_enemy Non-zero for the slot of the first enemy (spawn 0x25).
 * @return Port index 0 to 3, or -1 when the CPU keeps the slot.
 */
extern int sc1PRemixEnemyRespawnPort(int enemy_port, int is_first_enemy);

/**
 * Shield colour port of a fighter (onep_control_shield): a human controlled fighter of the 1P game takes the colour of the port that
 * controls it when an enemy port is set.
 *
 * @param is_1p_game Non-zero while the scene is the 1P game.
 * @param enemy_port Enemy port variable, 0 to 4.
 * @param is_human Non-zero when the fighter is a human player.
 * @param controller_port Index of the port that controls the fighter, outside 0 to 3 when it is none of them.
 * @param fighter_port Port index of the fighter.
 * @return Port index whose colour the shield takes.
 */
extern int sc1PRemixEnemyShieldPort(int is_1p_game, int enemy_port, int is_human, int controller_port, int fighter_port);

/**
 * Next value of the blink timer of the CSS labels (enemy_routine_): one step, back to 0 after 80.
 *
 * @param timer Current timer.
 * @return Timer for this frame.
 */
extern int sc1PRemixEnemyBlinkNext(int timer);

/**
 * Whether the "Enemy Player" and "Press Z" texts are in the shown half of the blink on the frame the timer steps from timer: the
 * stepped value, before it wraps, is below 41.
 */
extern int sc1PRemixEnemyBlinkIsOn(int timer);

/** Whether "Enemy Player" and "Press Z" show: in the shown half of the blink and while no enemy port is set. */
extern int sc1PRemixEnemyTextIsVisible(int enemy_port, int is_blink_on);

/** Whether the port label of a port shows (onep_routine_ to fourp_routine_): it does while that port is the enemy port. */
extern int sc1PRemixEnemyPortLabelIsVisible(int enemy_port, int port);

#endif

#endif
