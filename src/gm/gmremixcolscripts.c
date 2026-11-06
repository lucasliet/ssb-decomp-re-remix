/**
 * gmremixcolscripts.c - Remix GFX routines (GFXRoutine.asm) for the colour animation interpreter.
 *
 * The routines are command streams of 32-bit words in the Remix RAM image. They are copied into one host block, and the
 * operands that hold an N64 address (goto, subroutine, parallel script) become registered pointers into that block,
 * the same way gmColScriptsLinkRelocTargets does it for the vanilla scripts. The table entries are appended to
 * dGMColScriptsDescs from id 0x56.
 */

#ifdef REGION_REMIX

#include "gmremixcolscripts.h"
#include <gm/gmdef.h>
#include <gm/gmtypes.h>
#include <ft/ftcommondata.h>
#include <sys/objtypes.h>
#include <port_log.h>
#include <stdint.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#include <stddef.h>

extern void *calloc(size_t count, size_t size);

#define GMREMIX_COLSCRIPTS_ENTRY_SIZE 8
#define GMREMIX_COLSCRIPTS_PORTS 4
#define GMREMIX_COLSCRIPTS_OPCODE_SHIFT 26
#define GMREMIX_COLSCRIPTS_EXTRA_PRIORITY 100

static sb32 sGMRemixColScriptsIsLoaded;
static sb32 sGMRemixColScriptsIsAttempted;
static s32 sGMRemixColScriptsOverrides[GMREMIX_COLSCRIPTS_PORTS];

static u32 gmRemixColScriptsReadWord(u32 ram_addr)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr);

    return (word != NULL) ? *word : 0;
}

/** Words taken by the command with this opcode, and whether the stream continues after it. */
static s32 gmRemixColScriptsCommandLength(u32 opcode, sb32 *is_pointer, sb32 *is_terminal)
{
    *is_pointer = FALSE;
    *is_terminal = FALSE;

    switch (opcode)
    {
    case nGMColEventEnd:
    case nGMColEventReturn:
        *is_terminal = TRUE;
        return 1;

    case nGMColEventGoto:
        *is_pointer = TRUE;
        *is_terminal = TRUE;
        return 2;

    case nGMColEventSubroutine:
    case nGMColEventSetParallelScript:
        *is_pointer = TRUE;
        return 2;

    case nGMColEventSetColor1:
    case nGMColEventBlendColor1:
    case nGMColEventSetColor2:
    case nGMColEventBlendColor2:
        return 2;

    case nGMColEventEffect:
    case nGMColEventEffectItemHoldOffset:
        return 4;

    default:
        return 1;
    }
}

typedef struct GMRemixColScriptsBlock
{
    u32 *words;
    u32 base;
    u32 words_num;
    u8 *visited;
} GMRemixColScriptsBlock;

/** Walks the commands reachable from ram_addr and turns their address operands into registered pointers. */
static void gmRemixColScriptsLinkFrom(GMRemixColScriptsBlock *block, u32 ram_addr, s32 depth)
{
    while (depth < 64)
    {
        u32 index = (ram_addr - block->base) / 4;
        u32 opcode;
        sb32 is_pointer;
        sb32 is_terminal;
        s32 length;

        if ((ram_addr < block->base) || (index >= block->words_num) || (block->visited[index] != 0))
        {
            return;
        }
        block->visited[index] = 1;
        opcode = block->words[index] >> GMREMIX_COLSCRIPTS_OPCODE_SHIFT;
        length = gmRemixColScriptsCommandLength(opcode, &is_pointer, &is_terminal);

        if (is_pointer && ((index + 1) < block->words_num))
        {
            u32 target = block->words[index + 1];

            if ((target >= block->base) && (((target - block->base) / 4) < block->words_num))
            {
                block->words[index + 1] = portRelocRegisterPointer(&block->words[(target - block->base) / 4]);
                gmRemixColScriptsLinkFrom(block, target, depth + 1);
            }
        }
        if (is_terminal)
        {
            return;
        }
        ram_addr += length * 4;
    }
}

static GMRemixColScriptsBlock sGMRemixColScriptsBlock;

static sb32 gmRemixColScriptsLoad(void)
{
    u32 table = REMIX_SYM_GFXRoutine__gfx_routine_table_extended;
    s32 count = (REMIX_SYM_GFXRoutine__extend_gfx_routine_table_ - table) / GMREMIX_COLSCRIPTS_ENTRY_SIZE;
    GMRemixColScriptsBlock block;
    u32 lowest = table;
    s32 i;

    if ((count <= 0) || ((GMCOLSCRIPTS_REMIX_VANILLA_NUM + count) > GMCOLSCRIPTS_REMIX_DESCS_MAX) || (remix_ram_image_resolve(table) == NULL))
    {
        return FALSE;
    }
    for (i = 0; i < count; i++)
    {
        u32 script = gmRemixColScriptsReadWord(table + (i * GMREMIX_COLSCRIPTS_ENTRY_SIZE));

        if ((script != 0) && (script < lowest))
        {
            lowest = script;
        }
    }
    block.base = lowest;
    block.words_num = (table - lowest) / 4;
    block.words = (u32*)calloc(block.words_num + 4, sizeof(u32));
    block.visited = (u8*)calloc(block.words_num + 4, 1);

    if ((block.words == NULL) || (block.visited == NULL))
    {
        return FALSE;
    }
    for (i = 0; i < block.words_num; i++)
    {
        block.words[i] = gmRemixColScriptsReadWord(block.base + (i * 4));
    }
    for (i = 0; i < count; i++)
    {
        u32 entry = table + (i * GMREMIX_COLSCRIPTS_ENTRY_SIZE);
        u32 script = gmRemixColScriptsReadWord(entry);
        u32 flags = gmRemixColScriptsReadWord(entry + 4);
        GMColDesc *desc = &dGMColScriptsDescs[GMCOLSCRIPTS_REMIX_VANILLA_NUM + i];

        gmRemixColScriptsLinkFrom(&block, script, 0);

        desc->p_script = &block.words[(script - block.base) / 4];
        desc->priority = (u8)(flags >> 24);
        desc->is_unlocked = (u8)(flags >> 16);
    }
    dGMColScriptsDescsNum = GMCOLSCRIPTS_REMIX_VANILLA_NUM + count;
    sGMRemixColScriptsBlock = block;
    port_log("[remix-gfx] %d GFX routines loaded (ids 0x%X..0x%X)\n", count, GMCOLSCRIPTS_REMIX_VANILLA_NUM, dGMColScriptsDescsNum - 1);

    return TRUE;
}

sb32 gmRemixColScriptsEnsureLoaded(void)
{
    if (sGMRemixColScriptsIsAttempted == FALSE)
    {
        sGMRemixColScriptsIsAttempted = TRUE;
        sGMRemixColScriptsIsLoaded = gmRemixColScriptsLoad();
    }
    return sGMRemixColScriptsIsLoaded;
}

s32 gmRemixColScriptsRegisterRoutine(u32 ram_addr)
{
    GMRemixColScriptsBlock *block = &sGMRemixColScriptsBlock;
    GMColDesc *desc;

    if ((gmRemixColScriptsEnsureLoaded() == FALSE) || (dGMColScriptsDescsNum >= GMCOLSCRIPTS_REMIX_DESCS_MAX))
    {
        return 0;
    }
    if ((ram_addr < block->base) || (((ram_addr - block->base) / 4) >= block->words_num))
    {
        port_log("[remix-gfx] routine 0x%X is outside the loaded GFX routine block\n", (unsigned int)ram_addr);
        return 0;
    }
    gmRemixColScriptsLinkFrom(block, ram_addr, 0);

    desc = &dGMColScriptsDescs[dGMColScriptsDescsNum];
    desc->p_script = &block->words[(ram_addr - block->base) / 4];
    desc->priority = GMREMIX_COLSCRIPTS_EXTRA_PRIORITY;
    desc->is_unlocked = FALSE;

    return dGMColScriptsDescsNum++;
}

s32 gmRemixColScriptsGetOverride(s32 port)
{
    return ((port >= 0) && (port < GMREMIX_COLSCRIPTS_PORTS)) ? sGMRemixColScriptsOverrides[port] : 0;
}

void gmRemixColScriptsSetOverride(s32 port, s32 colanim_id)
{
    if ((port >= 0) && (port < GMREMIX_COLSCRIPTS_PORTS))
    {
        sGMRemixColScriptsOverrides[port] = colanim_id;
    }
}

void gmRemixColScriptsClearOverrides(void)
{
    s32 port;

    for (port = 0; port < GMREMIX_COLSCRIPTS_PORTS; port++)
    {
        sGMRemixColScriptsOverrides[port] = 0;
    }
}

#endif
