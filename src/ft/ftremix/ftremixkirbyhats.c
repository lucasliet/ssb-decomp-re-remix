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
#include "ft/ftremix/ftremixkirbyhooks.h"
#include "ft/ftremix/ftremixkirbyspawnrules.h"
#include "ft/ftremix/ftremixfields.h"
#include <ft/fighter.h>
#include <ft/ftmanager.h>
#include <gm/gmremixpanel.h>
#include <lb/lbreloc.h>
#include <lb/library.h>
#include <sc/scdef.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
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
#define FTREMIX_KIRBYHATS_GUN_JOINT 0x11
#define FTREMIX_KIRBYHATS_NO_OFFSET 0xFFFFFFFFu
#define FTREMIX_KIRBYHATS_COPY_ROWS 97

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

/**
 * Held-weapon part of a Kirby or J Kirby wearing a hat with a weapon (KirbyHats.asm use_extended_special_parts_pause_, gun
 * branch): the part table is the one of the hat in Kirby's model file, indexed by the same model part id as Kirby's own.
 *
 * @param fp Kirby or J Kirby.
 * @param modelpart_id Model part id of the weapon joint.
 * @param desc Part table of Kirby's weapon joint.
 * @param detail_index High or low poly index.
 * @return The part of the hat's table, or Kirby's own part when the hat has no weapon or the model file is not loaded.
 */
static FTModelPart* ftRemixKirbyHatsGunPart(FTStruct *fp, s32 modelpart_id, FTModelPartDesc *desc, s32 detail_index)
{
    u32 offset = ftRemixKirbyGunHatOffset(*ftRemixFighterKirbyHat(fp));
    FTModelPartDesc *gun_desc;

    if ((offset == 0) || (fp->data->p_file_model == NULL) || (*fp->data->p_file_model == NULL))
    {
        return &desc->modelparts[modelpart_id][detail_index];
    }
    gun_desc = lbRelocGetFileData(FTModelPartDesc*, *fp->data->p_file_model, offset);

    return &gun_desc->modelparts[modelpart_id][detail_index];
}

FTModelPart* ftRemixKirbyHatModelPart(FTStruct *fp, s32 joint_id, s32 modelpart_id, FTModelPartDesc *desc, s32 detail_index)
{
    u32 row_addr;
    FTModelPart *part;
    u32 base_hat;
    u32 file_index;
    u32 first_word;
    void *file;

    if ((joint_id == FTREMIX_KIRBYHATS_GUN_JOINT) && ftRemixKirbyHatsIsKirby(fp))
    {
        return ftRemixKirbyHatsGunPart(fp, modelpart_id, desc, detail_index);
    }
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

/**
 * Whether a Random panel value is rolled on the current screen (asm kirby_hat_select_ _handle_random_hat): the menus, the
 * character select and the results show a plain Kirby, and so does the 1P game over screen.
 */
static sb32 ftRemixKirbyHatsRandomIsRolled(FTStruct *fp)
{
    u8 scene = gSCManagerSceneData.scene_curr;

    if (scene == nSCKindTitle)
    {
        return (gSCManagerBattleState->players[fp->player].stock_count >= 0) ? TRUE : FALSE;
    }
    if (scene < nSCKindMaps)
    {
        return FALSE;
    }
    return (scene != nSCKindVSResults) ? TRUE : FALSE;
}

/**
 * Remix fighter id behind the CSS Kirby Hat value of a Kirby or J Kirby port, 'Random' rolling one of the 40 fighters once
 * (never None or magic, as asm pick_random_hat_ does).
 *
 * @param fp Kirby or J Kirby.
 * @return The fighter, REMIX_ID_KIRBY for the value None or for a Random that the screen does not roll.
 */
static s32 ftRemixKirbyHatsPanelFighterID(FTStruct *fp)
{
    s32 panel_value = gmRemixPanel(KIRBY_HAT, fp->player);
    s32 random_roll = 0;

    if (panel_value == FTREMIX_KIRBYSPAWN_PANEL_RANDOM)
    {
        if (ftRemixKirbyHatsRandomIsRolled(fp) == FALSE)
        {
            return REMIX_ID_KIRBY;
        }
        random_roll = syUtilsRandIntRange(FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT);
    }
    return ftRemixKirbySpawnPanelFighter(panel_value, random_roll);
}

void ftRemixKirbyHatSpawnInit(GObj *fighter_gobj, s32 spawn_copy_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTKirbyCopy *copy_table;
    s32 copy_id;
    s32 panel_fighter;

    if (ftRemixKirbyHatsIsKirby(fp) == FALSE)
    {
        return;
    }
    panel_fighter = ftRemixKirbyHatsPanelFighterID(fp);
    copy_id = (panel_fighter != REMIX_ID_KIRBY) ? panel_fighter : spawn_copy_id;
    fp->passive_vars.kirby.copy_id = copy_id;

    if ((panel_fighter != REMIX_ID_KIRBY) && (panel_fighter != REMIX_ID_NONE))
    {
        /* The hat's special reads that fighter's files, loaded only for fighters in the match (see ftRemixKirbyMagicHatRoll) */
        ftManagerSetupFilesAllKind(panel_fighter);
    }

    copy_table = ftRemixKirbyGetCopyTable();
    ftParamSetModelPartDefaultID(fighter_gobj, FTKIRBY_COPY_MODELPARTS_JOINT, copy_table[((copy_id >= 0) && (copy_id < FTREMIX_KIRBYHATS_COPY_ROWS)) ? copy_id : nFTKindKirby].copy_modelpart_id);
}

#endif
