#ifndef _FTREMIXAIMODES_H_
#define _FTREMIXAIMODES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* CPU hooks of the Remix game modes (Smashketball.asm ai_ball_behavior_ and ai_ball_attack_range, KingOfTheHill.asm
 * ai_hill_behavior_), used by ftcomputer.c. */

/** What the attack picker reads about its target; ai_ball_attack_range replaces it with the ball in Smashketball. */
typedef struct FTRemixAiBallTarget
{
    f32 hurtbox_width;
    f32 hurtbox_height;
    f32 pos_x;
    f32 pos_y;
    f32 vel_x;
    f32 vel_y;
    f32 tvel_base;
    f32 gravity;

} FTRemixAiBallTarget;

/**
 * ai_ball_behavior_ (end of ftComputerCheckFindTarget): in Smashketball with a ball in play, the CPU target position becomes
 * the ball moved 400 units behind it for its side. Recomputes distance, the squared distance from (this_pos_x, this_pos_y).
 * Returns TRUE when the target was replaced.
 */
sb32 ftRemixAiModesSeekBall(FTStruct *fp, f32 this_pos_x, f32 this_pos_y, f32 *distance);

/**
 * ai_ball_attack_range (ftComputerCheckDetectTarget, before the attack list): fills ball_target with the ball hurtbox, position,
 * velocity, fall speed and gravity. Returns FALSE outside Smashketball or with no ball in play.
 */
sb32 ftRemixAiModesGetBallTarget(FTRemixAiBallTarget *ball_target);

/**
 * ai_hill_behavior_ (start of ftComputerCheckTargetItemOrTwister): in King of the Hill the CPU walks to the hill marker, 500
 * units below it, on the hill line, and ignores items and the Hyrule tornado. Returns TRUE when the target was set and the
 * caller must return TRUE. With the marker hidden, sets target_line_id to -1 and returns FALSE.
 */
sb32 ftRemixAiModesSeekHill(FTStruct *fp);

#endif

#endif
