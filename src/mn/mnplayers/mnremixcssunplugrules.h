#ifndef _MNREMIXCSSUNPLUGRULES_H_
#define _MNREMIXCSSUNPLUGRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the B press that backs out of the 1P, Training and Bonus character selects when the controller of the player was
 * unplugged (SinglePlayerMenus.asm _1P_unplugged_b_handler).
 */

/**
 * Whether the B press of a character select counts as pressed.
 *
 * @param is_player_connected Non-zero while the controller of the player is plugged in.
 * @param player_tap_buttons Buttons tapped on the controller of the player.
 * @param port_tap_buttons Buttons tapped on each controller port.
 * @param port_count Number of controller ports.
 * @param b_button Mask of the B button.
 * @return Non-zero when the player tapped B, or when his controller is unplugged and any port tapped B.
 */
extern int mnRemixCssUnplugRulesBackTapped(int is_player_connected, unsigned int player_tap_buttons, const unsigned int *port_tap_buttons, int port_count, unsigned int b_button);

#endif

#endif
