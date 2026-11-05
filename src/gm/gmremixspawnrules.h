#ifndef _GMREMIXSPAWNRULES_H_
#define _GMREMIXSPAWNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of Spawn.asm load_spawn_ (which table and which row spawn a player), so the headless unit tests (tests/remix) can run
 * them. Ints only. Both tables (original_table and neutral_table) hold one row of 0x20 bytes per stage id, 8 bytes (x, y floats)
 * per spawn slot.
 */

/** Spawn.asm Stages.MAX_STAGE_ID: the last stage id with a row in the tables. */
#define GMREMIX_SPAWN_RULES_STAGE_MAX 0xD7

/** Spawn.asm: bytes of one stage row (four slots). */
#define GMREMIX_SPAWN_RULES_ROW_SIZE 0x20

/** Spawn.asm: bytes of one slot (x and y floats). */
#define GMREMIX_SPAWN_RULES_SLOT_SIZE 8

/** Spawn slots per stage row. */
#define GMREMIX_SPAWN_RULES_SLOTS_NUM 4

/**
 * Whether a VS player takes the neutral table (_load_neutral) instead of the original one (_load_original): the Neutral Spawns
 * toggle is on and the match has a valid team or 1v1 setup that gives the player a slot (_toggle_off and a setup that is not
 * found both load the original spawn). Smashketball always takes the neutral table and is decided by the caller.
 *
 * @param toggle_on 1 when the Neutral Spawns toggle is on.
 * @param neutral_slot Neutral slot of the player from the valid_teams / valid_singles tables, or -1 when none applies.
 * @return 1 for the neutral table, 0 for the original table.
 */
extern int gmRemixSpawnRulesVsUsesNeutral(int toggle_on, int neutral_slot);

/**
 * Slot (row of the stage) a VS player spawns at: the neutral slot of the setup for the neutral table, the player index for the
 * original table (`sll t1, a0, 3` with a0 untouched).
 *
 * @param player Player index, 0 to 3.
 * @param uses_neutral Result of gmRemixSpawnRulesVsUsesNeutral.
 * @param neutral_slot Neutral slot of the player.
 * @return Slot, 0 to 3.
 */
extern int gmRemixSpawnRulesVsSlot(int player, int uses_neutral, int neutral_slot);

/**
 * Slot a Training port spawns at (load_spawn_ training branch): spawn ids 0 to 3 (Port 1 to 4) read that slot of the original
 * table of the Training stage; any other id (4 = Custom) uses the stored position of the port instead of a table.
 *
 * @param spawn_id Spawn id of the port.
 * @return Slot 0 to 3, or -1 for the custom position.
 */
extern int gmRemixSpawnRulesTrainingSlot(int spawn_id);

/**
 * Byte offset of a slot in a spawn table: stage_id * 0x20 + slot * 8.
 *
 * @param stage_id Stage id, 0 to GMREMIX_SPAWN_RULES_STAGE_MAX.
 * @param slot Slot, 0 to 3.
 * @return Offset, or -1 when the stage or the slot is out of range.
 */
extern int gmRemixSpawnRulesTableOffset(int stage_id, int slot);

#endif

#endif
