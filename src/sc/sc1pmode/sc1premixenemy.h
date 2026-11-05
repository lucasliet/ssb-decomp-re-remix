#ifndef _SC1PREMIXENEMY_H_
#define _SC1PREMIXENEMY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>
#include <sys/controller.h>

/*
 * Single Player Enemy (SinglePlayerEnemy.asm): on the 1P character select Z of another port makes that port play the first enemy
 * of the 1P game instead of the CPU, B of another port takes it back. The choice lives until the 1P menu is used again. The "afk"
 * tag automation of the asm (_afk_1p) is not ported: it needs the custom player tag names, which the port does not store.
 */

/** Enemy port variable: 0 when the CPU plays the enemy, otherwise the port index plus one. */
extern s32 sc1PRemixEnemyPortGet(void);

/** Forgets the enemy port and the "already chosen" flag (remove_flag, when an option of the 1P menu is picked). */
extern void sc1PRemixEnemyClear(void);

/**
 * Z press of the 1P character select (z_button): picks the port that pressed Z as the enemy port, playing the laugh the first time.
 *
 * @param man_port Index of the playing port, which cannot be the enemy.
 */
extern void sc1PRemixEnemyOnSelectZ(s32 man_port);

/**
 * B press of the 1P character select (b_button): takes the enemy back to the CPU when a port other than player pressed B.
 *
 * @param player Index of the port whose cursor is updated.
 */
extern void sc1PRemixEnemyOnSelectB(s32 player);

/**
 * Hands the first enemy of the 1P game to the chosen port (onep_control_port): the slot becomes a human player of that port, with
 * the colour and tag of the port. Does nothing for other slots, for the Home-Run Contest or without an enemy port. Call before the
 * slot data is copied to the fighter description.
 *
 * @param slot Player slot being spawned.
 * @param mapobj_kind Spawn kind of the slot (nMPMapObjKind1PGame*).
 */
extern void sc1PRemixEnemySetupSlot(s32 slot, s32 mapobj_kind);

/**
 * Controller of a slot spawned at the start of the 1P game (onep_control_port): the controller of the enemy port for the first
 * enemy, the given one otherwise.
 *
 * @param mapobj_kind Spawn kind of the slot (nMPMapObjKind1PGame*).
 * @param controller Controller the slot would use.
 * @return Controller the slot uses.
 */
extern SYController *sc1PRemixEnemySetupController(s32 mapobj_kind, SYController *controller);

/**
 * Controller of a respawned member of the first enemy's team (onep_control_respawn).
 *
 * @param mapobj_kind Spawn kind of the slot (nMPMapObjKind1PGame*).
 * @param controller Controller the slot would use, replaced when the enemy port takes it.
 * @return Controller the respawned fighter uses.
 */
extern SYController *sc1PRemixEnemyRespawnController(s32 mapobj_kind, SYController *controller);

/**
 * Port whose colour the shield of a fighter takes (onep_control_shield).
 *
 * @param fp Fighter that raises the shield.
 * @return Port index of the colour.
 */
extern s32 sc1PRemixEnemyShieldPlayer(FTStruct *fp);

#endif

#endif
