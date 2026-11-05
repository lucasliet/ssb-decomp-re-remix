#ifndef _SC1PREMIXALLSTARREST_H_
#define _SC1PREMIXALLSTARREST_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Series icons of the All-Star Rest Area (SinglePlayerModes.asm rest_area_setup, _icons): the fighters still to face, as small icons
 * at the top left of the screen. The portraits of the next opponents drawn in the room (_portrait) are not made yet.
 */

/** Makes the icons of the fighters the run has left; no-op outside the Rest Area or without the icon table of the Remix. */
extern void sc1PRemixAllstarRestMakeIcons(void);

#endif

#endif
