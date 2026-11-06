/**
 * mnremixdatagallery.c - Gallery button of the Data screen, see mnremixdatagallery.h.
 *
 * Gallery.asm add_button_ makes the Sound Test button once more and moves its four pieces down the staircase of the other tabs: the
 * left, middle and right pieces of the tab (x 37, 53 and 181 at y 183) and the text (x 80, y 187). The text keeps the Sound Test
 * sprite and takes the bitmap at 0x4EE8 of the Data menu file (the word at 0x80133164 is sMNDataFiles[1]; the asm comment says
 * "file 0x0003"), 22 pixels high.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/menu.h>
#include <mn/mndata/mndata.h>
#include <mn/mndata/mnremixdatagallery.h>
#include <lb/library.h>
#include <reloc_data.h>

extern void portFixupBitmapArray(void *bitmaps, unsigned int count);
extern void portFixupSpriteBitmapData(void *sprite, void *bitmaps);

#define MNREMIX_DATA_GALLERY_TAB_X 37.0F
#define MNREMIX_DATA_GALLERY_TAB_Y 183.0F
#define MNREMIX_DATA_GALLERY_TAB_LRS 16
#define MNREMIX_DATA_GALLERY_TEXT_X 80.0F
#define MNREMIX_DATA_GALLERY_TEXT_Y 187.0F
#define MNREMIX_DATA_GALLERY_LABEL_BITMAP_OFFSET 0x4EE8
#define MNREMIX_DATA_GALLERY_LABEL_HEIGHT 22
#define MNREMIX_DATA_GALLERY_DISPLAY_LINK 2
#define MNREMIX_DATA_GALLERY_GOBJ_LINK 4

/**
 * Points the text sprite at the Gallery label bitmap of the Data menu file and sets its height.
 *
 * @param sobj Text SObj made from the Sound Test sprite.
 * @param data_file Data menu file.
 */
static void mnRemixDataGallerySetLabel(SObj *sobj, void *data_file)
{
    Bitmap *label = lbRelocGetFileData(Bitmap*, data_file, MNREMIX_DATA_GALLERY_LABEL_BITMAP_OFFSET);

    portFixupBitmapArray(label, 1);
    portFixupSpriteBitmapData(&sobj->sprite, label);

    sobj->sprite.bitmap = portRelocRegisterPointer(label);
    sobj->sprite.height = MNREMIX_DATA_GALLERY_LABEL_HEIGHT;
    sobj->sprite.bmheight = MNREMIX_DATA_GALLERY_LABEL_HEIGHT;
    sobj->sprite.bmHreal = MNREMIX_DATA_GALLERY_LABEL_HEIGHT;
}

GObj* mnRemixDataGalleryMakeButton(void *data_file, sb32 is_highlight)
{
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, MNREMIX_DATA_GALLERY_GOBJ_LINK, GOBJ_PRIORITY_DEFAULT);
    SObj *sobj;

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, MNREMIX_DATA_GALLERY_DISPLAY_LINK, GOBJ_PRIORITY_DEFAULT, ~0);

    mnDataMakeOptionTab(gobj, MNREMIX_DATA_GALLERY_TAB_X, MNREMIX_DATA_GALLERY_TAB_Y, MNREMIX_DATA_GALLERY_TAB_LRS);
    mnDataSetOptionSpriteColors(gobj, (is_highlight != FALSE) ? nMNOptionTabStatusHighlight : nMNOptionTabStatusNot);

    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, data_file, llMNDataSoundTestTextSprite));

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->sprite.red = 0x00;
    sobj->sprite.green = 0x00;
    sobj->sprite.blue = 0x00;

    sobj->pos.x = MNREMIX_DATA_GALLERY_TEXT_X;
    sobj->pos.y = MNREMIX_DATA_GALLERY_TEXT_Y;

    mnRemixDataGallerySetLabel(sobj, data_file);

    return gobj;
}

#endif
