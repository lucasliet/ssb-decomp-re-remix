/**
 * ftremixkirbyhats.c - Smash Remix Kirby hats (src/KirbyHats.asm).
 *
 * Kirby's file has 0xF model parts for the hat joint. Remix adds 33 more hats: hat id 0xF + n reads row n of
 * KirbyHats.extended_special_parts (0x20 bytes): base hat id (-1 = none), then display list, special images and special
 * tracks offsets for high poly and low poly, and the file the offsets belong to (KIRBY_CUSTOM_HATS or _2). The part
 * starts as a copy of the base hat's model part and the offsets that are not -1 replace its display list, images
 * (mobjsubs) and costume tracks.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmremixpanel.h>
#include <lb/lbreloc.h>
#include <sys/taskman.h>
#include <sys/objtypes.h>
#include <sys/utils.h>
#include <port_log.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define FTREMIX_KIRBYHATS_VANILLA_COUNT 0x0F
#define FTREMIX_KIRBYHATS_NEW_COUNT 33
#define FTREMIX_KIRBYHATS_ROW_SIZE 0x20
#define FTREMIX_KIRBYHATS_FILES 2
#define FTREMIX_KIRBYHATS_PLAYERS 4
#define FTREMIX_KIRBYHATS_HAT_JOINT 6
#define FTREMIX_KIRBYHATS_NO_OFFSET 0xFFFFFFFFu
#define FTREMIX_KIRBYHATS_PANEL_NONE 0
#define FTREMIX_KIRBYHATS_PANEL_MAGIC 41
#define FTREMIX_KIRBYHATS_PANEL_RANDOM 42
#define FTREMIX_KIRBYHATS_PANEL_FIGHTERS 40

static const u32 sFTRemixKirbyHatsFileIds[FTREMIX_KIRBYHATS_FILES] = { 0x0A8D, 0x1533 }; /* File.KIRBY_CUSTOM_HATS, _2 */

static void *sFTRemixKirbyHatsFiles[FTREMIX_KIRBYHATS_FILES];
static FTModelPart sFTRemixKirbyHatsParts[FTREMIX_KIRBYHATS_PLAYERS][2];

static u32 ftRemixKirbyHatsReadWord(u32 ram_addr)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr);

    return (word != NULL) ? *word : 0;
}

void ftRemixKirbyHatsClearCache(void)
{
    s32 i;

    for (i = 0; i < FTREMIX_KIRBYHATS_FILES; i++)
    {
        sFTRemixKirbyHatsFiles[i] = NULL;
    }
}

static void* ftRemixKirbyHatsGetFile(s32 index)
{
    u32 file_id = sFTRemixKirbyHatsFileIds[index];

    if (sFTRemixKirbyHatsFiles[index] == NULL)
    {
        sFTRemixKirbyHatsFiles[index] = lbRelocGetExternHeapFile(file_id, syTaskmanMalloc(lbRelocGetFileSize(file_id), 0x10));
        port_log("[remix-kirbyhat] loaded file 0x%X at %p\n", file_id, sFTRemixKirbyHatsFiles[index]);
    }
    return sFTRemixKirbyHatsFiles[index];
}

static sb32 ftRemixKirbyHatsIsKirby(FTStruct *fp)
{
    return (fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY);
}

/** Pointer of a file offset as a registered token, or keep_token when the offset is not defined. */
static u32 ftRemixKirbyHatsOverride(void *file, u32 offset, u32 keep_token)
{
    if ((offset == FTREMIX_KIRBYHATS_NO_OFFSET) || (file == NULL))
    {
        return keep_token;
    }
    return PORT_REGISTER((u8*)file + offset);
}

FTModelPart* ftRemixKirbyHatModelPart(FTStruct *fp, s32 joint_id, s32 modelpart_id, FTModelPartDesc *desc, s32 detail_index)
{
    u32 row_addr;
    FTModelPart *part;
    u32 base_hat;
    u32 file_index;
    u32 first_word;
    void *file;

    if ((joint_id != FTREMIX_KIRBYHATS_HAT_JOINT) || (modelpart_id < FTREMIX_KIRBYHATS_VANILLA_COUNT) || !ftRemixKirbyHatsIsKirby(fp))
    {
        return &desc->modelparts[modelpart_id][detail_index];
    }
    if (((modelpart_id - FTREMIX_KIRBYHATS_VANILLA_COUNT) >= FTREMIX_KIRBYHATS_NEW_COUNT) || (fp->player >= FTREMIX_KIRBYHATS_PLAYERS))
    {
        return &desc->modelparts[0][detail_index];
    }
    row_addr = REMIX_SYM_KirbyHats__extended_special_parts + ((modelpart_id - FTREMIX_KIRBYHATS_VANILLA_COUNT) * FTREMIX_KIRBYHATS_ROW_SIZE);
    base_hat = ftRemixKirbyHatsReadWord(row_addr);
    file_index = ftRemixKirbyHatsReadWord(row_addr + 0x1C) & 1;
    part = &sFTRemixKirbyHatsParts[fp->player][detail_index];
    *part = desc->modelparts[((base_hat < FTREMIX_KIRBYHATS_VANILLA_COUNT) ? base_hat : 0)][detail_index];
    file = ftRemixKirbyHatsGetFile(file_index);
    first_word = row_addr + 4 + (detail_index * 0xC);
    part->dl = ftRemixKirbyHatsOverride(file, ftRemixKirbyHatsReadWord(first_word), part->dl);
    part->mobjsubs = ftRemixKirbyHatsOverride(file, ftRemixKirbyHatsReadWord(first_word + 4), part->mobjsubs);
    part->costume_matanim_joints = ftRemixKirbyHatsOverride(file, ftRemixKirbyHatsReadWord(first_word + 8), part->costume_matanim_joints);

    return part;
}

/* KirbyHats.asm spawn_with_table_: Remix fighter id of each CSS Kirby Hat value 0..41 (Random, 42, is rolled below). */
static const u8 sFTRemixKirbyHatsPanelFighter[FTREMIX_KIRBYHATS_PANEL_MAGIC + 1] = {
    REMIX_ID_KIRBY, REMIX_ID_MARIO, REMIX_ID_FOX, REMIX_ID_DONKEY, REMIX_ID_SAMUS, REMIX_ID_LUIGI, REMIX_ID_LINK,
    REMIX_ID_YOSHI, REMIX_ID_CAPTAIN, REMIX_ID_PIKACHU, REMIX_ID_JIGGLY, REMIX_ID_NESS, REMIX_ID_FALCO, REMIX_ID_GND,
    REMIX_ID_YLINK, REMIX_ID_DRM, REMIX_ID_WARIO, 0x34, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3E, REMIX_ID_MARINA,
    REMIX_ID_DEDEDE, REMIX_ID_GOEMON, REMIX_ID_BANJO, REMIX_ID_CRASH, REMIX_ID_PEACH, 0x22, 0x26, REMIX_ID_PEPPY,
    REMIX_ID_SLIPPY, REMIX_ID_ROY, REMIX_ID_DRL, REMIX_ID_LANKY, REMIX_ID_DRAGONKING, REMIX_ID_EBI, 0x36, REMIX_ID_SSONIC,
    REMIX_ID_NONE,
};

/**
 * Remix fighter id behind a CSS Kirby Hat value 1..42. Random rolls one of the 40 fighters (1..40), never None or magic,
 * as asm pick_random_hat_ does.
 */
static s32 ftRemixKirbyHatsPanelFighterID(s32 panel_value)
{
    if (panel_value == FTREMIX_KIRBYHATS_PANEL_RANDOM)
    {
        return sFTRemixKirbyHatsPanelFighter[1 + syUtilsRandIntRange(FTREMIX_KIRBYHATS_PANEL_FIGHTERS)];
    }
    return sFTRemixKirbyHatsPanelFighter[panel_value];
}

/**
 * Hat modelpart a Kirby or J Kirby spawns with when its port has a CSS Kirby Hat value (asm kirby_hat_select_): the copy
 * hat of the chosen fighter, magic is the None row. Returns -1 when the spawn keeps the vanilla hat, that is no panel value
 * or a fighter that already holds a copy power.
 */
s32 ftRemixKirbyHatPanelModelPart(FTStruct *fp)
{
    s32 panel_value;

    if (!ftRemixKirbyHatsIsKirby(fp) || (fp->passive_vars.kirby.copy_id != nFTKindKirby))
    {
        return -1;
    }
    panel_value = gmRemixPanel(KIRBY_HAT, fp->player);
    if (panel_value == FTREMIX_KIRBYHATS_PANEL_NONE)
    {
        return -1;
    }
    return ftRemixKirbyGetCopyTable()[ftRemixKirbyHatsPanelFighterID(panel_value)].copy_modelpart_id;
}

#endif
