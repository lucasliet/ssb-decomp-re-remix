#ifndef _GRREMIXZEBES_H_
#define _GRREMIXZEBES_H_

/*
 * Lava level overrides of the Remix Zebes-based stages (Hazards.asm alt_acid_levels_). The levels are
 * written over dGRZebesAcidAttributes[].acid_level by grZebesMakeAcid under #ifdef REGION_REMIX, and the vanilla
 * levels are written back on every other Zebes-based stage. Plain C types, so the headless tests can run it.
 */

#include <stddef.h>
#include <stdint.h>

#define GRREMIX_ZEBES_ACID_LEVELS_NUM 16

/** Lava levels of the stage (GRREMIX_ZEBES_ACID_LEVELS_NUM entries), or NULL when the stage keeps the vanilla table. */
const float *grRemixGetAltAcidLevels(int32_t gkind);

/**
 * Lava levels a Zebes-based stage starts with: its Remix table, else the vanilla levels (on N64 the table is overlay data
 * reloaded for every match, so a Remix table never outlives its match).
 *
 * @param gkind Stage id.
 * @param vanilla_levels The vanilla levels (GRREMIX_ZEBES_ACID_LEVELS_NUM entries).
 * @return The levels to write into the lava table.
 */
const float *grRemixZebesSelectAcidLevels(int32_t gkind, const float *vanilla_levels);

#endif
