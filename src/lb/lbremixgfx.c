/**
 * lbremixgfx.c - Smash Remix GFX extension (src/GFX.asm): new moveset GFX ids 0x5C.., the extra particle scripts
 * ("GFX instructions") and the extra texture blocks of the common effect bank.
 *
 * The scripts and texture blocks of the new effects live in the Remix RAM image. They are copied into host memory with
 * the same byte-swap rules as the vanilla particle banks (particle_bank_bridge.cpp) and appended to the script and
 * texture arrays of the bank, so ids 0x77.. and 0x2F.. index them like the vanilla ones. A new GFX id runs the
 * handler of its base effect with its own script id substituted (lbRemixGfxTakeScriptOverride).
 */

#ifdef REGION_REMIX

#include "lbremixgfx.h"
#include <lb/lbparticle.h>
#include <lb/lbtypes.h>
#include <sys/objtypes.h>
#include <port_log.h>
#include <ft/fighter.h>
#include <sys/objman.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define LBREMIXGFX_FIRST_ID 0x5C
#define LBREMIXGFX_NEW_COUNT 33
#define LBREMIXGFX_TEXTURE_BLOCKS 9
#define LBREMIXGFX_SCRIPTS_VANILLA 0x77
#define LBREMIXGFX_TEXTURES_VANILLA 0x2F
#define LBREMIXGFX_SCRIPT_COPY_SIZE 0x100
#define LBREMIXGFX_SCRIPT_HEADER_SIZE 0x30
#define LBREMIXGFX_TEXTURE_HEADER_SIZE 0x18
#define LBREMIXGFX_PALETTE_BYTES 0x200

extern void *calloc(unsigned long count, unsigned long size);
extern void free(void *ptr);
extern s32 sLBParticleScriptBanksNum[];
extern s32 sLBParticleTextureBanksNum[];
extern u32 *sLBParticleScriptBanks[];
extern u32 *sLBParticleTextureBanks[];

/* Vanilla GFX id whose handler each new id borrows (asm add_gfx base_gfx_id), indexed by new id - 0x5C. */
static const u8 sLBRemixGfxBaseIDs[LBREMIXGFX_NEW_COUNT] =
{
    0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C,
    0x00,
    0x1C, 0x1C, 0x29, 0x5A, 0x06, 0x1F, 0x12, 0x13, 0x06, 0x29, 0x12, 0x12, 0x1F,
    0x1F, /* Orange Blast: the asm base id 0x62 has no vanilla handler; the Orange Spark one is the closest */
    0x0C, 0x0D, 0x4A,
    0x5B, 0x5B, 0x5B,
    0x0C, 0x0D, 0x4A,
    0x1C
};

static s32 sLBRemixGfxScriptOverride;
static sb32 sLBRemixGfxIsExtended;

static u32 lbRemixGfxReadBE32(const u8 *p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}

static u16 lbRemixGfxReadBE16(const u8 *p)
{
    return (u16)(((u32)p[0] << 8) | (u32)p[1]);
}

/** Native word of the image at a 4-aligned Remix RAM address; the image keeps its words in host order. */
static u32 lbRemixGfxReadWord(u32 ram_addr)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr & ~3u);

    return (word != NULL) ? *word : 0;
}

static sb32 lbRemixGfxIsInImage(u32 ram_addr)
{
    return remix_ram_image_resolve(ram_addr & ~3u) != NULL;
}

/** Copies size bytes of the image back into the big endian byte order of the N64 memory (zero padded). */
static u8* lbRemixGfxCopyBytes(u32 ram_addr, u32 size)
{
    u32 words = (size + 3) / 4;
    u8 *copy = (u8*)calloc(1, ((size_t)words * 4) + 0x10);
    u32 i;

    if (copy == NULL)
    {
        return NULL;
    }
    for (i = 0; i < words; i++)
    {
        u32 word = lbRemixGfxIsInImage(ram_addr + (i * 4)) ? lbRemixGfxReadWord(ram_addr + (i * 4)) : 0;

        copy[(i * 4) + 0] = (u8)(word >> 24);
        copy[(i * 4) + 1] = (u8)(word >> 16);
        copy[(i * 4) + 2] = (u8)(word >> 8);
        copy[(i * 4) + 3] = (u8)word;
    }
    return copy;
}

/** Copies one script out of the image: the 0x30-byte header is swapped to host order, the bytecode stays big endian. */
static u32 lbRemixGfxMakeScript(u32 ram_addr)
{
    u8 *be;
    s32 i;

    if (!lbRemixGfxIsInImage(ram_addr))
    {
        return 0;
    }
    be = lbRemixGfxCopyBytes(ram_addr, LBREMIXGFX_SCRIPT_COPY_SIZE);

    if (be == NULL)
    {
        return 0;
    }
    for (i = 0; i < 4; i++)
    {
        u16 field = lbRemixGfxReadBE16(be + (i * 2));

        memcpy(be + (i * 2), &field, sizeof(field));
    }
    for (i = 2; i < (LBREMIXGFX_SCRIPT_HEADER_SIZE / 4); i++)
    {
        u32 field = lbRemixGfxReadBE32(be + (i * 4));

        memcpy(be + (i * 4), &field, sizeof(field));
    }
    return PORT_REGISTER(be);
}

/** Bytes of one image of a texture block: width * height texels of the block's texel size. */
static u32 lbRemixGfxImageBytes(u32 width, u32 height, u32 siz)
{
    static const u32 sBitsPerTexel[4] = { 4, 8, 16, 32 };

    return ((width * height * sBitsPerTexel[siz & 3]) + 7) / 8;
}

/** Copies the pixel data of one image or palette into host memory and registers it. */
static u32 lbRemixGfxMakePixels(u32 ram_addr, u32 size)
{
    u8 *pixels;

    if (!lbRemixGfxIsInImage(ram_addr))
    {
        return 0;
    }
    pixels = lbRemixGfxCopyBytes(ram_addr, size);

    return (pixels != NULL) ? PORT_REGISTER(pixels) : 0;
}

/** Copies one texture block out of the image: the header in host order, the data entries pointing at host copies. */
static u32 lbRemixGfxMakeTexture(u32 ram_addr)
{
    u32 *copy;
    u32 count;
    u32 flags;
    s32 fmt;
    u32 image_bytes;
    u32 entries;
    u32 i;

    if (!lbRemixGfxIsInImage(ram_addr))
    {
        return 0;
    }
    count = lbRemixGfxReadWord(ram_addr);
    fmt = (s32)lbRemixGfxReadWord(ram_addr + 4);
    flags = lbRemixGfxReadWord(ram_addr + 0x14);
    image_bytes = lbRemixGfxImageBytes(lbRemixGfxReadWord(ram_addr + 0xC), lbRemixGfxReadWord(ram_addr + 0x10), lbRemixGfxReadWord(ram_addr + 8));
    entries = count;

    if (fmt == G_IM_FMT_CI)
    {
        entries += (flags & 1) ? 1 : count;
    }
    copy = (u32*)calloc(1, LBREMIXGFX_TEXTURE_HEADER_SIZE + ((size_t)entries * 4) + 0x10);

    if (copy == NULL)
    {
        return 0;
    }
    for (i = 0; i < (LBREMIXGFX_TEXTURE_HEADER_SIZE / 4); i++)
    {
        copy[i] = lbRemixGfxReadWord(ram_addr + (i * 4));
    }
    for (i = 0; i < entries; i++)
    {
        u32 pixels_addr = lbRemixGfxReadWord(ram_addr + LBREMIXGFX_TEXTURE_HEADER_SIZE + (i * 4));
        u32 bytes = (i < count) ? image_bytes : LBREMIXGFX_PALETTE_BYTES;

        copy[(LBREMIXGFX_TEXTURE_HEADER_SIZE / 4) + i] = lbRemixGfxMakePixels(pixels_addr, bytes);
    }
    return PORT_REGISTER(copy);
}

static u32* lbRemixGfxGrowTokens(u32 *old_tokens, s32 old_num, s32 new_num)
{
    u32 *tokens = (u32*)calloc((size_t)new_num, sizeof(u32));

    if (tokens != NULL)
    {
        memcpy(tokens, old_tokens, (size_t)old_num * sizeof(u32));
    }
    return tokens;
}

static void lbRemixGfxExtendScripts(s32 bank_id)
{
    u32 *scripts = lbRemixGfxGrowTokens(sLBParticleScriptBanks[bank_id], LBREMIXGFX_SCRIPTS_VANILLA, LBREMIXGFX_SCRIPTS_VANILLA + LBREMIXGFX_NEW_COUNT);
    s32 i;

    if (scripts == NULL)
    {
        return;
    }
    for (i = 0; i < LBREMIXGFX_NEW_COUNT; i++)
    {
        if (sLBRemixGfxBaseIDs[i] != 0)
        {
            scripts[LBREMIXGFX_SCRIPTS_VANILLA + i] = lbRemixGfxMakeScript(lbRemixGfxReadWord(REMIX_SYM_GFX__extended_gfx_instructions_map + (i * 4)));
        }
    }
    sLBParticleScriptBanks[bank_id] = scripts;
    sLBParticleScriptBanksNum[bank_id] = LBREMIXGFX_SCRIPTS_VANILLA + LBREMIXGFX_NEW_COUNT;
}

static void lbRemixGfxExtendTextures(s32 bank_id)
{
    u32 *textures = lbRemixGfxGrowTokens(sLBParticleTextureBanks[bank_id], LBREMIXGFX_TEXTURES_VANILLA, LBREMIXGFX_TEXTURES_VANILLA + LBREMIXGFX_TEXTURE_BLOCKS);
    s32 i;

    if (textures == NULL)
    {
        return;
    }
    for (i = 0; i < LBREMIXGFX_TEXTURE_BLOCKS; i++)
    {
        textures[LBREMIXGFX_TEXTURES_VANILLA + i] = lbRemixGfxMakeTexture(lbRemixGfxReadWord(REMIX_SYM_GFX__extended_gfx_texture_block_map + (i * 4)));
    }
    sLBParticleTextureBanks[bank_id] = textures;
    sLBParticleTextureBanksNum[bank_id] = LBREMIXGFX_TEXTURES_VANILLA + LBREMIXGFX_TEXTURE_BLOCKS;
}

void lbRemixGfxExtendBank(s32 bank_id)
{
    if ((sLBParticleScriptBanksNum[bank_id] != LBREMIXGFX_SCRIPTS_VANILLA) || (sLBParticleTextureBanksNum[bank_id] != LBREMIXGFX_TEXTURES_VANILLA))
    {
        return;
    }
    lbRemixGfxExtendScripts(bank_id);
    lbRemixGfxExtendTextures(bank_id);
    sLBRemixGfxIsExtended = TRUE;
    port_log("[remix-gfx] bank %d extended: %d scripts, %d texture blocks\n", bank_id, sLBParticleScriptBanksNum[bank_id], sLBParticleTextureBanksNum[bank_id]);
}

sb32 lbRemixGfxIsNewEffect(s32 effect_id)
{
    return (effect_id >= LBREMIXGFX_FIRST_ID) && (effect_id < (LBREMIXGFX_FIRST_ID + LBREMIXGFX_NEW_COUNT));
}

s32 lbRemixGfxPrepareEffect(s32 effect_id)
{
    s32 slot = effect_id - LBREMIXGFX_FIRST_ID;

    if (!lbRemixGfxIsNewEffect(effect_id) || (sLBRemixGfxIsExtended == FALSE) || (sLBRemixGfxBaseIDs[slot] == 0))
    {
        return -1;
    }
    sLBRemixGfxScriptOverride = LBREMIXGFX_SCRIPTS_VANILLA + slot;

    return sLBRemixGfxBaseIDs[slot];
}

void lbRemixGfxSetNextEffect(s32 effect_id)
{
    if (lbRemixGfxIsNewEffect(effect_id) && (sLBRemixGfxIsExtended != FALSE) && (sLBRemixGfxBaseIDs[effect_id - LBREMIXGFX_FIRST_ID] != 0))
    {
        sLBRemixGfxScriptOverride = LBREMIXGFX_SCRIPTS_VANILLA + (effect_id - LBREMIXGFX_FIRST_ID);
    }
}

void lbRemixGfxClearOverride(void)
{
    sLBRemixGfxScriptOverride = 0;
}

s32 lbRemixGfxTakeScriptOverride(s32 script_id)
{
    s32 override = sLBRemixGfxScriptOverride;

    if (override == 0)
    {
        return script_id;
    }
    sLBRemixGfxScriptOverride = 0;

    return override;
}

#endif

#ifdef REGION_REMIX
#define LBREMIXGFX_DEBUG_PERIOD 45

static s32 sLBRemixGfxDebugTimer;
static s32 sLBRemixGfxDebugFixedId;
static s32 sLBRemixGfxDebugColAnimId;
static s32 sLBRemixGfxDebugKirbyCopy = -1;
static sb32 sLBRemixGfxDebugKirbyDone;

/** Smoke-test harness (SSB64_REMIX_GFX_TEST=<decimal id>|all): spawns a new GFX id on the first fighter every 45 frames. */
static void lbRemixGfxDebugProcUpdate(GObj *gobj)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];
    s32 effect_id;

    if ((fighter_gobj == NULL) || ((sLBRemixGfxDebugTimer++ % LBREMIXGFX_DEBUG_PERIOD) != 0))
    {
        return;
    }
    if (sLBRemixGfxDebugKirbyCopy >= 0)
    {
        FTStruct *kirby_fp = ftGetStruct(fighter_gobj);

        if ((sLBRemixGfxDebugTimer >= 330) && (sLBRemixGfxDebugKirbyDone == FALSE) && (kirby_fp->fkind == nFTKindKirby))
        {
            sLBRemixGfxDebugKirbyDone = TRUE;
            port_log("[remix-gfx] test kirby copy %d\n", sLBRemixGfxDebugKirbyCopy);
            kirby_fp->status_vars.kirby.specialn.copy_id = sLBRemixGfxDebugKirbyCopy;
            kirby_fp->motion_vars.flags.flag1 = 1;
            ftKirbySpecialNCopyInitCopyVars(fighter_gobj);
        }
        return;
    }
    if (sLBRemixGfxDebugColAnimId != 0)
    {
        port_log("[remix-gfx] test colanim 0x%X\n", sLBRemixGfxDebugColAnimId);
        ftParamCheckSetFighterColAnimID(fighter_gobj, sLBRemixGfxDebugColAnimId, 0);
        return;
    }
    effect_id = (sLBRemixGfxDebugFixedId != 0) ? sLBRemixGfxDebugFixedId : (LBREMIXGFX_FIRST_ID + ((sLBRemixGfxDebugTimer / LBREMIXGFX_DEBUG_PERIOD) % LBREMIXGFX_NEW_COUNT));
    port_log("[remix-gfx] test effect 0x%X\n", effect_id);
    ftParamMakeEffect(fighter_gobj, effect_id, 0, NULL, NULL, 1, FALSE, 0);
}

void lbRemixGfxDebugStart(void)
{
    const char *text = getenv("SSB64_REMIX_GFX_TEST");
    const char *colanim_text = getenv("SSB64_REMIX_COLANIM_TEST");
    const char *kirby_text = getenv("SSB64_REMIX_KIRBY_COPY_TEST");

    sLBRemixGfxDebugTimer = 0;
    sLBRemixGfxDebugFixedId = 0;
    sLBRemixGfxDebugColAnimId = 0;
    sLBRemixGfxDebugKirbyCopy = -1;

    if ((kirby_text != NULL) && (kirby_text[0] != '\0'))
    {
        sLBRemixGfxDebugKirbyCopy = 0;
        while ((*kirby_text >= '0') && (*kirby_text <= '9'))
        {
            sLBRemixGfxDebugKirbyCopy = (sLBRemixGfxDebugKirbyCopy * 10) + (*kirby_text++ - '0');
        }
        text = "0";
    }
    if ((colanim_text != NULL) && (colanim_text[0] != '\0'))
    {
        while ((*colanim_text >= '0') && (*colanim_text <= '9'))
        {
            sLBRemixGfxDebugColAnimId = (sLBRemixGfxDebugColAnimId * 10) + (*colanim_text++ - '0');
        }
        text = "0";
    }
    if ((text == NULL) || (text[0] == '\0'))
    {
        return;
    }

    while ((*text >= '0') && (*text <= '9'))
    {
        sLBRemixGfxDebugFixedId = (sLBRemixGfxDebugFixedId * 10) + (*text++ - '0');
    }
    gcAddGObjProcess(gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT), lbRemixGfxDebugProcUpdate, nGCProcessKindFunc, 4);
}
#endif
