/**
 * grremixcamera.c - Remix camera routine and zoom overrides for the vanilla camera. See ft/ftremix/ftremixhooks.h.
 */

#ifdef PORT

#include <ssb_types.h>
#include <gm/gmdef.h>
#include <gr/grremix/grremixids.h>
#include <sc/scene.h>
#include <sc/scmanager.h>

#define GRREMIX_CAMERA_VENOM_ZOOM_RANGE 1.75F
#define GRREMIX_CAMERA_STANDARD_ZOOM_RANGE 1.0F

/**
 * Camera routine of a stage that has no Zebes or Mushroom Kingdom routine (Camera.asm override_camera_type_).
 * The Remix stages in the list use the Mushroom Kingdom camera, which is nGMCameraStatusInishie.
 */
s32 grRemixCameraDefaultStatus(s32 gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_MK_REMIX:
    case nGRRemixStage_SUBCON:
    case nGRRemixStage_SECTOR_Z_REMIX:
    case nGRRemixStage_ONETT:
    case nGRRemixStage_BLUE:
    case nGRRemixStage_MUTE:
    case nGRRemixStage_HTEMPLE:
    case nGRRemixStage_DRAGONKING_REMIX:
    case nGRRemixStage_DRAGONKING:
    case nGRRemixStage_PEACH2:
    case nGRRemixStage_MT_DEDEDE:
    case nGRRemixStage_FIRST_REMIX:
    case nGRRemixStage_MELRODE:
    case nGRRemixStage_SCUTTLE_TOWN:
    case nGRRemixStage_YOSHIS_ISLAND_MELEE:
    case nGRRemixStage_BIG_SNOWMAN:
    case nGRRemixStage_GYM_LEADER_CASTLE:
    case nGRRemixStage_N64_REMIX:
    case nGRRemixStage_TOADSTURNPIKE:
        return nGMCameraStatusInishie;

    default:
        return nGMCameraStatusDefault;
    }
}

/** TRUE on the screens where Camera.asm camera_adjust_ checks the stage: VS, Training and the 1P game. */
static sb32 grRemixCameraIsStageCheckScreen(void)
{
    u8 scene = gSCManagerSceneData.scene_curr;

    return (scene == nSCKindVSBattle) || (scene == nSCKind1PTrainingMode) || (scene == nSCKind1PGame);
}

/**
 * Zoom range of a fighter's status change (Camera.asm camera_adjust_): Venom and Gameboy Land pull the camera back
 * further. Other stages and screens keep the vanilla range.
 */
f32 grRemixCameraAdjustZoomRange(void)
{
    s32 gkind;

    if (grRemixCameraIsStageCheckScreen() == FALSE)
    {
        return GRREMIX_CAMERA_STANDARD_ZOOM_RANGE;
    }
    gkind = gSCManagerBattleState->gkind;

    if ((gkind == nGRRemixStage_GB_LAND) || (gkind == nGRRemixStage_VENOM))
    {
        return GRREMIX_CAMERA_VENOM_ZOOM_RANGE;
    }
    return GRREMIX_CAMERA_STANDARD_ZOOM_RANGE;
}

#endif
