/**
 * gmremixspecialzoomwallpaper.c - Smash Remix SpecialZoom.asm: the background of the Special Zoom (BETA) setting, the
 * wallpaper camera made with the stage camera and the background object drawn behind the zoomed fighters.
 *
 * Load: create_zoom_camera (RAM 0x8010DC10, file 0x04B). Show, play and hide: apply_zoom create_gfx, objects_update and
 * end_zoom (RAM 0x8000A5EC, 0x8010C9BC).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <lb/lbreloc.h>
#include <sys/objanim.h>
#include <sys/objdisplay.h>
#include <sys/objhelper.h>
#include <sys/objman.h>
#include <sys/rdp.h>
#include <sys/taskman.h>
#include <gm/gmcamera.h>
#include "ft/ftremix/ftremixhooks.h"

#define FTREMIX_SPECIAL_ZOOM_BACKGROUND_FILE 0x04B
#define FTREMIX_SPECIAL_ZOOM_BACKGROUND_OBJECT_ID 0x3F0
#define FTREMIX_SPECIAL_ZOOM_BACKGROUND_OBJECT_LINK 0xD
#define FTREMIX_SPECIAL_ZOOM_BACKGROUND_DISPLAY_ROOM 5
#define FTREMIX_SPECIAL_ZOOM_BACKGROUND_SCALE 7.6F
#define FTREMIX_SPECIAL_ZOOM_BACKGROUND_TRANSLATE_Z -8000.0F
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_ID 0x3EF
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_DL_PRIORITY 60
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_MASK 0x20
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_DIST 2000.0F
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_DOBJDESC_OFFSET 0x35F8
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_MOBJ_OFFSET 0x2AA8
#define FTREMIX_SPECIAL_ZOOM_WALLPAPER_ANIM_OFFSET 0x3700

/* Background file loaded by ftRemixSpecialZoomWallpaperLoad, and the background object shown during a KO zoom */
static void *sFTRemixSpecialZoomBackgroundFile;
static GObj *sFTRemixSpecialZoomBackgroundObject;

/**
 * Loads the background file into memory (Render.load_file) and forgets the previous background object.
 */
void ftRemixSpecialZoomWallpaperLoad(void)
{
    sFTRemixSpecialZoomBackgroundFile = lbRelocGetExternHeapFile(FTREMIX_SPECIAL_ZOOM_BACKGROUND_FILE, syTaskmanMalloc(lbRelocGetFileSize(FTREMIX_SPECIAL_ZOOM_BACKGROUND_FILE), 0x10));
    sFTRemixSpecialZoomBackgroundObject = NULL;
}

/**
 * Sets the eye and the aspect of the wallpaper camera from the stage camera viewport (the eye 2000 units in front of the
 * origin, looking at it), as create_zoom_camera does.
 */
static void ftRemixSpecialZoomWallpaperSetupCObj(CObj *cobj)
{
    s32 viewport_width = gGMCameraStruct.viewport_lrx - gGMCameraStruct.viewport_ulx;
    s32 viewport_height = gGMCameraStruct.viewport_lry - gGMCameraStruct.viewport_uly;

    cobj->flags |= 0x4;
    cobj->vec.eye.x = 0.0F;
    cobj->vec.eye.y = 0.0F;
    cobj->vec.eye.z = FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_DIST;
    cobj->vec.at.x = 0.0F;
    cobj->vec.at.y = 0.0F;
    cobj->vec.at.z = 0.0F;
    cobj->projection.persp.aspect = (f32) viewport_width / (f32) viewport_height;
}

/**
 * Makes the wallpaper camera of the Special Zoom, a camera that draws the background in its own room (create_zoom_camera).
 */
void ftRemixSpecialZoomWallpaperCreateCamera(void)
{
    GObj *camera_gobj = gcMakeCameraGObj
    (
        FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_ID,
        NULL,
        nGCCommonLinkIDCamera,
        GOBJ_PRIORITY_DEFAULT,
        func_80017EC0,
        FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_DL_PRIORITY,
        FTREMIX_SPECIAL_ZOOM_WALLPAPER_CAMERA_MASK,
        1,
        TRUE,
        nGCProcessKindFunc,
        NULL,
        1,
        FALSE
    );
    CObj *cobj = CObjGetStruct(camera_gobj);

    syRdpSetViewport
    (
        &cobj->viewport,
        gGMCameraStruct.viewport_ulx,
        gGMCameraStruct.viewport_uly,
        gGMCameraStruct.viewport_lrx,
        gGMCameraStruct.viewport_lry
    );
    ftRemixSpecialZoomWallpaperSetupCObj(cobj);
}

/**
 * Creates the background object of the Special Zoom (create_gfx): its display list, scale, materials and animations come
 * from the background file.
 */
void ftRemixSpecialZoomWallpaperShow(void)
{
    GObj *background_gobj = gcMakeGObjSPAfter(FTREMIX_SPECIAL_ZOOM_BACKGROUND_OBJECT_ID, NULL, FTREMIX_SPECIAL_ZOOM_BACKGROUND_OBJECT_LINK, GOBJ_PRIORITY_DEFAULT);
    DObj *dobj;

    sFTRemixSpecialZoomBackgroundObject = background_gobj;

    if (background_gobj == NULL)
    {
        return;
    }
    gcSetupCommonDObjs(background_gobj, (DObjDesc *) ((uintptr_t) sFTRemixSpecialZoomBackgroundFile + FTREMIX_SPECIAL_ZOOM_WALLPAPER_DOBJDESC_OFFSET), NULL);
    gcAddGObjDisplay(background_gobj, gcDrawDObjTreeForGObj, FTREMIX_SPECIAL_ZOOM_BACKGROUND_DISPLAY_ROOM, GOBJ_PRIORITY_DEFAULT, -1);

    dobj = DObjGetStruct(background_gobj);
    dobj->translate.vec.f.x = 0.0F;
    dobj->translate.vec.f.y = 0.0F;
    dobj->translate.vec.f.z = FTREMIX_SPECIAL_ZOOM_BACKGROUND_TRANSLATE_Z;
    /* REMIX-TODO(unknown global): create_zoom_camera sets dobj->rotate.vec.f.y from the float at RAM 0x80132704, which no
     * port names yet; the DObj keeps its default rotation. */
    dobj->scale.vec.f.x = FTREMIX_SPECIAL_ZOOM_BACKGROUND_SCALE;
    dobj->scale.vec.f.y = FTREMIX_SPECIAL_ZOOM_BACKGROUND_SCALE;
    dobj->scale.vec.f.z = FTREMIX_SPECIAL_ZOOM_BACKGROUND_SCALE;

    gcAddMObjAll(background_gobj, (MObjSub ***) ((uintptr_t) sFTRemixSpecialZoomBackgroundFile + FTREMIX_SPECIAL_ZOOM_WALLPAPER_MOBJ_OFFSET));
    gcAddAnimJointAll(background_gobj, (AObjEvent32 **) ((uintptr_t) sFTRemixSpecialZoomBackgroundFile + FTREMIX_SPECIAL_ZOOM_WALLPAPER_ANIM_OFFSET), 0.0F);
    gcAddGObjProcess(background_gobj, gcPlayAnimAll, nGCProcessKindFunc, 1);
    gcPlayAnimAll(background_gobj);
}

/**
 * Plays the animation of the background object, if there is one (objects_update runs it on odd frames).
 */
void ftRemixSpecialZoomWallpaperPlayAnim(void)
{
    if (sFTRemixSpecialZoomBackgroundObject != NULL)
    {
        gcPlayAnimAll(sFTRemixSpecialZoomBackgroundObject);
    }
}

/**
 * Destroys the background object of the Special Zoom, if there is one (end_zoom).
 */
void ftRemixSpecialZoomWallpaperHide(void)
{
    if (sFTRemixSpecialZoomBackgroundObject == NULL)
    {
        return;
    }
    gcEjectGObj(sFTRemixSpecialZoomBackgroundObject);
    sFTRemixSpecialZoomBackgroundObject = NULL;
}

#endif
