#ifndef _SC1PTRAININGREMIXTEXTLAYERS_H_
#define _SC1PTRAININGREMIXTEXTLAYERS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Text layers of the Remix Training (docs/remix/text_layers.md): the Remix strings of Training.asm and Menu.asm drawn by the game
 * in the rooms of the asm instead of the overlay. Room 0x17 (the interface link) holds the reset count, the legends, the exit hint
 * and the Z menu rows, over the vanilla pause menu; room 0x39 holds the Show Action rows under the Z menu panel and the pause menu.
 */

/** Makes the camera of room 0x39 and the two layers. Call once at the end of the Training setup, after the Z menu objects exist. */
extern void sc1PTrainingRemixTextLayersMake(void);

#endif

#endif
