/**
 * mnremixcsstextlayers.c - text layer of the CSS indicator room 0x38, see mnremixcsstextlayers.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <lb/lbcommon.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/rdp.h>
#include "if/ifremixtextlayer.h"
#include "mn/mnplayers/mnremixcsstextlayers.h"

/* CharacterSelect.asm setup_: Render.create_room_(0x38, 0x10, z) and the indicators in group 0x13. */
#define MNREMIX_CSS_TEXT_INDICATOR_DL_LINK 0x38
#define MNREMIX_CSS_TEXT_INDICATOR_GROUP 0x13
#define MNREMIX_CSS_TEXT_CAMERA_LINK 16

/** Sprite camera of room 0x38, which the character selects do not have: priority 21 draws it under the cursor hands (20). */
static void mnRemixCssTextMakeIndicatorCamera(s32 priority)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            nGCCommonKindSceneCamera,
            NULL,
            MNREMIX_CSS_TEXT_CAMERA_LINK,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            priority,
            COBJ_MASK_DLLINK(MNREMIX_CSS_TEXT_INDICATOR_DL_LINK),
            ~0,
            FALSE,
            nGCProcessKindFunc,
            NULL,
            1,
            FALSE
        )
    );
    syRdpSetViewport(&cobj->viewport, 10.0F, 10.0F, 310.0F, 230.0F);
}

void mnRemixCssTextLayersMake(s32 indicator_priority)
{
    GObj *layer_gobj;

    mnRemixCssTextMakeIndicatorCamera(indicator_priority);
    layer_gobj = ifRemixTextLayerMake(MNREMIX_CSS_TEXT_INDICATOR_GROUP, MNREMIX_CSS_TEXT_INDICATOR_DL_LINK, GOBJ_PRIORITY_DEFAULT);

    ifRemixTextLayerBindRects(layer_gobj, MNREMIX_CSS_TEXT_INDICATOR_RECT_FIRST, MNREMIX_CSS_TEXT_INDICATOR_RECT_COUNT);
    ifRemixTextLayerBindLabels(layer_gobj, MNREMIX_CSS_TEXT_INDICATOR_LABEL_FIRST, MNREMIX_CSS_TEXT_INDICATOR_LABEL_COUNT);
}

#endif
