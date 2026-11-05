#ifndef _GMREMIXSMASHKETBALLCAMERA_H_
#define _GMREMIXSMASHKETBALLCAMERA_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Smashketball.asm camera_follow (RAM 0x8010C114, the end of the follow entities update): once the fighters and the weapons
 * are in the camera bounds, the basketball is added, so the camera widens to keep it in view. Does nothing outside
 * Smashketball, with no ball, or while the ball respawns.
 *
 * @param left Left bound of the camera interests, updated.
 * @param right Right bound, updated.
 * @param bottom Bottom bound, updated.
 * @param top Top bound, updated.
 */
extern void gmRemixSmashketballCameraFollowBall(f32 *left, f32 *right, f32 *bottom, f32 *top);

#endif

#endif
