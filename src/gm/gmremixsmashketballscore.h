#ifndef _GMREMIXSMASHKETBALLSCORE_H_
#define _GMREMIXSMASHKETBALLSCORE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/objdef.h>

/** Finds the home team (the first fighter's) and the away team and counts the teammates of every team. */
extern void gmRemixSmashketballFindTeams(void);

/** Points of a team, 0 when the team does not exist. */
extern s32 gmRemixSmashketballGetTeamScore(s32 team);

/**
 * Scores a basket: the left hoop credits the Away team, the right one the Home team; the owner of the ball scores for the player,
 * the others share the winner score; plays the effect and the crowd.
 *
 * @param ball_gobj The ball.
 * @param side Hoop the ball went through, negative for the left one.
 */
extern void gmRemixSmashketballScore(GObj *ball_gobj, s32 side);

#endif

#endif
