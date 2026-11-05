/**
 * ftremixpartimagepalette.c - big-endian copies and token arrays of the palettes of the Remix RAM image, see
 * ftremixpartimagepalette.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <stddef.h>
#include <ft/fighter.h>
#include <port_log.h>
#include "remix/remix_ram_image.h"
#include "ft/ftremix/ftremixpartimagepalette.h"

#define FTREMIX_PALETTE_COLORS 256
#define FTREMIX_PALETTE_BYTES (FTREMIX_PALETTE_COLORS * 2)
#define FTREMIX_PALETTE_POOL_MAX 1024
#define FTREMIX_PALETTE_ARRAY_POOL_MAX 512

typedef struct FTRemixPaletteCopy
{
    u32 src_addr;
    u32 token;
    u8 bytes[FTREMIX_PALETTE_BYTES];

} FTRemixPaletteCopy;

typedef struct FTRemixPaletteArray
{
    u32 src_addr;
    s32 count;
    u32 token;
    u32 slots[FTREMIX_PALETTE_ARRAY_SLOTS];

} FTRemixPaletteArray;

extern void gfxRegisterTrustedLowVARange(const void *base, size_t size);

static FTRemixPaletteCopy sFTRemixPaletteCopies[FTREMIX_PALETTE_POOL_MAX];
static s32 sFTRemixPaletteCopiesNum;
static FTRemixPaletteArray sFTRemixPaletteArrays[FTREMIX_PALETTE_ARRAY_POOL_MAX];
static s32 sFTRemixPaletteArraysNum;
static sb32 sFTRemixPaletteIsPoolTrusted;

/**
 * Registers the palette pool as static texture data, once. The renderer drops a G_SETTIMG whose pointer sits in the low
 * address range unless the range is registered, and a non-PIE build places BSS there.
 */
static void ftRemixPartImageTrustPalettePool(void)
{
    if (sFTRemixPaletteIsPoolTrusted != FALSE)
    {
        return;
    }
    gfxRegisterTrustedLowVARange(sFTRemixPaletteCopies, sizeof(sFTRemixPaletteCopies));
    sFTRemixPaletteIsPoolTrusted = TRUE;
}

/** Token of `ptr`, registered again when the token table no longer maps `token` to it. */
static u32 ftRemixPartImageEnsureToken(u32 token, void *ptr)
{
    if ((token != 0) && (portRelocResolvePointer(token) == ptr))
    {
        return token;
    }
    return portRelocRegisterPointer(ptr);
}

/** Copies the RGBA5551 halfwords at `src_addr` as big-endian bytes, byte by byte because palettes can be unaligned. */
static void ftRemixPartImageCopyPaletteBytes(u32 src_addr, u8 *bytes)
{
    s32 i;

    for (i = 0; i < FTREMIX_PALETTE_COLORS; i++)
    {
        bytes[i * 2] = remix_ram_image_read_byte(src_addr + (i * 2));
        bytes[(i * 2) + 1] = remix_ram_image_read_byte(src_addr + (i * 2) + 1);
    }
}

/** Token of the big-endian copy of the palette at `src_addr`, or 0 for a null address or a full pool. */
u32 ftRemixPartImageGetPaletteToken(u32 src_addr)
{
    FTRemixPaletteCopy *copy;
    s32 i;

    if (src_addr == 0)
    {
        return 0;
    }
    for (i = 0; i < sFTRemixPaletteCopiesNum; i++)
    {
        copy = &sFTRemixPaletteCopies[i];

        if (copy->src_addr == src_addr)
        {
            copy->token = ftRemixPartImageEnsureToken(copy->token, copy->bytes);

            return copy->token;
        }
    }
    if (sFTRemixPaletteCopiesNum >= FTREMIX_PALETTE_POOL_MAX)
    {
        port_log("SSB64: Remix part image palette pool is full (%d palettes), palette 0x%08X skipped\n", FTREMIX_PALETTE_POOL_MAX, src_addr);

        return 0;
    }
    ftRemixPartImageTrustPalettePool();

    copy = &sFTRemixPaletteCopies[sFTRemixPaletteCopiesNum++];
    copy->src_addr = src_addr;
    ftRemixPartImageCopyPaletteBytes(src_addr, copy->bytes);
    copy->token = portRelocRegisterPointer(copy->bytes);

    return copy->token;
}

/** Existing slot array of `array_addr`, or NULL. */
static FTRemixPaletteArray* ftRemixPartImageFindPaletteArray(u32 array_addr)
{
    s32 i;

    for (i = 0; i < sFTRemixPaletteArraysNum; i++)
    {
        if (sFTRemixPaletteArrays[i].src_addr == array_addr)
        {
            return &sFTRemixPaletteArrays[i];
        }
    }
    return NULL;
}

/** Token of the slot array of `array_addr` for `count` palettes, built on first use. */
u32 ftRemixPartImageGetPaletteArrayToken(u32 array_addr, s32 count)
{
    FTRemixPaletteArray *array;
    s32 i;

    if ((array_addr == 0) || (count < 1) || (count > FTREMIX_PALETTE_ARRAY_SLOTS) || (remix_ram_image_resolve(array_addr) == NULL))
    {
        return 0;
    }
    array = ftRemixPartImageFindPaletteArray(array_addr);

    if ((array != NULL) && (array->count >= count))
    {
        array->token = ftRemixPartImageEnsureToken(array->token, array->slots);

        return array->token;
    }
    if (array == NULL)
    {
        if (sFTRemixPaletteArraysNum >= FTREMIX_PALETTE_ARRAY_POOL_MAX)
        {
            port_log("SSB64: Remix part image palette array pool is full (%d arrays), array 0x%08X skipped\n", FTREMIX_PALETTE_ARRAY_POOL_MAX, array_addr);

            return 0;
        }
        array = &sFTRemixPaletteArrays[sFTRemixPaletteArraysNum++];
        array->src_addr = array_addr;
    }
    for (i = 0; i < count; i++)
    {
        array->slots[i] = ftRemixPartImageGetPaletteToken(remix_ram_image_read_word(array_addr + (i * sizeof(u32))));
    }
    array->count = count;
    array->token = ftRemixPartImageEnsureToken(array->token, array->slots);

    return array->token;
}

#endif
