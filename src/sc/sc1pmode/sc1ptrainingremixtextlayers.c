/**
 * sc1ptrainingremixtextlayers.c - text layers of the Remix Training, see sc1ptrainingremixtextlayers.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/gbi.h>
#include <ft/fighter.h>
#include <gm/gmcamera.h>
#include <lb/lbcommon.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/rdp.h>
#include "if/ifremixtextlayer.h"
#include "sc/sc1pmode/sc1ptrainingremixtextlayers.h"

/* GObj link of the layers; a group the port never hides, so the layer shows whenever one of its texts is put. */
#define SC1PTRAININGREMIXTEXTLAYERS_GROUP 0x1D
#define SC1PTRAININGREMIXTEXTLAYERS_INTERFACE_DL_LINK 23

/* Training.asm setup_ 1653: Render.create_room_(0x39, ..., z 0x20) sits between the player arrows (35) and the magnifier (30). */
#define SC1PTRAININGREMIXTEXTLAYERS_ROWS_DL_LINK 0x39
#define SC1PTRAININGREMIXTEXTLAYERS_ROWS_PRIORITY 32

/* Label slots of remix_hud_text: reset count 25 and 27, legends and Z menu 73..104, Show Action rows 105..112, Show Action legend and exit hint 113..116. */
#define SC1PTRAININGREMIXTEXTLAYERS_RESET_VALUE_SLOT 25
#define SC1PTRAININGREMIXTEXTLAYERS_RESET_LABEL_SLOT 27
#define SC1PTRAININGREMIXTEXTLAYERS_MENU_SLOT_FIRST 73
#define SC1PTRAININGREMIXTEXTLAYERS_MENU_SLOT_COUNT 32
#define SC1PTRAININGREMIXTEXTLAYERS_ROWS_SLOT_FIRST 105
#define SC1PTRAININGREMIXTEXTLAYERS_ROWS_SLOT_COUNT 8
#define SC1PTRAININGREMIXTEXTLAYERS_HINT_SLOT_FIRST 113
#define SC1PTRAININGREMIXTEXTLAYERS_HINT_SLOT_COUNT 4

/* Rect slots: D-pad legend squares 24..27, Z menu cursor and underline 28..29, Hold to Exit bar 30. */
#define SC1PTRAININGREMIXTEXTLAYERS_RECT_SLOT_FIRST 24
#define SC1PTRAININGREMIXTEXTLAYERS_RECT_SLOT_COUNT 7

/** Sprite camera for room 0x39, made as the interface camera of the battle. */
static void sc1PTrainingRemixTextLayersMakeRowsCamera(void)
{
    GObj *camera_gobj = gcMakeCameraGObj
    (
        nGCCommonKindScissorCamera,
        NULL,
        nGCCommonLinkIDCamera,
        GOBJ_PRIORITY_DEFAULT,
        lbCommonDrawSprite,
        SC1PTRAININGREMIXTEXTLAYERS_ROWS_PRIORITY,
        COBJ_MASK_DLLINK(SC1PTRAININGREMIXTEXTLAYERS_ROWS_DL_LINK),
        ~0,
        FALSE,
        nGCProcessKindFunc,
        NULL,
        1,
        FALSE
    );
    CObj *cobj = CObjGetStruct(camera_gobj);

    syRdpSetViewport(&cobj->viewport, (f32)gGMCameraStruct.viewport_ulx, (f32)gGMCameraStruct.viewport_uly,
                     (f32)gGMCameraStruct.viewport_lrx, (f32)gGMCameraStruct.viewport_lry);
}

void sc1PTrainingRemixTextLayersMake(void)
{
    GObj *interface_gobj;
    GObj *rows_gobj;

    sc1PTrainingRemixTextLayersMakeRowsCamera();
    rows_gobj = ifRemixTextLayerMake(SC1PTRAININGREMIXTEXTLAYERS_GROUP, SC1PTRAININGREMIXTEXTLAYERS_ROWS_DL_LINK, GOBJ_PRIORITY_DEFAULT);
    interface_gobj = ifRemixTextLayerMake(SC1PTRAININGREMIXTEXTLAYERS_GROUP, SC1PTRAININGREMIXTEXTLAYERS_INTERFACE_DL_LINK,
                                          GOBJ_PRIORITY_DEFAULT);

    ifRemixTextLayerBindLabels(rows_gobj, SC1PTRAININGREMIXTEXTLAYERS_ROWS_SLOT_FIRST, SC1PTRAININGREMIXTEXTLAYERS_ROWS_SLOT_COUNT);
    ifRemixTextLayerBindLabels(interface_gobj, SC1PTRAININGREMIXTEXTLAYERS_RESET_VALUE_SLOT, 1);
    ifRemixTextLayerBindLabels(interface_gobj, SC1PTRAININGREMIXTEXTLAYERS_RESET_LABEL_SLOT, 1);
    ifRemixTextLayerBindLabels(interface_gobj, SC1PTRAININGREMIXTEXTLAYERS_MENU_SLOT_FIRST, SC1PTRAININGREMIXTEXTLAYERS_MENU_SLOT_COUNT);
    ifRemixTextLayerBindLabels(interface_gobj, SC1PTRAININGREMIXTEXTLAYERS_HINT_SLOT_FIRST, SC1PTRAININGREMIXTEXTLAYERS_HINT_SLOT_COUNT);
    ifRemixTextLayerBindRects(interface_gobj, SC1PTRAININGREMIXTEXTLAYERS_RECT_SLOT_FIRST, SC1PTRAININGREMIXTEXTLAYERS_RECT_SLOT_COUNT);
}

#endif
