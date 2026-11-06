/**
 * sc1premixallstarpercentrules.c - carried percent and Rest Area exit rules of All-Star, see sc1premixallstarpercentrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixallstarpercentrules.h"

#define SC1PREMIX_ALLSTAR_PORT_COUNT 4

int sc1PRemixAllstarEnemyPort(int man_port)
{
    return (man_port + 1) % SC1PREMIX_ALLSTAR_PORT_COUNT;
}

int sc1PRemixAllstarTakesPercent(int enemy_port, int man_port, int port)
{
    if (enemy_port == 0)
    {
        return 1;
    }
    return (port != sc1PRemixAllstarEnemyPort(man_port)) ? 1 : 0;
}

int sc1PRemixAllstarRecordedPercent(int percent, int is_limbo)
{
    return (is_limbo != 0) ? 0 : percent;
}

int sc1PRemixAllstarRestIsLeft(int is_grounded, int is_on_exit, int is_afk, unsigned int frames_elapsed)
{
    if (is_grounded == 0)
    {
        return 0;
    }
    if ((is_afk != 0) && (frames_elapsed >= SC1PREMIX_ALLSTAR_AFK_REST_FRAMES))
    {
        return 1;
    }
    return (is_on_exit != 0) ? 1 : 0;
}

#endif
