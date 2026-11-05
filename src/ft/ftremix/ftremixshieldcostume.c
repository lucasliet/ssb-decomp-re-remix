/**
 * ftremixshieldcostume.c - Smash Remix Shield option Costume: Character.costume_shield_color (Character.asm 2793).
 *
 * Each character has a pointer, in Character.costume_shield_color.table, to a byte array of Shield.color indices, one per
 * costume. Every Remix character copies the pointer of its parent (Character.asm 342) unless its own file calls
 * Character.set_costume_shield_colors or Character.polygon_setup. The table and the rows are read from the Remix RAM image, so
 * an index past the end of a row reads the next bytes of the image as the asm does (Shield.asm color_fix_).
 *
 * Classic Sonic (Sonic.classic_table) adds 6 to the costume id in Shield.asm color_fix_ (ftRemixSonicClassicShieldCostumeOffset),
 * so a classic Sonic reads past the end of the row it shares.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/sonic/ftremixsonicclassic.h"

#define FTREMIX_SHIELD_TABLE_ENTRY_SIZE 4

/** TRUE and the RAM address of the costume row of a fighter id, or FALSE when the id has no table entry or the row is outside the image. */
static sb32 ftRemixShieldGetRowAddress(s32 fkind, u32 *row_address)
{
    if ((fkind < REMIX_ID_MARIO) || (fkind > REMIX_ID_NCRASH))
    {
        return FALSE;
    }
    *row_address = remix_ram_image_read_word(REMIX_SYM_Character__costume_shield_color__table + (fkind * FTREMIX_SHIELD_TABLE_ENTRY_SIZE));

    return (remix_ram_image_resolve(*row_address) != NULL) ? TRUE : FALSE;
}

/**
 * Shield.color index of the costume of a fighter (Shield.asm color_fix_ _costume branch), or -1 when the character has no row
 * in the image.
 */
s32 ftRemixShieldGetCostumeColorIndex(const FTStruct *fp)
{
    u32 row_address;

    if (ftRemixShieldGetRowAddress(fp->fkind, &row_address) == FALSE)
    {
        return -1;
    }
    return remix_ram_image_read_byte(row_address + fp->costume + ftRemixSonicClassicShieldCostumeOffset(fp));
}

#endif
