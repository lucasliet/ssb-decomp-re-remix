#ifndef _GRREMIXWATER_H_
#define _GRREMIXWATER_H_

/*
 * Water hazard shared by Great Bay and Twilight City (Hazards.asm great_bay_water_,
 * twilight_city_water_). The water is the horizontal plane at water_y; splash_y is the
 * height where the splash effect is drawn.
 */

#include <ssb_types.h>

#ifdef PORT

void grRemixWaterMakeGround(f32 water_y, f32 splash_y);

#endif

#endif
