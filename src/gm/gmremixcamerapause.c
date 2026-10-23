/**
 * gmremixcamerapause.c - Disable HUD setting (Camera.asm entry_disable_hud) for the battle HUD and the pause restore.
 * See ft/ftremix/ftremixhooks.h for the vanilla call sites.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "if/ifcommon.h"
#include "if/ifremixhud.h"
#include "sys/objman.h"
#include "sys/objtypes.h"

/**
 * Hides the battle interface when Disable HUD is All (Camera.asm disable_hud_, after the "3, 2, 1, GO!" announcement).
 * Returns TRUE when the interface was hidden, and the caller then clears the magnifying glass.
 */
sb32 gmRemixCameraHudHiddenOnGo(void)
{
    if (ifRemixHudDisabledAll() == FALSE)
    {
        return FALSE;
    }
    ifCommonInterfaceSetGObjFlagsAll(GOBJ_FLAG_HIDDEN);
    return TRUE;
}

/**
 * Restores the battle interface when the pause ends (Camera.asm unpause_camera_). Hides the interface when Disable HUD is All
 * or the camera mode is Scene. Returns the magnifying glass flag.
 *
 * Camera.asm unpause_camera_ also destroys the Render groups 0xF and 0x10 (pause legend). Those groups are not common links
 * (vanilla links 15 and 16 are PlayerSelect and SceneCamera), and the port has no pause legend GObjs, so nothing is destroyed here.
 * REMIX-TODO(Render): destroy the pause legend GObjs here once they are ported.
 *
 * REMIX-TODO(SpecialZoom): unpause_camera_ also clears the magnifying glass while the Special Zoom is on (Zoom.zoom_type).
 */
sb32 gmRemixCameraRestorePauseHud(void)
{
    if ((ifRemixHudDisabledAll() == FALSE) && (gmRemixCameraIsScene() == FALSE))
    {
        return TRUE;
    }
    ifCommonInterfaceSetGObjFlagsAll(GOBJ_FLAG_HIDDEN);
    return FALSE;
}

#endif
