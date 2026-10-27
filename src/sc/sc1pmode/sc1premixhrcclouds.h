#ifndef _SC1PREMIXHRCCLOUDS_H_
#define _SC1PREMIXHRCCLOUDS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/objdef.h>

/*
 * Background clouds of the Home-Run Contest (HRC.asm animate_background_image_): they scroll against the Sandbag once it passes the start
 * line. The rules are in sc1premixhrcrules.h.
 */

/** Puts the clouds back at their start; called at the start of an attempt. */
extern void sc1PRemixHrcCloudsReset(void);

/**
 * Sets how fast the clouds scroll this frame.
 *
 * @param sandbag_dx X the Sandbag moved this frame.
 * @param is_past_start Whether the Sandbag is at or past the start line.
 */
extern void sc1PRemixHrcCloudsSetSpeed(float sandbag_dx, sb32 is_past_start);

/**
 * Scrolls the background image of the contest by one step; no-op outside it.
 *
 * @param wallpaper_sobj Background image.
 */
extern void sc1PRemixHrcCloudsApply(SObj *wallpaper_sobj);

#endif

#endif
