#ifndef _GMREMIXSPAWNSETUP_H_
#define _GMREMIXSPAWNSETUP_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/* VS spawn of a player: x and y from Spawn.neutral_table (Neutral Spawns on, valid setup) or Spawn.original_table (see gmremixspawnsetup.c). */
void gmRemixNeutralSpawnApply(s32 player, Vec3f *pos);

/* Training spawn of a port for spawn ids 0 to 3: x and y from Spawn.original_table of the Training stage. */
void gmRemixTrainingSpawnApply(s32 spawn_id, Vec3f *pos);

/* Default CPU LVL (V.S.) toggle: sets the CPU level of the four slots once per boot. */
void gmRemixDefaultCPULevelSetup(void);

/* A press on Default CPU LVL (V.S.) (Toggles.asm set_cpu_level): writes a non-zero toggle as the CPU level of the four slots. */
void gmRemixDefaultCPULevelRefresh(void);

#endif

#endif
