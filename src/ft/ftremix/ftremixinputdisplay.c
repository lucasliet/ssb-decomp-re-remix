/**
 * ftremixinputdisplay.c - Smash Remix InputDisplay.asm: sends the controller sample of each fighter to the port overlay.
 *
 * ftRemixInputDisplayUpdate (ftMainProcUpdateInterrupt) feeds the sample the fighter reads this frame, for human and CPU
 * fighters. ftRemixInputDisplayClear (scmanager, at each scene change) hides every port.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixpanel.h>
#include "remix/remix_input_display.h"

/**
 * Sends the sample of a fighter to the Input Display overlay of its port. Does nothing when the port's Input Display
 * panel value is 0 (Disabled), so the overlay never draws a port that is not set to show it.
 */
void ftRemixInputDisplayUpdate(FTStruct *fp, u16 buttons, s8 stick_x, s8 stick_y)
{
    s32 player = fp->player;

    if (player >= GMCOMMON_PLAYERS_MAX)
    {
        return;
    }
    if (gmRemixPanel(INPUT_DISPLAY, player) == 0)
    {
        return;
    }
    remix_input_display_update(player, buttons, stick_x, stick_y);
}

/** Hides every Input Display port. Called from scManagerRunLoop at each scene change. */
void ftRemixInputDisplayClear(void)
{
    remix_input_display_clear();
}

#endif
