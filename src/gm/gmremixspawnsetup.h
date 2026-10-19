#ifndef _GMREMIXSPAWNSETUP_H_
#define _GMREMIXSPAWNSETUP_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/* Neutral Spawns toggle: overwrites x and y of the VS spawn of a player (see gmremixspawnsetup.c). */
void gmRemixNeutralSpawnApply(s32 player, Vec3f *pos);

/* Default CPU LVL (V.S.) toggle: sets the CPU level of the four slots once per boot. */
void gmRemixDefaultCPULevelSetup(void);

#endif

#endif
