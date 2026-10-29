#ifndef _GMREMIXKOTHSTATE_H_
#define _GMREMIXKOTHSTATE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/sctypes.h>

/* State of a King of the Hill match, shared by the hill, the king and the match end (gmremixkoth.c, gmremixkothking.c). */

#define GMREMIX_KOTH_CONTESTED_BUFFER 30
#define GMREMIX_KOTH_FGM_KING_GAINED 0x115
#define GMREMIX_KOTH_FGM_KING_LOST 0xA9

typedef struct GMRemixKothState
{
    sb32 is_running;
    sb32 is_ended;
    s32 scores[GMCOMMON_PLAYERS_MAX];
    s32 team_scores[GMCOMMON_PLAYERS_MAX];
    s32 winner_times[GMCOMMON_PLAYERS_MAX];
    s32 hill_line;
    s32 timer;
    s32 hidden_frames;
    s32 king;
    s32 contested_buffer;
} GMRemixKothState;

extern GMRemixKothState gGMRemixKoth;

/** Score key of a fighter: its team (key of team_scores and winner_times). */
extern s32 gmRemixKothGetKey(FTStruct *fp);

/** TRUE when the fighter stands on the hill without being held (the hill line, on the ground). */
extern sb32 gmRemixKothIsOnHill(FTStruct *fp);

#endif

#endif
