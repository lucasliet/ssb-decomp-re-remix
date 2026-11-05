/**
 * sc1premixenemyrules.c - state-free rules of the Single Player Enemy option, see sc1premixenemyrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixenemyrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

#define SC1PREMIX_ENEMY_BLINK_PERIOD 80
#define SC1PREMIX_ENEMY_BLINK_SHOWN_BELOW 41

int sc1PRemixEnemySelectOnZ(SC1PRemixEnemySelection *selection, int presser, int man_port)
{
    int is_first_choice;

    if ((presser == 0) || (presser == (man_port + 1)))
    {
        return 0;
    }
    selection->enemy_port = presser;
    is_first_choice = (selection->is_selected == 0) ? 1 : 0;
    selection->is_selected = 1;

    return is_first_choice;
}

int sc1PRemixEnemyIsClearedByB(int player, unsigned int b_tap_mask)
{
    int port;

    for (port = 0; port < SC1PREMIX_ENEMY_PORT_COUNT; port++)
    {
        if ((port != player) && ((b_tap_mask & (1U << port)) != 0))
        {
            return 1;
        }
    }
    return 0;
}

int sc1PRemixEnemyControlPort(int enemy_port, int mode, int is_first_enemy)
{
    if ((is_first_enemy == 0) || (mode == SC1PREMIX_MODE_HRC) || (enemy_port == 0))
    {
        return -1;
    }
    return enemy_port - 1;
}

int sc1PRemixEnemyRespawnPort(int enemy_port, int is_first_enemy)
{
    if ((is_first_enemy == 0) || (enemy_port == 0))
    {
        return -1;
    }
    return enemy_port - 1;
}

int sc1PRemixEnemyShieldPort(int is_1p_game, int enemy_port, int is_human, int controller_port, int fighter_port)
{
    if ((is_1p_game == 0) || (enemy_port == 0) || (is_human == 0))
    {
        return fighter_port;
    }
    if ((controller_port < 0) || (controller_port >= SC1PREMIX_ENEMY_PORT_COUNT))
    {
        return fighter_port;
    }
    return controller_port;
}

int sc1PRemixEnemyBlinkNext(int timer)
{
    int next = timer + 1;

    return (next < SC1PREMIX_ENEMY_BLINK_PERIOD) ? next : 0;
}

int sc1PRemixEnemyBlinkIsOn(int timer)
{
    return ((timer + 1) < SC1PREMIX_ENEMY_BLINK_SHOWN_BELOW) ? 1 : 0;
}

int sc1PRemixEnemyTextIsVisible(int enemy_port, int is_blink_on)
{
    return ((is_blink_on != 0) && (enemy_port == 0)) ? 1 : 0;
}

int sc1PRemixEnemyPortLabelIsVisible(int enemy_port, int port)
{
    return (enemy_port == port) ? 1 : 0;
}

#endif
