#ifndef _FTREMIXDPADMENU_H_
#define _FTREMIXDPADMENU_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sys/controller.h>

/**
 * Applies the D-pad Control option and the Dpad CSS Cursor Control toggle to the global controller (css/DpadControl.asm
 * css_cursor_and_dpad_controls_, the hook at the end of syControllerUpdateGlobalData): Stick Swap on every screen but an unpaused
 * match (the Training screen too, with the DR turbo), the stick cleared by Stickless in an unpaused match and on the title, the
 * D-pad as a stick on the paused Training menu with Stickless, and the D-pad cursor of the character select screens.
 * Called from syControllerUpdateGlobalData for each connected port after the controller values are stored.
 *
 * @param port Port of the controller.
 * @param device Controller values of the port, rewritten.
 * @param prev_hold Buttons held on the previous frame.
 */
extern void ftRemixDpadMenuTransform(s32 port, SYController *device, u16 prev_hold);

/**
 * Tells whether ftRemixDpadMenuTransform has already applied Stick Swap to the controller of a Stick Swap port on the current
 * scene while the match runs (the Training scene, whose D-pad shortcuts read the global controller). In that case the D-pad bits
 * of the controller come from the stick and its stick carries the physical D-pad, so the fighter must not swap them again.
 *
 * @return TRUE when the fighter reads a controller that is already swapped.
 */
extern sb32 ftRemixDpadMenuHasSwappedController(void);

/**
 * Tells the D-pad cursor whether the token of a port is held over a fighter (the asm reads the held token index and the hovered
 * fighter of the CSS panel struct). The character select screens call it every frame; until then the D-pad moves the cursor at once.
 *
 * @param port Port of the panel.
 * @param is_token_over_fighter TRUE while the token is held over a fighter.
 */
extern void ftRemixDpadMenuSetTokenOverFighter(s32 port, sb32 is_token_over_fighter);

#endif

#endif
