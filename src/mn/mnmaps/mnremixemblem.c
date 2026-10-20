/**
 * mnremixemblem.c - Series emblem on the wooden coin of the stage select (src/Stages.asm extend_series_logos_,
 * Stages.series_logo_table, CharacterSelect.series_logo.table).
 *
 * `Stages.series_logo_table` holds one series index per stage id. `CharacterSelect.series_logo.table` has 12 bytes per
 * series: the sprite offset in the emblem file and the X and Y position offsets (single floats) from the coin origin.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <string.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "mn/mnmaps/mnremixemblem.h"

#define MNREMIX_EMBLEM_ENTRY_SIZE 12
#define MNREMIX_EMBLEM_SERIES_NONE 0

static sb32 mnRemixEmblemReadWord(u32 ram_addr, u32 *value)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr);

    if (word == NULL)
    {
        return FALSE;
    }
    *value = *word;

    return TRUE;
}

static sb32 mnRemixEmblemReadSeries(s32 gkind, u32 *series)
{
    u32 word;
    u32 ram_addr = REMIX_SYM_Stages__series_logo_table + gkind;

    if ((gkind < 0) || !mnRemixEmblemReadWord(ram_addr & ~3U, &word))
    {
        return FALSE;
    }
    *series = (word >> ((3 - (ram_addr & 3)) * 8)) & 0xFF;

    return TRUE;
}

static f32 mnRemixEmblemBitsToFloat(u32 bits)
{
    f32 value;

    memcpy(&value, &bits, sizeof(value));

    return value;
}

/** Sprite offset and position offsets of the emblem of stage `gkind`; FALSE when the stage has no series logo. */
sb32 mnRemixMapsGetEmblem(s32 gkind, u32 *sprite_offset, f32 *pos_x, f32 *pos_y)
{
    u32 series, offset, x_bits, y_bits;
    u32 entry;

    if (!mnRemixEmblemReadSeries(gkind, &series) || (series == MNREMIX_EMBLEM_SERIES_NONE))
    {
        return FALSE;
    }
    entry = REMIX_SYM_CharacterSelect__series_logo__table + (series * MNREMIX_EMBLEM_ENTRY_SIZE);

    if (!mnRemixEmblemReadWord(entry, &offset) || !mnRemixEmblemReadWord(entry + 4, &x_bits) || !mnRemixEmblemReadWord(entry + 8, &y_bits))
    {
        return FALSE;
    }
    *sprite_offset = offset;
    *pos_x = mnRemixEmblemBitsToFloat(x_bits);
    *pos_y = mnRemixEmblemBitsToFloat(y_bits);

    return (offset != 0) ? TRUE : FALSE;
}

#endif
