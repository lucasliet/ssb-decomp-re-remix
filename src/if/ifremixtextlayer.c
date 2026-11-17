/**
 * ifremixtextlayer.c - Remix text drawn by the game renderer in a display link, see ifremixtextlayer.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixtextlayer.h"
#include "if/ifremixsprite.h"
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <sys/objman.h>
#include <sys/taskman.h>

#include <port_log.h>

#include "remix/remix_font_decode.h"
#include "remix/remix_hud_layer.h"

extern void portFixupSprite(void *sprite);
extern void portFixupBitmapArray(void *bitmaps, unsigned int count);
extern void portFixupSpriteBitmapData(void *sprite, void *bitmaps);

/* File.CUSTOM_FONT, the Kabel font: one 16 x 16 IA8 sprite per character. */
#define IFREMIX_TEXTLAYER_FONT_FILE 0xA15
/* Render.draw_string_ with blur on: sh 0x0201 at +0x24 of every character image. */
#define IFREMIX_TEXTLAYER_GLYPH_ATTR 0x0201

static void *sIFRemixTextLayerFontFile;

/** Sprite header of a character in the font file. */
static Sprite* ifRemixTextLayerGetGlyphSprite(s32 character)
{
    return lbRelocGetFileData
    (
        Sprite*,
        sIFRemixTextLayerFontFile,
        REMIX_FONT_FIRST_SPRITE_OFFSET + (REMIX_FONT_GLYPH_BLOCK_SIZE * (character - REMIX_FONT_FIRST_CHARACTER))
    );
}

/**
 * Loads the font file into the scene heap and puts every glyph sprite in host byte order, what lbCommonMakeSObjForGObj does
 * for the sprite of an SObj. The fixups keep track of what they already did, so a second layer of the scene costs nothing.
 *
 * @return FALSE when the file could not be loaded.
 */
static sb32 ifRemixTextLayerLoadFont(void)
{
    s32 character;

    sIFRemixTextLayerFontFile = ifRemixSpriteGetFile(IFREMIX_TEXTLAYER_FONT_FILE);

    if (sIFRemixTextLayerFontFile == NULL)
    {
        return FALSE;
    }
    for (character = REMIX_FONT_FIRST_CHARACTER; character <= REMIX_FONT_LAST_CHARACTER; character++)
    {
        Sprite *sprite = ifRemixTextLayerGetGlyphSprite(character);
        Bitmap *bitmaps;

        portFixupSprite(sprite);
        bitmaps = (Bitmap*)PORT_RESOLVE(sprite->bitmap);

        if (bitmaps != NULL)
        {
            portFixupBitmapArray(bitmaps, sprite->nbitmaps);
            portFixupSpriteBitmapData(sprite, bitmaps);
        }
    }
    return TRUE;
}

/**
 * Draws one glyph as the sprite of an SObj that lives only for this call: the sprite of the font file with the position,
 * scale and colour of the glyph, and the SObj defaults of lbCommonMakeSObjForGObj.
 *
 * @param glyph Character, top-left corner and scale.
 * @param rgba Colour of the string as 0xRRGGBBAA.
 */
static void ifRemixTextLayerDrawGlyph(const RemixHudLayerGlyph *glyph, u32 rgba)
{
    SObj sobj;

    sobj.sprite = *ifRemixTextLayerGetGlyphSprite(glyph->character);
    sobj.sprite.attr = IFREMIX_TEXTLAYER_GLYPH_ATTR;
    sobj.sprite.scalex = glyph->scalex;
    sobj.sprite.scaley = glyph->scaley;
    ifRemixSpriteRulesUnpackColor(rgba, &sobj.sprite.red, &sobj.sprite.green, &sobj.sprite.blue, &sobj.sprite.alpha);
    sobj.envcolor.r = sobj.envcolor.g = sobj.envcolor.b = sobj.envcolor.a = 0x00;
    sobj.maskt = sobj.masks = 0;
    sobj.cmt = sobj.cms = 2;
    sobj.lrs = sobj.lrt = 0;
    sobj.pos.x = glyph->x;
    sobj.pos.y = glyph->y;

    lbCommonPrepSObjAttr(gSYTaskmanDLHeads, &sobj);
    lbCommonPrepSObjDraw(gSYTaskmanDLHeads, &sobj);
    lbCommonSetExternSpriteParams(&sobj.sprite);
}

/** Draws the labels of the layer in slot order, so a higher slot is over a lower one. */
static void ifRemixTextLayerDrawLabels(s32 layer)
{
    RemixHudLayerGlyph glyphs[REMIX_HUD_LAYER_GLYPHS_MAX];
    uint32_t rgba;
    s32 slot;
    s32 count;
    s32 i;

    for (slot = 0; slot < REMIX_HUD_TEXT_SLOTS; slot++)
    {
        count = remix_hud_layer_label(layer, slot, &rgba, glyphs, ARRAY_COUNT(glyphs));

        for (i = 0; i < count; i++)
        {
            ifRemixTextLayerDrawGlyph(&glyphs[i], rgba);
        }
    }
}

/**
 * Fills the rectangles and bars of the layer (Render.draw_rectangle_ with alpha on: primitive colour, G_RM_CLD_SURF), on
 * whole pixels as the asm takes them. The sprite state is forgotten at the end, as the asm does, so the next sprite sets
 * the render mode up again.
 */
static void ifRemixTextLayerDrawRects(s32 layer)
{
    RemixHudLayerRect rects[REMIX_HUD_LAYER_RECTS_MAX];
    s32 count = remix_hud_layer_rects(layer, rects, ARRAY_COUNT(rects));
    s32 i;

    if (count == 0)
    {
        return;
    }
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_CLD_SURF, G_RM_CLD_SURF2);

    for (i = 0; i < count; i++)
    {
        RemixHudLayerPixelRect pixels;
        u32 rgba = rects[i].rgba;

        if (remix_hud_layer_rect_pixels(rects[i].x, rects[i].y, rects[i].width, rects[i].height, &pixels) == 0)
        {
            continue;
        }
        gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, (rgba >> 24) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF, rgba & 0xFF);
        gDPFillRectangle(gSYTaskmanDLHeads[0]++, pixels.ulx, pixels.uly, pixels.lrx, pixels.lry);
    }
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    lbCommonClearExternSpriteParams();
}

/**
 * Display function of a layer: rectangles, bars, then labels.
 *
 * @param layer_gobj Layer GObj, user_data.s is its layer id.
 */
static void ifRemixTextLayerProcDisplay(GObj *layer_gobj)
{
    s32 layer = layer_gobj->user_data.s;

    ifRemixTextLayerDrawRects(layer);
    ifRemixTextLayerDrawLabels(layer);
}

GObj* ifRemixTextLayerMake(s32 gobj_link, s32 dl_link, u32 dl_priority)
{
    GObj *layer_gobj;
    s32 layer;

    if (ifRemixTextLayerLoadFont() == FALSE)
    {
        port_log("ifRemixTextLayer: font file 0x%X not loaded, the text of display link %d is not drawn\n",
                 IFREMIX_TEXTLAYER_FONT_FILE, (int)dl_link);
        return NULL;
    }
    layer = remix_hud_layer_open();

    if (layer == REMIX_HUD_LAYER_NONE)
    {
        return NULL;
    }
    layer_gobj = gcMakeGObjSPAfter(0, NULL, gobj_link, GOBJ_PRIORITY_DEFAULT);
    layer_gobj->user_data.s = layer;
    gcAddGObjDisplay(layer_gobj, ifRemixTextLayerProcDisplay, dl_link, dl_priority, ~0);

    return layer_gobj;
}

/** Binds a slot range of one kind to the layer; a NULL layer binds nothing. */
static void ifRemixTextLayerBind(GObj *layer_gobj, s32 kind, s32 first_slot, s32 count)
{
    if (layer_gobj == NULL)
    {
        return;
    }
    remix_hud_layer_bind(layer_gobj->user_data.s, kind, first_slot, count);
}

void ifRemixTextLayerBindLabels(GObj *layer_gobj, s32 first_slot, s32 count)
{
    ifRemixTextLayerBind(layer_gobj, REMIX_HUD_SLOT_TEXT, first_slot, count);
}

void ifRemixTextLayerBindRects(GObj *layer_gobj, s32 first_slot, s32 count)
{
    ifRemixTextLayerBind(layer_gobj, REMIX_HUD_SLOT_RECT, first_slot, count);
}

void ifRemixTextLayerBindBars(GObj *layer_gobj, s32 first_slot, s32 count)
{
    ifRemixTextLayerBind(layer_gobj, REMIX_HUD_SLOT_BAR, first_slot, count);
}

#endif
