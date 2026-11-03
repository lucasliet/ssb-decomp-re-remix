/**
 * ifremixsprite.c - Remix button and extension sprites as native SObjs, see ifremixsprite.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixsprite.h"
#include <lb/lbcommon.h>
#include <lb/lbreloc.h>
#include <lb/library.h>
#include <sys/objman.h>
#include <sys/taskman.h>

#include <port_log.h>

#define IFREMIX_SPRITE_FILES_MAX 8
/* Render.draw_texture_ turns the blur attribute on for every texture (sh 0x0201 at +0x24). */
#define IFREMIX_SPRITE_ATTR 0x0201

typedef struct IFRemixSpriteFile
{
    u32 file_id;
    void *file;
} IFRemixSpriteFile;

static IFRemixSpriteFile sIFRemixSpriteFiles[IFREMIX_SPRITE_FILES_MAX];
static s32 sIFRemixSpriteFileCount;

void ifRemixSpriteSceneReset(void)
{
    sIFRemixSpriteFileCount = 0;
}

/** File data of a reloc file, loaded into the scene heap on first use. NULL when the file table is full. */
static void *ifRemixSpriteGetFile(u32 file_id)
{
    s32 i;
    void *file;

    for (i = 0; i < sIFRemixSpriteFileCount; i++)
    {
        if (sIFRemixSpriteFiles[i].file_id == file_id)
        {
            return sIFRemixSpriteFiles[i].file;
        }
    }
    if (sIFRemixSpriteFileCount >= IFREMIX_SPRITE_FILES_MAX)
    {
        port_log("ifRemixSprite: more than %d files, file 0x%X not loaded\n", IFREMIX_SPRITE_FILES_MAX, (unsigned int)file_id);
        return NULL;
    }
    file = lbRelocGetExternHeapFile(file_id, syTaskmanMalloc(lbRelocGetFileSize(file_id), 0x10));
    sIFRemixSpriteFiles[sIFRemixSpriteFileCount].file_id = file_id;
    sIFRemixSpriteFiles[sIFRemixSpriteFileCount].file = file;
    sIFRemixSpriteFileCount++;
    return file;
}

static void ifRemixSpriteSetColors(SObj *sobj, u32 tint_rgba, u32 palette_rgba)
{
    u8 alpha;

    ifRemixSpriteRulesUnpackColor(tint_rgba, &sobj->sprite.red, &sobj->sprite.green, &sobj->sprite.blue, &sobj->sprite.alpha);
    ifRemixSpriteRulesUnpackColor(palette_rgba, &sobj->envcolor.r, &sobj->envcolor.g, &sobj->envcolor.b, &alpha);
    sobj->envcolor.a = alpha;
}

GObj *ifRemixSpriteMakeFromFile(void *file, u32 sprite_offset, s32 gobj_link, s32 dl_link, f32 x, f32 y, u32 tint_rgba,
                                u32 palette_rgba, f32 scale)
{
    GObj *gobj;
    SObj *sobj;

    if (file == NULL)
    {
        return NULL;
    }
    gobj = gcMakeGObjSPAfter(0, NULL, gobj_link, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, dl_link, GOBJ_PRIORITY_DEFAULT, ~0);
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, file, sprite_offset));
    sobj->sprite.attr = IFREMIX_SPRITE_ATTR;
    sobj->sprite.scalex = scale;
    sobj->sprite.scaley = scale;
    sobj->pos.x = x;
    sobj->pos.y = y;
    ifRemixSpriteSetColors(sobj, tint_rgba, palette_rgba);
    return gobj;
}

GObj *ifRemixSpriteMake(s32 id, s32 gobj_link, s32 dl_link, f32 x, f32 y, u32 tint_rgba, u32 palette_rgba, f32 scale)
{
    const IFRemixSpriteEntry *entry = ifRemixSpriteRulesGetEntry(id);

    if (entry == NULL)
    {
        port_log("ifRemixSprite: sprite id %d is outside 0..%d\n", (int)id, IFREMIX_SPRITE_COUNT - 1);
        return NULL;
    }
    return ifRemixSpriteMakeFromFile(ifRemixSpriteGetFile(entry->file_id), entry->sprite_offset, gobj_link, dl_link, x, y,
                                     tint_rgba, palette_rgba, scale);
}

#endif
