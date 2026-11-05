#ifndef _GMREMIXCAMERAPAN_H_
#define _GMREMIXCAMERAPAN_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pause camera pan, zoom and field of view of the close-up camera (Camera.asm vs_pause_pan_, pause_initial_ and unpause_camera_,
 * fed by Pause.asm extra_camera_controls_ and Camera.asm extended_movement_). Rules: gm/gmremixcamerapanrules.h.
 */

#include <ssb_types.h>
#include <sys/vector.h>

/**
 * Reads the held buttons of the player who paused and moves the offsets (Pause.asm extra_camera_controls_ at RAM 0x80114328,
 * once per pause frame while the pause camera is the controllable one).
 *
 * @param button_hold Buttons held by the player who paused.
 */
extern void gmRemixCameraPanUpdate(u16 button_hold);

/**
 * Zeroes the offsets unless the Camera Mode is Scene, where they stay (Camera.asm pause_initial_ and unpause_camera_).
 */
extern void gmRemixCameraPanClear(void);

/**
 * Pause start check (Camera.asm pause_initial_, RAM 0x801141B4): clears the offsets, then runs the vanilla bounds check.
 *
 * @param pos Position of the fighter who paused.
 * @return TRUE when the fighter is out of the camera bounds.
 */
extern sb32 gmRemixCameraPanCheckPauseOutBounds(Vec3f *pos);

/**
 * Adds the offsets to the focus and to the distance of the close-up camera (Camera.asm vs_pause_pan_, RAM 0x8010C9CC).
 *
 * @param pos Focus of the close-up camera, x and y are moved.
 * @param dist Distance of the close-up camera, the distance offset is added.
 */
extern void gmRemixCameraPanApply(Vec3f *pos, f32 *dist);

/**
 * Sets the field of view of the close-up camera to its target plus the field of view offset when that offset is not zero
 * (Camera.asm vs_pause_pan_, the store to the camera fovy). Runs after gmCameraAdjustFOV.
 *
 * @param target_fov Field of view the close-up camera moves to (pzoom_fov).
 */
extern void gmRemixCameraPanApplyFov(f32 target_fov);

#endif
#endif
