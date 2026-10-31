#ifndef _MNREMIXTIMEOUTS_H_
#define _MNREMIXTIMEOUTS_H_

#include <ssb_types.h>

/* timeouts.asm (Smash Remix): the 5 minute idle timer never returns to the title.
 * Each helper gates the vanilla `TotalTimeTics == ReturnTic` branch of one scene
 * group: it returns TRUE only when that branch must run (never in the Remix build). */
extern sb32 mnRemixTimeoutsMenuReturns(s32 total_tics, s32 return_tic);
extern sb32 mnRemixTimeoutsVSReturns(s32 total_tics, s32 return_tic);
extern sb32 mnRemixTimeoutsCSSReturns(s32 total_tics, s32 return_tic);

#endif
