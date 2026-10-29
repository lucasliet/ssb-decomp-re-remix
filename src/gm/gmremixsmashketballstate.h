#ifndef _GMREMIXSMASHKETBALLSTATE_H_
#define _GMREMIXSMASHKETBALLSTATE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sc/sctypes.h>

/* State of a Smashketball match, shared by the match (gmremixsmashketball.c) and the scoring (gmremixsmashketballscore.c). */

typedef struct GMRemixSmashketballState
{
    s32 frames;
    sb32 is_ended;
    sb32 spawn_golden;
    s32 home_team;
    s32 away_team;
    s32 scores[GMCOMMON_PLAYERS_MAX];
    s32 team_scores[GMCOMMON_PLAYERS_MAX];
    s32 winner_scores[GMCOMMON_PLAYERS_MAX];
    s32 teammates[GMCOMMON_PLAYERS_MAX];
    s32 consecutive_baskets[GMCOMMON_PLAYERS_MAX];
} GMRemixSmashketballState;

extern GMRemixSmashketballState gGMRemixSmashketball;

#endif

#endif
