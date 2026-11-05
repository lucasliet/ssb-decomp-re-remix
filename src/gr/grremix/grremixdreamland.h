#ifndef _GRREMIXDREAMLAND_H_
#define _GRREMIXDREAMLAND_H_

/*
 * Whispy Woods wind and downtime of Dream Land under the Remix Whispy Mode toggle (Hazards.asm
 * dreamland_downtime_initially_, dreamland_downtime_, dreamland_wind_speed_multiplier_, dreamland_wind_speed_max_).
 * The vanilla functions in gr/grcommon/grpupupu.c call these under #ifdef REGION_REMIX.
 */

#include <ssb_types.h>

/** Whispy wind box of Dream Land: the area where fighters are pushed and the x the wind blows from. */
typedef struct GRRemixDreamLandWindBox
{
    f32 edge_left;
    f32 edge_right;
    f32 top;
    f32 bottom;
    f32 origin_x;

} GRRemixDreamLandWindBox;

/** Frames before the first blow, given the vanilla random wait. Hyper Mode replaces it with a fixed 256 frames. */
s32 grRemixDreamLandWhispyInitialWait(s32 random_wait);

/** Frames between two blows, given the vanilla random wait. Hyper Mode replaces it with a fixed 37 frames. */
s32 grRemixDreamLandWhispyStopWait(s32 random_wait);

/** Wind push base velocity: the vanilla 6.0 in Normal and Japanese modes, 30.0 otherwise. */
f32 grRemixDreamLandWindSpeedMax(void);

/** Wind push decay per unit of distance to Whispy: picks the Japanese, Super or Normal value. */
f32 grRemixDreamLandWindDistDecay(void);

/**
 * Fills the wind box used by grPupupuWhispySetWindPush (Hazards.asm dream_land_wind_parameters). Dream Greens uses its
 * own box, whatever the Whispy Mode. Elsewhere, Super and Hyper Mode scale the left and right edges and the top of the
 * vanilla box by 1.09375 and 1.1953125, except on the 1P game, which keeps the vanilla box.
 */
void grRemixDreamLandGetWindBox(GRRemixDreamLandWindBox *wind_box);

#endif
