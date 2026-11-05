#ifndef _MNREMIXENEMYLABEL_H_
#define _MNREMIXENEMYLABEL_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Labels of the Single Player Enemy option on the 1P character select (SinglePlayerEnemy.asm setup_, enemy_routine_ and
 * onep_routine_ to fourp_routine_): the blinking "Enemy Player" and "Press Z" texts while the CPU plays the enemy, and the 1P to 4P
 * text sprite of the chosen port in the same place afterwards.
 */

/**
 * Makes the four port sprites (hidden until a port is chosen) and restarts the blink. Call once when the character select is built.
 *
 * @param common_file Loaded llMNPlayersCommonFileID file (sMNPlayers1PGameFiles[0]).
 */
extern void mnRemixEnemyLabelMake(void *common_file);

/** Steps the blink and shows the texts or the sprite of the chosen port; call once per frame of the character select. */
extern void mnRemixEnemyLabelUpdate(void);

#endif

#endif
