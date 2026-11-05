#ifndef _GMREMIXCAMERAPANRULES_H_
#define _GMREMIXCAMERAPANRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pause camera pan rules (Camera.asm extended_movement_, called from Pause.asm extra_camera_controls_), pure so the headless
 * unit tests (tests/remix) can run them. The offsets are the four floats of Camera.asm camera_pan_offsets_.
 */

/* Controller button bits of the held buttons (button_hold). */
#define GMREMIX_CAMERAPAN_BUTTON_A 0x8000
#define GMREMIX_CAMERAPAN_BUTTON_B 0x4000
#define GMREMIX_CAMERAPAN_BUTTON_Z 0x2000
#define GMREMIX_CAMERAPAN_BUTTON_C_UP 0x0008
#define GMREMIX_CAMERAPAN_BUTTON_C_DOWN 0x0004
#define GMREMIX_CAMERAPAN_BUTTON_C_LEFT 0x0002
#define GMREMIX_CAMERAPAN_BUTTON_C_RIGHT 0x0001

/* Per-frame steps of extended_movement_ (0x4200 = 32.0, 0x42A0 = 80.0, 0x3C230000 = 163 / 16384 scales the 80.0 fov step). */
#define GMREMIX_CAMERAPAN_STEP_XY 32.0F
#define GMREMIX_CAMERAPAN_STEP_ZOOM 80.0F
#define GMREMIX_CAMERAPAN_FOV_STEP_SCALE (163.0F / 16384.0F)

/* Clamps: max_zoom_offset (0xC480, the lower bound of the distance offset), min_fov (0xC1E0) and max_fov (0x42F8). */
#define GMREMIX_CAMERAPAN_ZOOM_MIN (-1024.0F)
#define GMREMIX_CAMERAPAN_FOV_MIN (-28.0F)
#define GMREMIX_CAMERAPAN_FOV_MAX 124.0F

/** Pause camera offsets added to the close-up camera: x and y of the focus, distance and field of view. */
typedef struct GMRemixCameraPanOffsets
{
    float x;
    float y;
    float z;
    float fov;

} GMRemixCameraPanOffsets;

/**
 * Runs one frame of extended_movement_. C-Up and C-Down move y (C-Up wins when both are held), C-Left and C-Right move x (C-Left
 * wins), B and A move the distance (B wins; A zooms in down to GMREMIX_CAMERAPAN_ZOOM_MIN, B has no limit) or, with Z held,
 * the field of view. The first A/B block of the asm stores nothing, so it is not ported.
 *
 * @param offsets Offsets to update.
 * @param button_hold Buttons held by the player who paused.
 */
extern void gmRemixCameraPanRulesStep(GMRemixCameraPanOffsets *offsets, unsigned int button_hold);

#endif
#endif
