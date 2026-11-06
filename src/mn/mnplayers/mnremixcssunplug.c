/**
 * mnremixcssunplug.c - B press with an unplugged controller on the 1P, Training and Bonus character selects, see mnremixcssunplug.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/controller.h>
#include "mn/mnplayers/mnremixcssunplug.h"
#include "mn/mnplayers/mnremixcssunplugrules.h"
#include "sc/scsubsys/scsubsys.h"

sb32 mnRemixCssUnplugIsBackTapped(s32 player)
{
    unsigned int port_tap_buttons[MAXCONTROLLERS];
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        port_tap_buttons[port] = gSYControllerDevices[port].button_tap;
    }
    return (mnRemixCssUnplugRulesBackTapped((scSubsysControllerCheckConnected(player) != FALSE) ? 1 : 0, gSYControllerDevices[player].button_tap,
                                            port_tap_buttons, MAXCONTROLLERS, B_BUTTON) != 0) ? TRUE : FALSE;
}

#endif
