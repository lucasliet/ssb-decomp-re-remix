/**
 * mnremixcssunplugrules.c - pure rule of the B press with an unplugged controller, see mnremixcssunplugrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcssunplugrules.h"

int mnRemixCssUnplugRulesBackTapped(int is_player_connected, unsigned int player_tap_buttons, const unsigned int *port_tap_buttons, int port_count, unsigned int b_button)
{
    int port;

    if (is_player_connected != 0)
    {
        return ((player_tap_buttons & b_button) != 0) ? 1 : 0;
    }
    for (port = 0; port < port_count; port++)
    {
        if ((port_tap_buttons[port] & b_button) != 0)
        {
            return 1;
        }
    }
    return 0;
}

#endif
