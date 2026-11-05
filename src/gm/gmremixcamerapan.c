/**
 * gmremixcamerapan.c - pause camera offsets of the Remix close-up camera, see gmremixcamerapan.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixcamerapan.h"
#include "gm/gmremixcamerapanrules.h"
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmcamera.h"

static GMRemixCameraPanOffsets sGMRemixCameraPanOffsets;

void gmRemixCameraPanUpdate(u16 button_hold)
{
    gmRemixCameraPanRulesStep(&sGMRemixCameraPanOffsets, button_hold);
}

void gmRemixCameraPanClear(void)
{
    if (gmRemixCameraIsScene() != FALSE)
    {
        return;
    }
    sGMRemixCameraPanOffsets.x = 0.0F;
    sGMRemixCameraPanOffsets.y = 0.0F;
    sGMRemixCameraPanOffsets.z = 0.0F;
    sGMRemixCameraPanOffsets.fov = 0.0F;
}

sb32 gmRemixCameraPanCheckPauseOutBounds(Vec3f *pos)
{
    gmRemixCameraPanClear();

    return gmCameraCheckPausePlayerOutBounds(pos);
}

void gmRemixCameraPanApply(Vec3f *pos, f32 *dist)
{
    pos->x += sGMRemixCameraPanOffsets.x;
    pos->y += sGMRemixCameraPanOffsets.y;
    *dist += sGMRemixCameraPanOffsets.z;
}

void gmRemixCameraPanApplyFov(f32 target_fov)
{
    if (sGMRemixCameraPanOffsets.fov == 0.0F)
    {
        return;
    }
    gGMCameraStruct.fovy = target_fov + sGMRemixCameraPanOffsets.fov;
}

#endif
