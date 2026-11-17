#ifndef _IFREMIXINPUTDISPLAY_H_
#define _IFREMIXINPUTDISPLAY_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Input Display of a Remix match (InputDisplay.asm): for every port whose Input Display panel value is Top or Bottom, a block of
 * button sprites (L, R, A, B, Z, the C buttons, the D-pad, the joystick and its gate) next to the series logo of the port's
 * percent HUD. The sprites are game objects of group 0x0B in display link 23 (room 0x17), made once the HUD of the match exists, so
 * the vanilla pause and Disable HUD = All hide them with the HUD and the match objects made later draw over them.
 */

/**
 * Makes the blocks of the ports that show one. Call once after the percent HUD of the match is made.
 */
extern void ifRemixInputDisplayMake(void);

/** Forgets the blocks of the previous scene; their SObjs are freed with it. Called at every scene change. */
extern void ifRemixInputDisplayClear(void);

/**
 * Lights the sprites of a port for the sample its fighter reads this frame (update_input_display_). Does nothing for a port
 * with no block.
 *
 * @param player Port index.
 * @param buttons Pressed buttons, libultra CONT_* bits.
 * @param stick_x Raw stick X.
 * @param stick_y Raw stick Y.
 */
extern void ifRemixInputDisplayApply(s32 player, u16 buttons, s32 stick_x, s32 stick_y);

#endif

#endif
