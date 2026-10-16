#ifndef _GRREMIXDREAMLAND_H_
#define _GRREMIXDREAMLAND_H_

/*
 * Whispy Woods wind and downtime of Dream Land under the Remix Whispy Mode toggle (Hazards.asm
 * dreamland_downtime_initially_, dreamland_downtime_, dreamland_wind_speed_multiplier_, dreamland_wind_speed_max_).
 * The vanilla functions in gr/grcommon/grpupupu.c call these under #ifdef REGION_REMIX.
 */

#include <ssb_types.h>

/** Frames before the first blow, given the vanilla random wait. Hyper Mode replaces it with a fixed 256 frames. */
s32 grRemixDreamLandWhispyInitialWait(s32 random_wait);

/** Frames between two blows, given the vanilla random wait. Hyper Mode replaces it with a fixed 37 frames. */
s32 grRemixDreamLandWhispyStopWait(s32 random_wait);

/** Wind push base velocity: the vanilla 6.0 in Normal and Japanese modes, 30.0 otherwise. */
f32 grRemixDreamLandWindSpeedMax(void);

/** Wind push decay per unit of distance to Whispy: picks the Japanese, Super or Normal value. */
f32 grRemixDreamLandWindDistDecay(void);

#endif
