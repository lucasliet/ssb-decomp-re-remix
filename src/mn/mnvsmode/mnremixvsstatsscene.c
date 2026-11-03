/**
 * mnremixvsstatsscene.c - camera, dimmed background and button sprites of the VS match stats, see mnremixvsstatsscene.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <lb/lbcommon.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/rdp.h>
#include <sys/taskman.h>
#include "gm/gmremixgroupdisplay.h"
#include "if/ifremixsprite.h"
#include "mn/mnvsmode/mnremixvsstatsscene.h"

extern Gfx *gSYTaskmanDLHeads[4];

/* Display links nothing else in the results scene uses (it takes 26 to 35), and the camera that draws them after the rows
 * (camera priority 15) and before the transition (10). */
#define MNREMIX_VSSTATS_SCENE_DL_BACKGROUND 36
#define MNREMIX_VSSTATS_SCENE_DL_SPRITES 37
#define MNREMIX_VSSTATS_SCENE_CAMERA_PRIORITY 12
#define MNREMIX_VSSTATS_SCENE_CAMERA_LINK 16
#define MNREMIX_VSSTATS_SCENE_BACKGROUND_LINK_ID 0x0E

/* The results screen safe area (10, 10) to (310, 230), and the page background alpha of the asm draw_rectangle. */
#define MNREMIX_VSSTATS_SCENE_AREA_X0 10
#define MNREMIX_VSSTATS_SCENE_AREA_Y0 10
#define MNREMIX_VSSTATS_SCENE_AREA_X1 310
#define MNREMIX_VSSTATS_SCENE_AREA_Y1 230
#define MNREMIX_VSSTATS_SCENE_BACKGROUND_ALPHA 0xB0

/* VsStats.asm press_a_obj / press_b / press_r: X, Y, tint, palette, scale of each button texture. */
#define MNREMIX_VSSTATS_SCENE_A_X 126.0F
#define MNREMIX_VSSTATS_SCENE_A_Y 216.0F
#define MNREMIX_VSSTATS_SCENE_A_TINT 0x50A8FFFF
#define MNREMIX_VSSTATS_SCENE_A_PALETTE 0x0010FFFF
#define MNREMIX_VSSTATS_SCENE_B_X 22.5F
#define MNREMIX_VSSTATS_SCENE_B_Y 14.0F
#define MNREMIX_VSSTATS_SCENE_B_TINT 0x00D040FF
#define MNREMIX_VSSTATS_SCENE_B_PALETTE 0x003000FF
#define MNREMIX_VSSTATS_SCENE_R_X 71.5F
#define MNREMIX_VSSTATS_SCENE_R_Y 15.0F
#define MNREMIX_VSSTATS_SCENE_R_TINT 0x848484FF
#define MNREMIX_VSSTATS_SCENE_R_PALETTE 0x303030FF
#define MNREMIX_VSSTATS_SCENE_R_SCALE 0.9375F

static void mnRemixVSStatsSceneMakeCamera(void)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            nGCCommonKindSceneCamera,
            NULL,
            MNREMIX_VSSTATS_SCENE_CAMERA_LINK,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            MNREMIX_VSSTATS_SCENE_CAMERA_PRIORITY,
            COBJ_MASK_DLLINK(MNREMIX_VSSTATS_SCENE_DL_BACKGROUND) | COBJ_MASK_DLLINK(MNREMIX_VSSTATS_SCENE_DL_SPRITES),
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

/** Translucent black over the whole results screen, the page background (Render.draw_rectangle in the asm). */
static void mnRemixVSStatsSceneBackgroundDisplay(GObj *gobj)
{
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, 0x00, 0x00, 0x00, MNREMIX_VSSTATS_SCENE_BACKGROUND_ALPHA);
    gDPSetCombineLERP(gSYTaskmanDLHeads[0]++, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE, 0, 0, 0, PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPFillRectangle(gSYTaskmanDLHeads[0]++, MNREMIX_VSSTATS_SCENE_AREA_X0, MNREMIX_VSSTATS_SCENE_AREA_Y0,
                     MNREMIX_VSSTATS_SCENE_AREA_X1, MNREMIX_VSSTATS_SCENE_AREA_Y1);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);

    lbCommonClearExternSpriteParams();
}

static void mnRemixVSStatsSceneMakeBackground(void)
{
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, MNREMIX_VSSTATS_SCENE_BACKGROUND_LINK_ID, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, mnRemixVSStatsSceneBackgroundDisplay, MNREMIX_VSSTATS_SCENE_DL_BACKGROUND, GOBJ_PRIORITY_DEFAULT, ~0);
}

void mnRemixVSStatsSceneMake(void)
{
    mnRemixVSStatsSceneMakeCamera();
    mnRemixVSStatsSceneMakeBackground();
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_A, MNREMIX_VSSTATS_GROUP_PROMPT, MNREMIX_VSSTATS_SCENE_DL_SPRITES,
                      MNREMIX_VSSTATS_SCENE_A_X, MNREMIX_VSSTATS_SCENE_A_Y, MNREMIX_VSSTATS_SCENE_A_TINT,
                      MNREMIX_VSSTATS_SCENE_A_PALETTE, 1.0F);
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_B, MNREMIX_VSSTATS_GROUP_PAGES, MNREMIX_VSSTATS_SCENE_DL_SPRITES,
                      MNREMIX_VSSTATS_SCENE_B_X, MNREMIX_VSSTATS_SCENE_B_Y, MNREMIX_VSSTATS_SCENE_B_TINT,
                      MNREMIX_VSSTATS_SCENE_B_PALETTE, 1.0F);
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_R, MNREMIX_VSSTATS_GROUP_PAGES, MNREMIX_VSSTATS_SCENE_DL_SPRITES,
                      MNREMIX_VSSTATS_SCENE_R_X, MNREMIX_VSSTATS_SCENE_R_Y, MNREMIX_VSSTATS_SCENE_R_TINT,
                      MNREMIX_VSSTATS_SCENE_R_PALETTE, MNREMIX_VSSTATS_SCENE_R_SCALE);
    gmRemixGroupDisplaySet(MNREMIX_VSSTATS_GROUP_PROMPT, 1);
    gmRemixGroupDisplaySet(MNREMIX_VSSTATS_GROUP_PAGES, 1);
}

#endif
