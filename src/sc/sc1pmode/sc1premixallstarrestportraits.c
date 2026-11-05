/**
 * sc1premixallstarrestportraits.c - portraits of the next opponents in the All-Star Rest Area, see sc1premixallstarrestportraits.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/gbi.h>
#include <lb/library.h>
#include <port_log.h>
#include <sys/objman.h>
#include <sys/objtypes.h>
#include <sys/taskman.h>
#include "remix/remix_css_data.h"
#include "sc/sc1pmode/sc1premixallstar.h"
#include "sc/sc1pmode/sc1premixallstarrest.h"
#include "sc/sc1pmode/sc1premixallstarrestrules.h"

extern void portFixupSprite(void *sprite);
extern void portFixupBitmapArray(void *bitmaps, unsigned int count);
extern void portFixupSpriteBitmapData(void *sprite, void *bitmaps);

/* The asm quad is 400 by 400 with a 32 by 32 RGBA16 texture (Render.asm draw_stage_texture_ vertex_data and model_part_image). */
#define SC1PREMIX_ALLSTAR_REST_PORTRAIT_TEXELS 32
#define SC1PREMIX_ALLSTAR_REST_PORTRAIT_DISPLAY_LINK 6
#define SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES 4

typedef struct SC1PRemixAllstarRestPortrait
{
    void *texels;
    f32 x;
    Vtx vertices[SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES];

} SC1PRemixAllstarRestPortrait;

static SC1PRemixAllstarRestPortrait sSC1PRemixAllstarRestPortraits[SC1PREMIX_ALLSTAR_REST_PORTRAIT_SLOT_COUNT];

/**
 * Fills the corners of a portrait: bottom right, bottom left, top left and top right, the order of vertex_data.
 *
 * @param portrait Portrait to fill.
 */
static void sc1PRemixAllstarRestFillVertices(SC1PRemixAllstarRestPortrait *portrait)
{
    static const s16 corner_x[SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES] = { 1, -1, -1, 1 };
    static const s16 corner_y[SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES] = { -1, -1, 1, 1 };
    static const s16 corner_s[SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES] = { 1, 0, 0, 1 };
    static const s16 corner_t[SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES] = { 1, 1, 0, 0 };
    s32 i;

    for (i = 0; i < SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES; i++)
    {
        portrait->vertices[i].v.ob[0] = (s16)portrait->x + (corner_x[i] * SC1PREMIX_ALLSTAR_REST_PORTRAIT_HALF_SIZE);
        portrait->vertices[i].v.ob[1] = (s16)SC1PREMIX_ALLSTAR_REST_PORTRAIT_Y + (corner_y[i] * SC1PREMIX_ALLSTAR_REST_PORTRAIT_HALF_SIZE);
        portrait->vertices[i].v.ob[2] = (s16)SC1PREMIX_ALLSTAR_REST_PORTRAIT_Z;
        portrait->vertices[i].v.flag = 0;
        portrait->vertices[i].v.tc[0] = (corner_s[i] * SC1PREMIX_ALLSTAR_REST_PORTRAIT_TEXELS) << 5;
        portrait->vertices[i].v.tc[1] = (corner_t[i] * SC1PREMIX_ALLSTAR_REST_PORTRAIT_TEXELS) << 5;
        portrait->vertices[i].v.cn[0] = 0xFF;
        portrait->vertices[i].v.cn[1] = 0xFF;
        portrait->vertices[i].v.cn[2] = 0xFF;
        portrait->vertices[i].v.cn[3] = 0x00;
    }
}

/**
 * Display proc: the portrait texture on a translucent quad, tinted by the prim colour alpha of the asm (0xA0).
 *
 * @param gobj Portrait GObj, user_data points to its slot.
 */
static void sc1PRemixAllstarRestPortraitDisplay(GObj *gobj)
{
    SC1PRemixAllstarRestPortrait *portrait = (SC1PRemixAllstarRestPortrait*) gobj->user_data.p;

    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_ZB_XLU_SURF, G_RM_AA_ZB_XLU_SURF2);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, 0xFF, 0xFF, 0xFF, SC1PREMIX_ALLSTAR_REST_PORTRAIT_ALPHA);
    gDPSetCombineLERP(gSYTaskmanDLHeads[0]++, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0);
    gSPClearGeometryMode(gSYTaskmanDLHeads[0]++, G_LIGHTING | G_CULL_BOTH);
    gSPSetGeometryMode(gSYTaskmanDLHeads[0]++, G_ZBUFFER | G_SHADE);
    gSPTexture(gSYTaskmanDLHeads[0]++, 0xFFFF, 0xFFFF, 0, G_TX_RENDERTILE, G_ON);
    gDPLoadTextureBlock(gSYTaskmanDLHeads[0]++, portrait->texels, G_IM_FMT_RGBA, G_IM_SIZ_16b, SC1PREMIX_ALLSTAR_REST_PORTRAIT_TEXELS,
                        SC1PREMIX_ALLSTAR_REST_PORTRAIT_TEXELS, 0, G_TX_CLAMP, G_TX_CLAMP, 5, 5, G_TX_NOLOD, G_TX_NOLOD);
    gSPVertex(gSYTaskmanDLHeads[0]++, portrait->vertices, SC1PREMIX_ALLSTAR_REST_PORTRAIT_VERTICES, 0);
    gSP2Triangles(gSYTaskmanDLHeads[0]++, 3, 2, 1, 0, 0, 3, 1, 0);
    gSPTexture(gSYTaskmanDLHeads[0]++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
}

/**
 * Finds the texels of a portrait sprite, checking the format the asm assumes.
 *
 * @param sprite Portrait sprite of the Remix portrait file.
 * @return The RGBA16 texels, or NULL when the sprite is not a 32 by 32 RGBA16 picture.
 */
static void* sc1PRemixAllstarRestGetTexels(Sprite *sprite)
{
    Bitmap *bitmap;

    /* Same normalisation lbCommonMakeSObjForGObj does before a sprite is drawn: byte order of the structs and of the texels. */
    portFixupSprite(sprite);
    bitmap = (Bitmap*) PORT_RESOLVE(sprite->bitmap);

    if (bitmap != NULL)
    {
        portFixupBitmapArray(bitmap, (unsigned int)sprite->nbitmaps);
        portFixupSpriteBitmapData(sprite, bitmap);
    }
    if ((bitmap == NULL) || (sprite->bmfmt != G_IM_FMT_RGBA) || (sprite->bmsiz != G_IM_SIZ_16b) || (bitmap->width_img != SC1PREMIX_ALLSTAR_REST_PORTRAIT_TEXELS))
    {
        port_log("allstar: rest portrait skipped, fmt=%d siz=%d\n", (int)sprite->bmfmt, (int)sprite->bmsiz);
        return NULL;
    }
    return PORT_RESOLVE(bitmap->buf);
}

static void sc1PRemixAllstarRestMakePortrait(void *portrait_file, s32 slot, s32 count, s32 index, s32 fkind)
{
    SC1PRemixAllstarRestPortrait *portrait = &sSC1PRemixAllstarRestPortraits[slot];
    GObj *gobj;

    if ((fkind < 0) || (fkind >= REMIX_CSS_NUM_CHARACTERS))
    {
        return;
    }
    portrait->texels = sc1PRemixAllstarRestGetTexels(lbRelocGetFileData(Sprite*, portrait_file, kRemixCssPortraitOffset[fkind]));

    if (portrait->texels == NULL)
    {
        return;
    }
    portrait->x = sc1PRemixAllstarRestPortraitX(count, index);
    sc1PRemixAllstarRestFillVertices(portrait);

    gobj = gcMakeGObjSPAfter(nGCCommonKindEffect, NULL, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT);
    gobj->user_data.p = portrait;
    gcAddGObjDisplay(gobj, sc1PRemixAllstarRestPortraitDisplay, SC1PREMIX_ALLSTAR_REST_PORTRAIT_DISPLAY_LINK, GOBJ_PRIORITY_DEFAULT, ~0);
}

void sc1PRemixAllstarRestMakePortraits(void)
{
    s32 progress = sc1PRemixAllstarGetProgress();
    s32 count = sc1PRemixAllstarRestPortraitCount(sc1PRemixAllstarOpponentsAt(progress), SC1PREMIX_ALLSTAR_FIGHTER_COUNT, progress);
    void *portrait_file;
    s32 index;

    if ((sc1PRemixAllstarIsRestArea() == FALSE) || (count == 0))
    {
        return;
    }
    portrait_file = lbRelocGetExternHeapFile(REMIX_CSS_PORTRAITS_FILE_ID, syTaskmanMalloc(lbRelocGetFileSize(REMIX_CSS_PORTRAITS_FILE_ID), 0x10));

    for (index = 0; index < count; index++)
    {
        sc1PRemixAllstarRestMakePortrait(portrait_file, index, count, index, sc1PRemixAllstarGetFkind(progress + index));
    }
}

#endif
