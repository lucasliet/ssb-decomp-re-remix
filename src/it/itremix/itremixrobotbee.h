#ifndef _ITREMIX_ROBOTBEE_H_
#define _ITREMIX_ROBOTBEE_H_

/*
 * Smash Remix Robot Bee item (Metallic Madness), ported from src/items/RobotBee.asm.
 * The descriptor is dITRemixRobotBeeItemDesc in itremixrobotbee.c.
 */

#include "it/itremix/itremix.h"

#define ITREMIX_ROBOTBEE_KIND 0x3B /* Item.RobotBee.id, Remix item ids 0x3A..0x3F */

/** SPAWN_ITEM of Robot Bee: spawns the bee from the Metallic Madness stage file; the parent is NULL for stage spawns. */
GObj* itRemixRobotBeeMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
