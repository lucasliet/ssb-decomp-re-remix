/**
 * itremixspawn.c - spawn weights and switches of the Remix standard items. See itremixspawn.h.
 */

#ifdef PORT

#include "itremix.h"
#include "itremixspawn.h"
#include "it/itremix/itremixcloakingdevice.h"
#include "it/itremix/itremixfranklinbadge.h"
#include "it/itremix/itremixlightning.h"
#include "it/itremix/itremixpwing.h"
#include "it/itremix/itremixstopwatch.h"
#include "it/itremix/itremixsupermushroom.h"
#include <sc/scene.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define ITREMIX_RATE_TABLE_ROWS \
    ((REMIX_SYM_Stages__series_logo_table - REMIX_SYM_Stages__custom_item_spawn_rate_table) / ITREMIX_ITEMS_NUM)

u32 gITRemixExtendedToggles = ITREMIX_EXTENDED_TOGGLES_DEFAULT;

/* Saved copy of the extended toggles; Item.asm EXTENDED_ENABLED_BITMASK +0x04. */
static u32 sITRemixExtendedTogglesSaved = ITREMIX_EXTENDED_TOGGLES_DEFAULT;

void itRemixSaveExtendedToggles(void)
{
    sITRemixExtendedTogglesSaved = gITRemixExtendedToggles;
}

void itRemixRestoreExtendedToggles(void)
{
    gITRemixExtendedToggles = sITRemixExtendedTogglesSaved;
}

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

    /* Vanilla 1P suppression needs no code here: the gkind rows of the vanilla
     * 1P stages (0x13..0x1C) are all zero in Stages.custom_item_spawn_rate_table
     * (verified in the Remix RAM image), so weights are 0 there no matter the
     * toggle state — the data-side equivalent of prevent_1p_custom_item_spawns_.
     * Remix's own 1P stages (gkind 0x30/0x64/0xD7) carry nonzero rows. */
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

/*
 * Item.asm clear_active_custom_items_ (called on every screen change to
 * CSS/VS/Training/VS Results): forget every per-item effect state so nothing
 * survives into the next scene. REMIX-TODO(Poison): Poison.clear_poison_ (asm Poison.asm, clears
 * poisoned_players) is not called here; the poison state is not ported yet.
 */
void itRemixClearActiveItems(void)
{
    itRemixCloakingDeviceClearStates();
    itRemixSuperMushroomClearStates();
    itRemixLightningClearActive();
    itRemixFranklinBadgeClearAll();
    itRemixPwingClearActive();
    itRemixStopwatchClearActive();
}

#endif
