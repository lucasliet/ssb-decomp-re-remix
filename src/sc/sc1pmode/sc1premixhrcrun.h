#ifndef _SC1PREMIXHRCRUN_H_
#define _SC1PREMIXHRCRUN_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Per-frame run of the Home-Run Contest (HRC.asm main_): the camera that follows the Sandbag, the distance counter and the end of
 * the attempt with COMPLETE or FAILURE.
 */

/** Starts a new attempt: distance and warp to zero, the camera back at its start. No-op outside the contest. */
extern void sc1PRemixHrcRunStart(void);

/** Runs one frame of the contest; no-op outside it. */
extern void sc1PRemixHrcRunUpdate(void);

/** Distance of the attempt in tenths of a foot. */
extern s32 sc1PRemixHrcRunGetDistance(void);

#endif

#endif
