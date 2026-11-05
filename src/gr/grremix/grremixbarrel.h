#ifndef _GRREMIXBARREL_H_
#define _GRREMIXBARREL_H_

/*
 * Barrel obstacle slots of the Remix stages (Hazards.asm many_barrels_fix_). The fighter code keeps the barrels in a
 * ground obstacle array of 2 entries; Banjo's Board the Platforms swaps it for an array of TOTAL_BARRELS entries.
 */

#include <ssb_types.h>

#define GRREMIX_BARREL_OBSTACLES_VANILLA 2
#define GRREMIX_BARREL_OBSTACLES_BANJO 12

/** Number of ground obstacle slots the current stage may fill: 12 on Board the Platforms (Banjo), 2 elsewhere. */
s32 grRemixBarrelGetObstacleCapacity(void);

#endif
