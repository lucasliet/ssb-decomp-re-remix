/**
 * gmremixcamerapause.c - Disable HUD setting (Camera.asm entry_disable_hud) for the battle HUD and the pause restore.
 * See ft/ftremix/ftremixhooks.h for the vanilla call sites.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "if/ifcommon.h"
#include "if/ifremixhud.h"
#include "lb/lbcommon.h"
#include "sys/objman.h"
#include "sys/objtypes.h"

/* Camera.asm unpause_camera_ destroys the Remix pause legend objects of the common link groups 0xF and 0x10 */
#define GM_REMIX_CAMERA_LEGEND_LINK_FIRST 0x0F
#define GM_REMIX_CAMERA_LEGEND_LINK_SECOND 0x10

/** Destroys the objects of one common link group, if it has any. */
static void gmRemixCameraEjectLegendLink(s32 link_id)
{
    if (gGCCommonLinks[link_id] != NULL)
    {
        lbCommonEjectGObjLinkedList(gGCCommonLinks[link_id]);
    }
}

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
 * Restores the battle interface when the pause ends (Camera.asm unpause_camera_). Destroys the pause legend, then hides the
 * interface when Disable HUD is All or the camera mode is Scene. Returns the magnifying glass flag.
 *
 * REMIX-TODO(SpecialZoom): unpause_camera_ also clears the magnifying glass while the Special Zoom is on (Zoom.zoom_type).
 */
sb32 gmRemixCameraRestorePauseHud(void)
{
    gmRemixCameraEjectLegendLink(GM_REMIX_CAMERA_LEGEND_LINK_FIRST);
    gmRemixCameraEjectLegendLink(GM_REMIX_CAMERA_LEGEND_LINK_SECOND);

    if ((ifRemixHudDisabledAll() == FALSE) && (gmRemixCameraIsScene() == FALSE))
    {
        return TRUE;
    }
    ifCommonInterfaceSetGObjFlagsAll(GOBJ_FLAG_HIDDEN);
    return FALSE;
}

#endif
