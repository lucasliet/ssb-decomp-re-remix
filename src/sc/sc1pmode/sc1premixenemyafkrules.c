/**
 * sc1premixenemyafkrules.c - state-free rules of the "afk" player tag automation, see sc1premixenemyafkrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixenemyafkrules.h"

#include <stddef.h>

int sc1PRemixAfkIsTagName(const char *name)
{
    if (name == NULL)
    {
        return 0;
    }
    return ((name[0] == 'a') && (name[1] == 'f') && (name[2] == 'k') && (name[3] == '\0')) ? 1 : 0;
}

int sc1PRemixAfkMakesPlayerBot(int is_afk, int is_player_spawn, int is_contest)
{
    return ((is_afk != 0) && (is_player_spawn != 0) && (is_contest == 0)) ? 1 : 0;
}

int sc1PRemixAfkSpeedAssistFires(int is_afk, int name_id, float speed, float normal_speed)
{
    if ((is_afk == 0) || (name_id != SC1PREMIX_AFK_CREDITS_FIRST_REMIX_NAME_ID))
    {
        return 0;
    }
    return (speed != normal_speed) ? 1 : 0;
}

#endif
