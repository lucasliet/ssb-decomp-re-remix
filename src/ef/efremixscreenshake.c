/**
 * efremixscreenshake.c - Screenshake setting of Smash Remix (Toggles.asm entry_screenshake).
 *
 * Accessibility.asm screenshake_toggle patches efManagerQuakeMakeEffect (0x80100964) so that the severity it switches on
 * is remapped: LIGHT forces the lightest shake, OFF gives an invalid severity that takes no shake branch.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"

#define EFREMIX_SCREENSHAKE_LIGHT 1
#define EFREMIX_SCREENSHAKE_OFF 2

/** Returns the severity the quake effect switches on for the Screenshake setting; `severity` when the setting is DEFAULT. */
s32 efRemixQuakeSeverity(s32 severity)
{
    s32 mode = gmRemixToggle(screenshake);

    if (mode == EFREMIX_SCREENSHAKE_LIGHT)
    {
        return 0;
    }
    if (mode == EFREMIX_SCREENSHAKE_OFF)
    {
        return -1;
    }
    return severity;
}

#endif
