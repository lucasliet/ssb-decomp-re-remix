/**
 * grremixstagebg.c - stage model walk and stage background swap of the Remix hazards, see grremixstagebg.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixstagebg.h"
#include <gr/grwallpaper.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <mp/map.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/objtypes.h>

#include <port_log.h>
#include <string.h>

/* nGCCommonLinkIDGroundDisplay: the link of the stage model */
#define GRREMIX_STAGEBG_MODEL_LINK 2
/* asm: top joint -> first joint -> four siblings along is the warp ball */
#define GRREMIX_STAGEBG_WARP_BALL_SIBLINGS 4
/* asm: DObj flag byte 2 is "not displayed" (DOBJ_FLAG_HIDDEN) */
#define GRREMIX_STAGEBG_BALL_HIDDEN DOBJ_FLAG_HIDDEN
/* Link of the throwaway GObj of the sprite fix-up, ejected before the call returns */
#define GRREMIX_STAGEBG_SCRATCH_LINK 31
/* asm: Sprite.bitmap of the image header, at +0x34 */
#define GRREMIX_STAGEBG_IMAGE_BITMAP_FIELD 0x34

/** First joint of the stage model, NULL when the model is missing. */
static DObj *grRemixStageGetFirstJoint(void)
{
    GObj *model_gobj = gGCCommonLinks[GRREMIX_STAGEBG_MODEL_LINK];
    DObj *top_joint;

    if (model_gobj == NULL)
    {
        return NULL;
    }
    top_joint = DObjGetStruct(model_gobj);

    return (top_joint != NULL) ? top_joint->child : NULL;
}

sb32 grRemixStageReadWarpBallBgIndex(s32 *out_bg_index)
{
    DObj *ball = grRemixStageGetFirstJoint();
    s32 sibling;

    for (sibling = 0; (ball != NULL) && (sibling < GRREMIX_STAGEBG_WARP_BALL_SIBLINGS); sibling++)
    {
        ball = ball->sib_next;
    }
    if ((ball == NULL) || (ball->flags == GRREMIX_STAGEBG_BALL_HIDDEN) || (ball->mobj == NULL))
    {
        return FALSE;
    }
    *out_bg_index = ball->mobj->texture_id_curr;
    return TRUE;
}

u32 grRemixStageReadFirstJointPaletteBits(void)
{
    DObj *joint = grRemixStageGetFirstJoint();
    f32 palette_id;
    u32 bits;

    if ((joint == NULL) || (joint->mobj == NULL))
    {
        return 0;
    }
    palette_id = joint->mobj->palette_id;
    memcpy(&bits, &palette_id, sizeof(bits));
    return bits;
}

/**
 * Runs the sprite fix-ups (byte order, bitmap array) of an image header that the wallpaper maker never handled, the way
 * lbCommonMakeSObjForGObj does for the stage's own wallpaper, by making a throwaway SObj of it.
 */
static void grRemixStageBgFixupSprite(Sprite *sprite)
{
    GObj *scratch_gobj = gcMakeGObjSPAfter(0, NULL, GRREMIX_STAGEBG_SCRATCH_LINK, GOBJ_PRIORITY_DEFAULT);

    if (scratch_gobj == NULL)
    {
        port_log("[remix-stagebg] sprite fix-up skipped, the GObj pool is full\n");
        return;
    }
    lbCommonMakeSObjForGObj(scratch_gobj, sprite);
    gcEjectGObj(scratch_gobj);
}

u32 grRemixStageBgBitmapToken(u32 ground_field)
{
    u8 *ground = (u8 *) gMPCollisionGroundData;
    u32 header_token = *(u32 *) (ground + ground_field);
    Sprite *header = (Sprite *) PORT_RESOLVE(header_token);

    if (header == NULL)
    {
        port_log("[remix-stagebg] ground data field 0x%X holds no image header\n", (unsigned int) ground_field);
        return 0;
    }
    grRemixStageBgFixupSprite(header);
    return *(u32 *) ((u8 *) header + GRREMIX_STAGEBG_IMAGE_BITMAP_FIELD);
}

sb32 grRemixStageBgSetBitmap(u32 bitmap_token)
{
    GObj *wallpaper_gobj = grWallpaperGetGObj();
    SObj *wallpaper_sobj;

    if ((wallpaper_gobj == NULL) || (bitmap_token == 0))
    {
        return FALSE;
    }
    wallpaper_sobj = SObjGetStruct(wallpaper_gobj);

    if (wallpaper_sobj == NULL)
    {
        return FALSE;
    }
    wallpaper_sobj->sprite.bitmap = bitmap_token;
    return TRUE;
}

#endif
