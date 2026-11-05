#ifndef _SC1PTRAININGREMIXSPAWN_H_
#define _SC1PTRAININGREMIXSPAWN_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fttypes.h>
#include <ssb_types.h>

/*
 * Custom spawn of the Remix Training Z menu (Training.asm struct.port_N spawn_pos, spawn_dir and clipping_id, set_custom_spawn_ at
 * 1380, init_character_ at 426-441, set_action_ at 468-503, Spawn.asm load_spawn_ at 97-134). "Set Custom Spawn" saves where a
 * fighter stands, which way it faces and the ledge it hangs from; the Spawn entry set to Custom starts the port there.
 */

/**
 * Training.asm set_custom_spawn_: saves the position, facing and ledge of the fighter of `port` and makes the port's spawn Custom.
 * Does nothing when the port has no fighter (set_custom_spawn macro, "skip if no player struct is returned").
 *
 * @param port Port 0 to 3.
 */
extern void sc1PTrainingRemixSpawnSave(s32 port);

/** TRUE when the Spawn entry of `port` is Custom. */
extern sb32 sc1PTrainingRemixSpawnIsCustom(s32 port);

/** Takes the port off Custom (the Spawn entry was set to Auto or to a port). The saved spawn is kept like the asm struct keeps it. */
extern void sc1PTrainingRemixSpawnClear(s32 port);

/** Makes the port's spawn Custom again from the spawn saved before (the Spawn entry was set to Custom). */
extern void sc1PTrainingRemixSpawnSelect(s32 port);

/**
 * Spawn.asm load_spawn_ and init_character_: puts the saved position and facing in the fighter description of a Custom port.
 *
 * @return TRUE when `desc` was changed.
 */
extern sb32 sc1PTrainingRemixSpawnApplyDesc(s32 port, FTDesc *desc);

/** Training.asm set_action_: a Custom port saved on a ledge starts hanging from it instead of standing. Call after the fighter is made. */
extern void sc1PTrainingRemixSpawnEnterLedge(s32 port, GObj *fighter_gobj);

#endif

#endif
