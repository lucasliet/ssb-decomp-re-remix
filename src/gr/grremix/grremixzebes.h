#ifndef _GRREMIXZEBES_H_
#define _GRREMIXZEBES_H_

/*
 * Lava level overrides of the Remix Zebes-based stages (Hazards.asm alt_acid_levels_). The levels are
 * written over dGRZebesAcidAttributes[].acid_level by grZebesMakeAcid under #ifdef REGION_REMIX.
 */

#include <ssb_types.h>

#define GRREMIX_ZEBES_ACID_LEVELS_NUM 16

/** Lava levels of the stage (GRREMIX_ZEBES_ACID_LEVELS_NUM entries), or NULL when the stage keeps the vanilla table. */
const f32 *grRemixGetAltAcidLevels(s32 gkind);

#endif
