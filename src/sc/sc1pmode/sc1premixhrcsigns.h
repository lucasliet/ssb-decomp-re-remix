#ifndef _SC1PREMIXHRCSIGNS_H_
#define _SC1PREMIXHRCSIGNS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Distance signs of the Home-Run Contest stage: the digit images of the stage's sign objects (HRC.asm initialize_signs_ and the sign
 * update of main_). The digit rules are in sc1premixhrcsignrules.h.
 */

/** Sets the second, third and fourth signs to 100, 200 and 300 ft and stops their digit animation; run once the stage is built. */
extern void sc1PRemixHrcSignsInit(void);

/** Moves the fourth sign 100 ft further; called each time the camera warps back by that distance. */
extern void sc1PRemixHrcSignsAdvance(void);

#endif

#endif
