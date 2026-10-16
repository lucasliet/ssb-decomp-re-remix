/**
 * ftremixkirbycopy.c - Kirby's copy table for every Remix fighter (Character.kirby_inhale_struct.table).
 *
 * The table has one 0xC-byte row per Remix character id: copied power character id, hat id, star scale, star damage.
 * It has the layout of FTKirbyCopy, so the vanilla copy[] lookups read from it unchanged. The Remix RAM image keeps
 * its words in host order, which puts the two halfwords of the first word in logical (big endian) order.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <string.h>
#include <ft/fighter.h>
#include "fighter_registry.h"
#include "ft/ftremix/ftremix.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define FTREMIX_KIRBY_COPY_ROW_SIZE 0xC
#define FTREMIX_KIRBY_COPY_ROWS 97

static FTKirbyCopy sFTRemixKirbyCopyTable[FTREMIX_KIRBY_COPY_ROWS];
static sb32 sFTRemixKirbyCopyIsLoaded;

static u32 ftRemixKirbyCopyReadWord(u32 ram_addr)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr);

    return (word != NULL) ? *word : 0;
}

static void ftRemixKirbyCopyLoadRow(s32 row)
{
    u32 addr = REMIX_SYM_Character__kirby_inhale_struct__table + (row * FTREMIX_KIRBY_COPY_ROW_SIZE);
    u32 ids = ftRemixKirbyCopyReadWord(addr);
    u32 scale_bits = ftRemixKirbyCopyReadWord(addr + 4);
    FTKirbyCopy *copy = &sFTRemixKirbyCopyTable[row];

    copy->copy_id = (u16)(ids >> 16);
    copy->copy_modelpart_id = (s16)(ids & 0xFFFF);
    memcpy(&copy->effect_scale, &scale_bits, sizeof(f32));
    copy->star_damage = (s32)ftRemixKirbyCopyReadWord(addr + 8);
}

FTData *ftRemixPowerData(FTStruct *fp)
{
    FTData *power_data;

    if (((fp->fkind != nFTKindKirby) && (fp->fkind != REMIX_ID_JKIRBY)) || (fp->passive_vars.kirby.copy_id < nFTKindEnumCount))
    {
        return fp->data;
    }
    power_data = port_fighter_data(fp->passive_vars.kirby.copy_id);

    return (power_data != NULL) ? power_data : fp->data;
}

FTKirbyCopy* ftRemixKirbyGetCopyTable(void)
{
    s32 row;

    if (sFTRemixKirbyCopyIsLoaded == FALSE)
    {
        for (row = 0; row < FTREMIX_KIRBY_COPY_ROWS; row++)
        {
            ftRemixKirbyCopyLoadRow(row);
        }
        sFTRemixKirbyCopyIsLoaded = TRUE;
    }
    return sFTRemixKirbyCopyTable;
}

#endif
