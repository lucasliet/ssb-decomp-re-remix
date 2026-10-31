/**
 * ifremixrectangle.c - translucent screen rectangle of the Remix Render library (Render.asm draw_rectangle_, _render).
 * Used by the Stopwatch backfire background. A fixed pool of slots keeps the state, the GObj points to its slot.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixrectangle.h"
#include <ft/fighter.h>
#include <sc/scene.h>
#include <sys/objman.h>
#include <sys/taskman.h>

#define IFREMIX_RECTANGLE_SLOTS_MAX 4
#define IFREMIX_RECTANGLE_DL_LINK_ID 22 /* the screen flash display link, over the battle scene */

typedef struct IFRemixRectangle
{
    sb32 is_used;
    s32 ulx;
    s32 uly;
    s32 width;
    s32 height;
    u32 color;
} IFRemixRectangle;

static IFRemixRectangle sIFRemixRectangles[IFREMIX_RECTANGLE_SLOTS_MAX];

/**
 * Display proc: fills the rectangle with its colour in the translucent render mode. A fully transparent colour draws nothing.
 *
 * @param gobj rectangle GObj, user_data points to its slot
 */
static void ifRemixRectangleProcDisplay(GObj *gobj)
{
    IFRemixRectangle *rectangle = (IFRemixRectangle*) gobj->user_data.p;

    if ((rectangle->color & 0xFF) == 0)
    {
        return;
    }
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, (rectangle->color >> 24) & 0xFF, (rectangle->color >> 16) & 0xFF, (rectangle->color >> 8) & 0xFF, rectangle->color & 0xFF);
    gDPFillRectangle(gSYTaskmanDLHeads[0]++, rectangle->ulx, rectangle->uly, rectangle->ulx + rectangle->width, rectangle->uly + rectangle->height);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
}

/**
 * Creates a rectangle (asm draw_rectangle_ with alpha on).
 *
 * @param ulx left edge in screen pixels
 * @param uly top edge in screen pixels
 * @param width width in pixels
 * @param height height in pixels
 * @param color 0xRRGGBBAA
 * @return the display GObj, or NULL when the slot pool is full
 */
GObj* ifRemixRectangleMake(s32 ulx, s32 uly, s32 width, s32 height, u32 color)
{
    GObj *rectangle_gobj;
    s32 i;

    for (i = 0; i < IFREMIX_RECTANGLE_SLOTS_MAX; i++)
    {
        if (sIFRemixRectangles[i].is_used == FALSE)
        {
            break;
        }
    }
    if (i == IFREMIX_RECTANGLE_SLOTS_MAX)
    {
        return NULL;
    }
    sIFRemixRectangles[i].is_used = TRUE;
    sIFRemixRectangles[i].ulx = ulx;
    sIFRemixRectangles[i].uly = uly;
    sIFRemixRectangles[i].width = width;
    sIFRemixRectangles[i].height = height;
    sIFRemixRectangles[i].color = color;

    rectangle_gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    rectangle_gobj->user_data.p = &sIFRemixRectangles[i];
    gcAddGObjDisplay(rectangle_gobj, ifRemixRectangleProcDisplay, IFREMIX_RECTANGLE_DL_LINK_ID, GOBJ_PRIORITY_DEFAULT, ~0);

    return rectangle_gobj;
}

/** Sets the 0xRRGGBBAA colour of a rectangle (asm: sw to object + 0x40). */
void ifRemixRectangleSetColor(GObj *rectangle_gobj, u32 color)
{
    ((IFRemixRectangle*) rectangle_gobj->user_data.p)->color = color;
}

/** Returns the 0xRRGGBBAA colour of a rectangle. */
u32 ifRemixRectangleGetColor(GObj *rectangle_gobj)
{
    return ((IFRemixRectangle*) rectangle_gobj->user_data.p)->color;
}

/** Frees every slot; the GObjs are freed with the scene. */
void ifRemixRectangleResetAll(void)
{
    s32 i;

    for (i = 0; i < IFREMIX_RECTANGLE_SLOTS_MAX; i++)
    {
        sIFRemixRectangles[i].is_used = FALSE;
    }
}

/** Frees the slot and the GObj (asm Render.DESTROY_OBJECT_). */
void ifRemixRectangleDestroy(GObj *rectangle_gobj)
{
    ((IFRemixRectangle*) rectangle_gobj->user_data.p)->is_used = FALSE;
    gcEjectGObj(rectangle_gobj);
}

#endif
