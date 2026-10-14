#ifndef _GRREMIXPIRATECANNON_H_
#define _GRREMIXPIRATECANNON_H_

/*
 * Pirate Land cannon (Hazards.asm pirate_land_setup, pirate_land_main_, cannon_main_). The cannon turns its barrel
 * between three positions and fires a cannonball from the barrel. Water physics for the stage is in grremixwater.h.
 */

#include <ssb_types.h>

#ifdef PORT

/** Creates the Pirate Land cannon, its barrel and the water object. Called from grRemixHazardPirate. */
void grRemixPirateMakeGround(void);

#endif

#endif
