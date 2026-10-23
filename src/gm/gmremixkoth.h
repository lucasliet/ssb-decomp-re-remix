#ifndef _GMREMIXKOTH_H_
#define _GMREMIXKOTH_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fttypes.h>

/*
 * Smash Remix King of the Hill (KingOfTheHill.asm): a floor line of the stage is the hill, picked at random every 20 to
 * 40 seconds. The team (the port in free-for-all) that stands alone on the hill, with the contest buffer of 30 frames
 * spent, is the king and gains one frame of score per frame. Reaching the goal (the CSS goal in seconds times 60) starts
 * the win timer and the match ends when it fills. The match runs on the rules of the Time rule, the vanilla time limit
 * is the match length and the results score is the frames on the hill.
 */

#define GMREMIX_KOTH_NO_KING (-1)
#define GMREMIX_KOTH_CONTESTED (-2)
#define GMREMIX_KOTH_NO_LINE (-1)

/** TRUE during a King of the Hill match. */
extern sb32 gmRemixKothIsActive(void);

/** Match start: resets the state, picks the first hill after 120 frames and adds the update process and the meters. */
extern void gmRemixKothSetup(void);

/** TimedStock.asm calculate_time_score_ KOTH branch: frames `port` (0..3) stood on the hill as the king. */
extern s32 gmRemixKothGetScore(s32 port);

/** Port (or team) holding the hill: GMREMIX_KOTH_NO_KING or GMREMIX_KOTH_CONTESTED when nobody does. */
extern s32 gmRemixKothGetKing(void);

/** Floor line id of the current hill, GMREMIX_KOTH_NO_LINE before the first one. */
extern s32 gmRemixKothGetHillLine(void);

/** Counter of the frames left of the contest buffer, drives the gray of the hill strip. */
extern s32 gmRemixKothGetContestedBuffer(void);

/** TRUE when `team` (the port in free-for-all) is the team of `fp` for the king check. */
extern sb32 gmRemixKothIsOnHill(FTStruct *fp);

#endif

#endif
