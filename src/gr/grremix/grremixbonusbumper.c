/**
 * grremixbonusbumper.c - bumpers of the Remix Board the Platforms stages, see grremixbonusbumper.h.
 *
 * Hazards.asm sonic_btp_setup patches the bumper display list in place with three word writes: byte 0x50 of the list
 * (word 0 of command 10, the palette load that the game fills through segment 0xE and the MObj palette) becomes
 * 0xFD100000 (a plain RGBA16 G_SETTIMG), byte 0x54 (its address) the palette of the Sonic bumper file (+0x40), and byte 0x84
 * (word 1 of command 16, the texture G_SETTIMG) the texture of the file (+0x90). The file is the stage header word 0xB0.
 *
 * The port finds the two commands by opcode instead of by byte offset: the palette load is the first call of segment 0xE
 * (opcode 0xDE) and the texture is the first G_SETTIMG after it. Like the asm it patches the list of the loaded item file in
 * place (the file is reloaded every scene, and the interpreter reads packed lists only inside registered file ranges, so a
 * private copy would be misread); every bumper shares that list, so the second and later bumpers find it already patched.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixbonusbumper.h"
#include "gr/grremix/grremixids.h"
#include <it/item.h>
#include <mp/map.h>
#include <sc/scene.h>
#include <port_log.h>

#define GRREMIX_BONUS_BUMPER_SONIC_FGM 0x03D5
#define GRREMIX_BONUS_BUMPER_FILE_FIELD 0xB0 /* stage header: file with the Sonic bumper palette and texture */
#define GRREMIX_BONUS_BUMPER_PALETTE_OFFSET 0x40
#define GRREMIX_BONUS_BUMPER_TEXTURE_OFFSET 0x90
#define GRREMIX_BONUS_BUMPER_DL_COMMANDS_MAX 64
#define GRREMIX_BONUS_BUMPER_GBI_ENDDL 0xDFu
#define GRREMIX_BONUS_BUMPER_GBI_CALL 0xDEu
#define GRREMIX_BONUS_BUMPER_GBI_SETTIMG 0xFDu
#define GRREMIX_BONUS_BUMPER_GBI_SETTIMG_RGBA16 0xFD100000u
#define GRREMIX_BONUS_BUMPER_SEGMENT_E 0x0Eu

/**
 * Number of commands of a packed display list up to and including its G_ENDDL.
 *
 * @param words Display list as the file stores it: two u32 per command.
 * @return Command count, or 0 when no G_ENDDL is found in the first GRREMIX_BONUS_BUMPER_DL_COMMANDS_MAX commands.
 */
static s32 grRemixBonusBumperCountCommands(const u32 *words)
{
    s32 i;

    for (i = 0; i < GRREMIX_BONUS_BUMPER_DL_COMMANDS_MAX; i++)
    {
        if ((words[i * 2] >> 24) == GRREMIX_BONUS_BUMPER_GBI_ENDDL)
        {
            return i + 1;
        }
    }
    return 0;
}

/**
 * The palette load of the bumper list: its first call of segment 0xE.
 *
 * @param words Packed display list.
 * @param count Number of commands.
 * @return Command index, or -1.
 */
static s32 grRemixBonusBumperFindPaletteCommand(const u32 *words, s32 count)
{
    s32 i;

    for (i = 0; i < count; i++)
    {
        if (((words[i * 2] >> 24) == GRREMIX_BONUS_BUMPER_GBI_CALL) && ((words[(i * 2) + 1] >> 24) == GRREMIX_BONUS_BUMPER_SEGMENT_E))
        {
            return i;
        }
    }
    return -1;
}

/**
 * The texture load of the bumper list: the first G_SETTIMG after `from`.
 *
 * @param words Packed display list.
 * @param count Number of commands.
 * @param from Command index to start after.
 * @return Command index, or -1.
 */
static s32 grRemixBonusBumperFindTextureCommand(const u32 *words, s32 count, s32 from)
{
    s32 i;

    for (i = from + 1; i < count; i++)
    {
        if ((words[i * 2] >> 24) == GRREMIX_BONUS_BUMPER_GBI_SETTIMG)
        {
            return i;
        }
    }
    return -1;
}

/**
 * sonic_btp_setup: the display list of the bumper draws the Sonic bumper palette and texture of the stage header file.
 *
 * @param item_gobj Bumper item.
 */
static void grRemixBonusBumperSwapSonicImages(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    u32 file_token = *(u32*) ((uintptr_t)gMPCollisionGroundData + GRREMIX_BONUS_BUMPER_FILE_FIELD);
    u8 *bumper_file = (u8*) PORT_RESOLVE(file_token);
    u32 *words = (dobj != NULL) ? (u32*) dobj->dl : NULL;
    s32 count;
    s32 palette_index;
    s32 texture_index;

    if ((words == NULL) || (bumper_file == NULL))
    {
        port_log("grremixbonusbumper: BTP_SONIC header word 0xB0=0x%08X or bumper display list unresolved, bumper keeps its texture\n", file_token);
        return;
    }
    count = grRemixBonusBumperCountCommands(words);
    palette_index = grRemixBonusBumperFindPaletteCommand(words, count);

    if (palette_index < 0)
    {
        return;
    }
    texture_index = grRemixBonusBumperFindTextureCommand(words, count, palette_index);

    if (texture_index < 0)
    {
        port_log("grremixbonusbumper: bumper display list has a palette call but no texture load in %d commands, keeps its texture\n", count);
        return;
    }
    words[palette_index * 2] = GRREMIX_BONUS_BUMPER_GBI_SETTIMG_RGBA16;
    words[(palette_index * 2) + 1] = PORT_REGISTER(bumper_file + GRREMIX_BONUS_BUMPER_PALETTE_OFFSET);
    words[(texture_index * 2) + 1] = PORT_REGISTER(bumper_file + GRREMIX_BONUS_BUMPER_TEXTURE_OFFSET);
}

void grRemixBonusBumperSetup(GObj *item_gobj)
{
    if ((gSCManagerBattleState->gkind != nGRRemixStage_BTP_SONIC) || (item_gobj == NULL))
    {
        return;
    }
    itGetStruct(item_gobj)->attack_coll.fgm_id = GRREMIX_BONUS_BUMPER_SONIC_FGM;
    grRemixBonusBumperSwapSonicImages(item_gobj);
}

#endif
