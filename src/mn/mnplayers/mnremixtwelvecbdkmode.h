#ifndef _MNREMIXTWELVECBDKMODE_H_
#define _MNREMIXTWELVECBDKMODE_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * DK Mode and Lanky Mode easter egg of the 12-Character Battle character select (DKMode.asm _check_portraits): ties the pattern check
 * of mnremixtwelvecbdkmoderules.c to the Custom sets and to the head scale of ftremixdkmode.c.
 */

/** Reads the Custom sets of both players and switches DK Mode or Lanky Mode on or off. Call it after every change of a Custom set. */
extern void mnRemixTwelveCBDKModeUpdate(void);

#endif

#endif
