/**
 * itremixspawn.c - spawn weights and switches of the Remix standard items. See itremixspawn.h.
 */

#ifdef PORT

#include "itremix.h"
#include "itremixspawn.h"
#include <sc/scene.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define ITREMIX_RATE_TABLE_ROWS \
    ((REMIX_SYM_Stages__series_logo_table - REMIX_SYM_Stages__custom_item_spawn_rate_table) / ITREMIX_ITEMS_NUM)

u32 gITRemixExtendedToggles = ITREMIX_EXTENDED_TOGGLES_DEFAULT;

static sb32 itRemixStandardItemIsPorted(s32 index)
{
    s32 i;

    for (i = 0; i < dITRemixItemDescsNum; i++)
    {
        if (dITRemixItemDescs[i]->kind == (ITREMIX_KIND_START + index))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/* Byte of the Remix RAM image; the image holds native u32 words. */
static s32 itRemixReadImageByte(u32 addr)
{
    u32 *word = remix_ram_image_resolve(addr & ~3u);

    if (word == NULL)
    {
        return 0;
    }
    return (*word >> ((3 - (addr & 3)) * 8)) & 0xFF;
}

sb32 itRemixStandardItemIsOn(s32 index)
{
    if (index < ITREMIX_VANILLA_TOGGLE_ITEMS_MAX)
    {
        return (gSCManagerBattleState->item_toggles >> (nITKindCommonEnd + 1 + index)) & 1;
    }
    return (gITRemixExtendedToggles >> (index - ITREMIX_VANILLA_TOGGLE_ITEMS_MAX)) & 1;
}

s32 itRemixStandardItemGetWeight(s32 index)
{
    s32 stage = gSCManagerBattleState->gkind;

    if ((index < 0) || (index >= ITREMIX_STANDARD_ITEMS_NUM) || (stage >= ITREMIX_RATE_TABLE_ROWS))
    {
        return 0;
    }
    if (!itRemixStandardItemIsOn(index) || !itRemixStandardItemIsPorted(index))
    {
        return 0;
    }
    return itRemixReadImageByte(REMIX_SYM_Stages__custom_item_spawn_rate_table + (stage * ITREMIX_ITEMS_NUM) + index);
}

sb32 itRemixAnyStandardItemIsOn(void)
{
    s32 i;

    for (i = 0; i < ITREMIX_STANDARD_ITEMS_NUM; i++)
    {
        if (itRemixStandardItemIsOn(i))
        {
            return TRUE;
        }
    }
    return FALSE;
}

#endif
