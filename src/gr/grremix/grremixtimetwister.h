#ifndef _GRREMIXTIMETWISTER_H_
#define _GRREMIXTIMETWISTER_H_

/*
 * Time Twister background fade (Hazards.asm time_twister_setup_, time_twister_bg_fade_): a full screen fade
 * rectangle that runs the warp transition when the warp ball appears.
 */

#include <ssb_types.h>

#ifdef PORT

void grRemixTimeTwisterSetup(void);

#endif

#endif
