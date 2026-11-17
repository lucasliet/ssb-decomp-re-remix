/**
 * ftremixinputdisplay.c - Smash Remix InputDisplay.asm: sends the controller sample of each fighter to the Input Display sprites.
 *
 * ftRemixInputDisplayUpdate (ftMainProcUpdateInterrupt) feeds the sample the fighter reads this frame, for human and CPU
 * fighters. The sprites are made with the match by ifRemixInputDisplayMake (if/ifremixinputdisplay.c).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "if/ifremixinputdisplay.h"

/**
 * Lights the Input Display sprites of the port of a fighter for its sample; a port with no block (panel value Disabled when
 * the match started) is left alone.
 *
 * @param fp Fighter that reads the sample.
 * @param buttons Pressed buttons, libultra CONT_* bits.
 * @param stick_x Raw stick X.
 * @param stick_y Raw stick Y.
 */
void ftRemixInputDisplayUpdate(FTStruct *fp, u16 buttons, s8 stick_x, s8 stick_y)
{
    ifRemixInputDisplayApply(fp->player, buttons, stick_x, stick_y);
}

#endif
