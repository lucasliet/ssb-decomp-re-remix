/**
 * ftremixpartimage.c - extra costumes by swapping the image fields of model part MObjs (Costumes.asm check_for_custom_costume_).
 *
 * The asm hooks lbCommonAddMObjForFighterPartsDObj (RAM 0x800C8CB8): after each MObj of a part is made and its costume matanim
 * played, it looks the part up in `Costumes.extra_costumes_table[fkind]` and overwrites the MObj's palette array, prim color and
 * diffuse/ambient colors with the arrays of the extra costume. The port cannot name the part inside that function, so it applies
 * the same writes to the joints of the fighter before the fighter is drawn (ftRemixPartImageApplyFighter); the result is the same
 * because the extra costume's matanim frame is past the last frame of the vanilla matanim, which leaves nothing the swap depends on.
 *
 * Part table (Costumes.asm define_part): byte 0 number of extra costumes, byte 1 part id and byte 2 first image index of the
 * accessory part (the separate FighterParts GObj, id 0x3E9), byte 3 skipped costumes, then 8 bytes per part id: the images array
 * and the special parts array. An image is 0x10 bytes: palette array, prim color array (8 bytes per costume, high and low poly),
 * texture array array, diffuse/ambient array (one pointer to a diffuse/ambient word pair per costume).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "remix/remix_debug.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixpartimage.h"
#include "ft/ftremix/ftremixpartimagepalette.h"
#include "ft/ftremix/ftremixpartimagerules.h"

/* Part 0 never has images: its 8 bytes hold the header described above. */
#define FTREMIX_PARTIMAGE_HEADER_PART 0
#define FTREMIX_PARTIMAGE_HEADER_EXTRA_COUNT 0
#define FTREMIX_PARTIMAGE_HEADER_ACCESSORY_PART 1
#define FTREMIX_PARTIMAGE_HEADER_ACCESSORY_IMAGE 2
#define FTREMIX_PARTIMAGE_HEADER_SKIPPED 3
#define FTREMIX_PARTIMAGE_SPECIAL_ARRAY_OFFSET 4
#define FTREMIX_PARTIMAGE_PALETTE_OFFSET 0x0
#define FTREMIX_PARTIMAGE_PRIM_COLOR_OFFSET 0x4
#define FTREMIX_PARTIMAGE_DIFFUSE_AMBIENT_OFFSET 0xC
#define FTREMIX_PARTIMAGE_SAMUS_BODY_PART 2
#define FTREMIX_PARTIMAGE_NO_SPECIAL (-1)

typedef struct FTRemixPartImageCostume
{
    u32 parts_table;
    s32 extra_count;
    s32 extra_index;
    sb32 is_low_poly;
    s32 fkind;

} FTRemixPartImageCostume;

typedef struct FTRemixPartImageTarget
{
    s32 part_id;
    s32 special_index;
    sb32 is_accessory;
    s32 accessory_image_start;

} FTRemixPartImageTarget;

/** Part table of `fkind` in the RAM image, or 0 when the character has no extra costumes. */
static u32 ftRemixPartImageGetPartsTable(s32 fkind)
{
    if ((fkind < 0) || (fkind >= FTREMIX_PARTIMAGE_CHARACTER_COUNT))
    {
        return 0;
    }
    return remix_ram_image_read_word(REMIX_SYM_Costumes__extra_costumes_table + (fkind * sizeof(u32)));
}

/** Highest vanilla costume id of `fkind` (Costumes.select_.num_costumes). */
static s32 ftRemixPartImageGetNumCostumes(s32 fkind)
{
    return remix_ram_image_read_byte(REMIX_SYM_Costumes__select___num_costumes + fkind);
}

/** Costume numbers of a character: vanilla maximum, skipped ids and extra costumes. Zeroed extras and skips when it has no table. */
static void ftRemixPartImageGetCostumeCounts(s32 fkind, s32 *num_costumes, s32 *skipped, s32 *extra_count)
{
    u32 parts_table = ftRemixPartImageGetPartsTable(fkind);

    *num_costumes = 0;
    *skipped = 0;
    *extra_count = 0;

    if ((fkind < 0) || (fkind >= FTREMIX_PARTIMAGE_CHARACTER_COUNT))
    {
        return;
    }
    *num_costumes = ftRemixPartImageGetNumCostumes(fkind);

    if (parts_table == 0)
    {
        return;
    }
    *skipped = remix_ram_image_read_byte(parts_table + FTREMIX_PARTIMAGE_HEADER_SKIPPED);
    *extra_count = remix_ram_image_read_byte(parts_table + FTREMIX_PARTIMAGE_HEADER_EXTRA_COUNT);
}

/** Number of extra costumes of `fkind`. */
s32 ftRemixPartImageGetExtraCostumeCount(s32 fkind)
{
    s32 num_costumes, skipped, extra_count;

    ftRemixPartImageGetCostumeCounts(fkind, &num_costumes, &skipped, &extra_count);

    return extra_count;
}

/** Highest costume id a player can reach with `fkind`. */
s32 ftRemixPartImageGetMaxCostumeID(s32 fkind)
{
    s32 num_costumes, skipped, extra_count;

    ftRemixPartImageGetCostumeCounts(fkind, &num_costumes, &skipped, &extra_count);

    return ftRemixPartImageRulesMaxCostume(num_costumes, skipped, extra_count);
}

/** Costume after a C right press. */
s32 ftRemixPartImageGetNextCostume(s32 fkind, s32 costume)
{
    s32 num_costumes, skipped, extra_count;

    ftRemixPartImageGetCostumeCounts(fkind, &num_costumes, &skipped, &extra_count);

    return ftRemixPartImageRulesNextCostume(costume, num_costumes, skipped, extra_count);
}

/** Costume after a C left press. */
s32 ftRemixPartImageGetPrevCostume(s32 fkind, s32 costume)
{
    s32 num_costumes, skipped, extra_count;

    ftRemixPartImageGetCostumeCounts(fkind, &num_costumes, &skipped, &extra_count);

    return ftRemixPartImageRulesPrevCostume(costume, num_costumes, skipped, extra_count);
}

/** TRUE when a Random pick may land on `costume`. */
sb32 ftRemixPartImageIsLegalRandomCostume(s32 fkind, s32 costume)
{
    s32 num_costumes, skipped, extra_count;

    ftRemixPartImageGetCostumeCounts(fkind, &num_costumes, &skipped, &extra_count);

    return ftRemixPartImageRulesIsLegalRandomCostume(costume, num_costumes, skipped);
}

/** Fills `costume` for a fighter showing an extra costume. Returns FALSE for a vanilla costume or a character with no extras. */
static sb32 ftRemixPartImageInitCostume(FTRemixPartImageCostume *costume, const FTStruct *fp)
{
    s32 num_costumes, skipped, extra_count;

    costume->parts_table = ftRemixPartImageGetPartsTable(fp->fkind);

    if (costume->parts_table == 0)
    {
        return FALSE;
    }
    ftRemixPartImageGetCostumeCounts(fp->fkind, &num_costumes, &skipped, &extra_count);

    costume->extra_index = ftRemixPartImageRulesExtraIndex(fp->costume, num_costumes, skipped, extra_count);

    if (costume->extra_index == FTREMIX_PARTIMAGE_NO_EXTRA)
    {
        return FALSE;
    }
    costume->extra_count = extra_count;
    costume->is_low_poly = (fp->detail_curr == nFTPartsDetailLow);
    costume->fkind = fp->fkind;

    return TRUE;
}

/**
 * N64 address of the images array of a part (check_for_custom_costume_ _get_images_array): the special part's array when the
 * part shows a special model part and lists one, else the normal one. 0 when the part has no images.
 */
static u32 ftRemixPartImageGetImagesAddr(const FTRemixPartImageCostume *costume, const FTRemixPartImageTarget *target)
{
    u32 entry = costume->parts_table + (target->part_id * FTREMIX_PARTIMAGE_PART_ENTRY_SIZE);
    u32 specials;

    if (!ftRemixPartImageRulesUsesSpecialArray(target->special_index))
    {
        return remix_ram_image_read_word(entry);
    }
    specials = remix_ram_image_read_word(entry + FTREMIX_PARTIMAGE_SPECIAL_ARRAY_OFFSET);

    if (specials == 0)
    {
        return remix_ram_image_read_word(entry);
    }
    return remix_ram_image_read_word(specials + (target->special_index * sizeof(u32)));
}

/** Index of the image an MObj uses, its position in the part's MObj list or the accessory part's image slot. */
static s32 ftRemixPartImageGetImageIndex(const FTRemixPartImageTarget *target, const DObj *dobj, const MObj *mobj, s32 position)
{
    if (target->is_accessory)
    {
        return ftRemixPartImageRulesAccessoryImageIndex(target->accessory_image_start, mobj == dobj->mobj);
    }
    return position;
}

/**
 * The low poly Samus body (part 2) lists its two images in the opposite order (check_for_custom_costume_ SAMUS FRIGGIN ARAN):
 * image 0 reads the second info slot and image 1 the first one.
 */
static u32 ftRemixPartImageSwapSamusLowPolyImage(const FTRemixPartImageCostume *costume, const FTRemixPartImageTarget *target, s32 image_index, u32 image_addr)
{
    if ((costume->fkind != REMIX_ID_SAMUS) && (costume->fkind != REMIX_ID_ESAMUS) && (costume->fkind != REMIX_ID_JSAMUS))
    {
        return image_addr;
    }
    if (!costume->is_low_poly || (target->part_id != FTREMIX_PARTIMAGE_SAMUS_BODY_PART) || (target->special_index > 0) || (image_index > 1))
    {
        return image_addr;
    }
    return (image_index == 0) ? (image_addr + FTREMIX_PARTIMAGE_IMAGE_INFO_SIZE) : (image_addr - FTREMIX_PARTIMAGE_IMAGE_INFO_SIZE);
}

/** Writes an 0xRRGGBBAA word of the image into a port color. */
static void ftRemixPartImageSetColor(SYColorPack *color, u32 rgba)
{
    color->s.r = (rgba >> 24) & 0xFF;
    color->s.g = (rgba >> 16) & 0xFF;
    color->s.b = (rgba >> 8) & 0xFF;
    color->s.a = rgba & 0xFF;
}

/** Palette array of the extra costume, and the palette index that selects the costume's palette in it. */
static void ftRemixPartImageApplyPalette(const FTRemixPartImageCostume *costume, MObj *mobj, u32 image_addr)
{
    u32 array_addr = remix_ram_image_read_word(image_addr + FTREMIX_PARTIMAGE_PALETTE_OFFSET);
    u32 token;

    if (array_addr == 0)
    {
        return;
    }
    token = ftRemixPartImageGetPaletteArrayToken(array_addr, costume->extra_count);

    if (token == 0)
    {
        return;
    }
    mobj->sub.palettes = token;
    mobj->palette_id = costume->extra_index;
}

/** Prim color of the extra costume, the low poly one when the model is low poly and defines it. */
static void ftRemixPartImageApplyPrimColor(const FTRemixPartImageCostume *costume, MObj *mobj, u32 image_addr)
{
    u32 array_addr = remix_ram_image_read_word(image_addr + FTREMIX_PARTIMAGE_PRIM_COLOR_OFFSET);
    u32 entry_addr;

    if (array_addr == 0)
    {
        return;
    }
    entry_addr = array_addr + (costume->extra_index * FTREMIX_PARTIMAGE_PRIM_COLOR_SIZE);

    ftRemixPartImageSetColor
    (
        &mobj->sub.primcolor,
        ftRemixPartImageRulesPickPrimColor(remix_ram_image_read_word(entry_addr), remix_ram_image_read_word(entry_addr + sizeof(u32)), costume->is_low_poly)
    );
}

/** Diffuse and ambient colors of the extra costume (the MObj light colors 1 and 2). */
static void ftRemixPartImageApplyLightColors(const FTRemixPartImageCostume *costume, MObj *mobj, u32 image_addr)
{
    u32 array_addr = remix_ram_image_read_word(image_addr + FTREMIX_PARTIMAGE_DIFFUSE_AMBIENT_OFFSET);
    u32 pair_addr;

    if (array_addr == 0)
    {
        return;
    }
    pair_addr = remix_ram_image_read_word(array_addr + (costume->extra_index * sizeof(u32)));

    if (pair_addr == 0)
    {
        return;
    }
    ftRemixPartImageSetColor(&mobj->sub.light1color, remix_ram_image_read_word(pair_addr));
    ftRemixPartImageSetColor(&mobj->sub.light2color, remix_ram_image_read_word(pair_addr + sizeof(u32)));
}

/**
 * Applies the fields the MObj's flags say it has. The texture array branch of the asm (flag 1, MObj +0x0C) reads
 * `part_type.TEXTURE_ARRAY` arrays that no Remix costume file fills (no `set_texture_array` macro exists), so every entry is 0 and
 * the asm leaves the sprites alone; the port does the same.
 */
static void ftRemixPartImageApplyToMObj(const FTRemixPartImageCostume *costume, MObj *mobj, u32 image_addr)
{
    if (mobj->sub.flags & MOBJ_FLAG_PALETTE)
    {
        ftRemixPartImageApplyPalette(costume, mobj, image_addr);
    }
    if (mobj->sub.flags & MOBJ_FLAG_PRIMCOLOR)
    {
        ftRemixPartImageApplyPrimColor(costume, mobj, image_addr);
    }
    if (mobj->sub.flags & (MOBJ_FLAG_LIGHT1 | MOBJ_FLAG_LIGHT2))
    {
        ftRemixPartImageApplyLightColors(costume, mobj, image_addr);
    }
}

/** Applies the extra costume to every MObj of one part DObj. */
static void ftRemixPartImageApplyDObj(const FTRemixPartImageCostume *costume, const FTRemixPartImageTarget *target, DObj *dobj)
{
    u32 images_addr;
    MObj *mobj;
    s32 position = 0;
    s32 image_index;
    u32 image_addr;

    if (target->part_id == FTREMIX_PARTIMAGE_HEADER_PART)
    {
        return;
    }
    images_addr = ftRemixPartImageGetImagesAddr(costume, target);

    if (images_addr == 0)
    {
        return;
    }
    for (mobj = dobj->mobj; mobj != NULL; mobj = mobj->next, position++)
    {
        image_index = ftRemixPartImageGetImageIndex(target, dobj, mobj, position);
        image_addr = ftRemixPartImageSwapSamusLowPolyImage(costume, target, image_index, images_addr + (image_index * FTREMIX_PARTIMAGE_IMAGE_INFO_SIZE));

        ftRemixPartImageApplyToMObj(costume, mobj, image_addr);
    }
}

/**
 * Special part index of a joint: the model part id it shows when it has special model parts (the asm only knows it for the special
 * part swap of ftParamSetModelPartID), else FTREMIX_PARTIMAGE_NO_SPECIAL.
 */
static s32 ftRemixPartImageGetSpecialIndex(const FTStruct *fp, s32 part_id)
{
    FTModelPartContainer *modelparts_container = (FTModelPartContainer*)PORT_RESOLVE(fp->attr->modelparts_container);

    if ((modelparts_container == NULL) || (FTMODELPARTCONTAINER_GET_DESC(modelparts_container, part_id) == NULL))
    {
        return FTREMIX_PARTIMAGE_NO_SPECIAL;
    }
    return fp->modelpart_status[part_id].modelpart_id_curr;
}

/** Applies the extra costume to the accessory part of a joint (the FighterParts GObj, its images start at table byte 2). */
static void ftRemixPartImageApplyAccessory(const FTRemixPartImageCostume *costume, GObj *accessory_gobj)
{
    FTRemixPartImageTarget target;

    target.part_id = remix_ram_image_read_byte(costume->parts_table + FTREMIX_PARTIMAGE_HEADER_ACCESSORY_PART);
    target.special_index = FTREMIX_PARTIMAGE_NO_SPECIAL;
    target.is_accessory = TRUE;
    target.accessory_image_start = remix_ram_image_read_byte(costume->parts_table + FTREMIX_PARTIMAGE_HEADER_ACCESSORY_IMAGE);

    ftRemixPartImageApplyDObj(costume, &target, DObjGetStruct(accessory_gobj));
}

/** Applies the extra costume to one joint of the fighter and to its accessory part. */
static void ftRemixPartImageApplyJoint(const FTRemixPartImageCostume *costume, const FTStruct *fp, s32 joint_id)
{
    DObj *joint = fp->joints[joint_id];
    FTParts *parts;
    FTRemixPartImageTarget target;

    if (joint == NULL)
    {
        return;
    }
    target.part_id = joint_id - nFTPartsJointCommonStart;
    target.special_index = ftRemixPartImageGetSpecialIndex(fp, target.part_id);
    target.is_accessory = FALSE;
    target.accessory_image_start = 0;

    ftRemixPartImageApplyDObj(costume, &target, joint);

    parts = ftGetParts(joint);

    if ((parts != NULL) && (parts->gobj != NULL) && (DObjGetStruct(parts->gobj) != NULL))
    {
        ftRemixPartImageApplyAccessory(costume, parts->gobj);
    }
}

/** Applies the extra costume of a fighter to its model parts. */
void ftRemixPartImageApplyFighter(FTStruct *fp)
{
    FTRemixPartImageCostume costume;
    s32 joint_id;

    if (!ftRemixPartImageInitCostume(&costume, fp))
    {
        return;
    }
    for (joint_id = nFTPartsJointCommonStart; joint_id < ARRAY_COUNT(fp->joints); joint_id++)
    {
        ftRemixPartImageApplyJoint(&costume, fp, joint_id);
    }
}

/**
 * Stock icon palette of a fighter in the match HUD (Costumes.asm icon_palette_override_). The Remix array of a character lists a
 * palette for every costume id from the first extra costume on, so the extra costume's id indexes it directly. Returns the LUT token
 * of the extra costume, or `vanilla_lut` for a vanilla costume or when the array lists no palette.
 */
u32 ftRemixPartImageGetStockIconLUT(const FTStruct *fp, u32 vanilla_lut)
{
    FTRemixPartImageCostume costume;
    u32 array_addr;
    u32 token;

    if (!ftRemixPartImageInitCostume(&costume, fp))
    {
        return vanilla_lut;
    }
    array_addr = remix_ram_image_read_word(REMIX_SYM_Costumes__stock_icon_palette_array_table + (fp->fkind * sizeof(u32)));

    if (array_addr == 0)
    {
        return vanilla_lut;
    }
    token = ftRemixPartImageGetPaletteToken(remix_ram_image_read_word(array_addr + (fp->costume * sizeof(u32))));

    return (token != 0) ? token : vanilla_lut;
}

/**
 * Debug override of the costume of a fighter at creation: SSB64_REMIX_COSTUME_P1 to SSB64_REMIX_COSTUME_P4 name the costume id of
 * that port, so the VS shortcut (SSB64_VS_FKINDS, which always makes costume 0) can show an extra costume. Returns `costume` when
 * the variable is unset or the port is outside 0 to 3.
 */
s32 ftRemixPartImageDebugCostume(s32 player, s32 costume)
{
    static const char *const names[] = { "SSB64_REMIX_COSTUME_P1", "SSB64_REMIX_COSTUME_P2", "SSB64_REMIX_COSTUME_P3", "SSB64_REMIX_COSTUME_P4" };

    if ((player < 0) || (player >= ARRAY_COUNT(names)))
    {
        return costume;
    }
    return remix_debug_env_int(names[player], costume);
}

#endif
