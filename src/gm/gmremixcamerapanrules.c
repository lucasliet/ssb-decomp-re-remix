/**
 * gmremixcamerapanrules.c - pure pause camera pan rules, see gmremixcamerapanrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixcamerapanrules.h"

/** Moves the focus up or down (extended_movement_ _check_pan: C-Up is tested first). */
static void gmRemixCameraPanRulesStepY(GMRemixCameraPanOffsets *offsets, unsigned int button_hold)
{
    if ((button_hold & (GMREMIX_CAMERAPAN_BUTTON_C_UP | GMREMIX_CAMERAPAN_BUTTON_C_DOWN)) == 0)
    {
        return;
    }
    if (button_hold & GMREMIX_CAMERAPAN_BUTTON_C_UP)
    {
        offsets->y += GMREMIX_CAMERAPAN_STEP_XY;
        return;
    }
    offsets->y -= GMREMIX_CAMERAPAN_STEP_XY;
}

/** Moves the focus left or right (extended_movement_ _check_CPAD_LR: C-Left is tested first). */
static void gmRemixCameraPanRulesStepX(GMRemixCameraPanOffsets *offsets, unsigned int button_hold)
{
    if ((button_hold & (GMREMIX_CAMERAPAN_BUTTON_C_LEFT | GMREMIX_CAMERAPAN_BUTTON_C_RIGHT)) == 0)
    {
        return;
    }
    if (button_hold & GMREMIX_CAMERAPAN_BUTTON_C_LEFT)
    {
        offsets->x -= GMREMIX_CAMERAPAN_STEP_XY;
        return;
    }
    offsets->x += GMREMIX_CAMERAPAN_STEP_XY;
}

/** Changes the distance offset; it stops at GMREMIX_CAMERAPAN_ZOOM_MIN (apply_z_distance). */
static void gmRemixCameraPanRulesStepZoom(GMRemixCameraPanOffsets *offsets, float step)
{
    offsets->z += step;

    if (offsets->z <= GMREMIX_CAMERAPAN_ZOOM_MIN)
    {
        offsets->z = GMREMIX_CAMERAPAN_ZOOM_MIN;
    }
}

/** Changes the field of view offset inside GMREMIX_CAMERAPAN_FOV_MIN and GMREMIX_CAMERAPAN_FOV_MAX (apply_fov). */
static void gmRemixCameraPanRulesStepFov(GMRemixCameraPanOffsets *offsets, float step)
{
    offsets->fov += step * GMREMIX_CAMERAPAN_FOV_STEP_SCALE;

    if (offsets->fov <= GMREMIX_CAMERAPAN_FOV_MIN)
    {
        offsets->fov = GMREMIX_CAMERAPAN_FOV_MIN;
        return;
    }
    if (offsets->fov >= GMREMIX_CAMERAPAN_FOV_MAX)
    {
        offsets->fov = GMREMIX_CAMERAPAN_FOV_MAX;
    }
}

void gmRemixCameraPanRulesStep(GMRemixCameraPanOffsets *offsets, unsigned int button_hold)
{
    float step;

    gmRemixCameraPanRulesStepY(offsets, button_hold);
    gmRemixCameraPanRulesStepX(offsets, button_hold);

    if ((button_hold & (GMREMIX_CAMERAPAN_BUTTON_A | GMREMIX_CAMERAPAN_BUTTON_B)) == 0)
    {
        return;
    }
    step = (button_hold & GMREMIX_CAMERAPAN_BUTTON_B) ? GMREMIX_CAMERAPAN_STEP_ZOOM : -GMREMIX_CAMERAPAN_STEP_ZOOM;

    if (button_hold & GMREMIX_CAMERAPAN_BUTTON_Z)
    {
        gmRemixCameraPanRulesStepFov(offsets, step);
        return;
    }
    gmRemixCameraPanRulesStepZoom(offsets, step);
}

#endif
