/**
 * mnremixvariants.c - Smash Remix D-pad variants of the character select (src/CharacterSelect.asm check_variant_input_
 * and get_character_id_, Character.variants.table).
 *
 * While a token is held, D-pad up/down/left/right pick the variant slot of the portrait under the token (special,
 * polygon, J, E). Pressing the same direction again goes back to the normal fighter. The slot is stored per token.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <PR/os.h>
#include <ft/ftdef.h>
#include <sys/utils.h>
#include "gm/gmremixtoggle.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "mn/mnplayers/mnremixvariants.h"

#define MNREMIX_VARIANT_PLAYERS_MAX 4
#define MNREMIX_VARIANT_SLOTS MNREMIX_VARIANT_SLOT_COUNT
#define MNREMIX_VARIANT_NONE 0
#define MNREMIX_RANDOM_ALL_VARIANTS 2
#define MNREMIX_RANDOM_VANILLA 3

static u8 sMNRemixVariantOffsets[MNREMIX_VARIANT_PLAYERS_MAX];

static sb32 mnRemixVariantReadByte(u32 ram_addr, u8 *value)
{
    const u32 *word = (const u32*)remix_ram_image_resolve(ram_addr & ~3U);

    if (word == NULL)
    {
        return FALSE;
    }
    *value = (u8)(*word >> ((3 - (ram_addr & 3)) * 8));

    return TRUE;
}

static s32 mnRemixVariantGetTapSlot(u16 button_tap)
{
    if (button_tap & U_JPAD)
    {
        return 1;
    }
    if (button_tap & D_JPAD)
    {
        return 2;
    }
    if (button_tap & L_JPAD)
    {
        return 3;
    }
    if (button_tap & R_JPAD)
    {
        return 4;
    }
    return MNREMIX_VARIANT_NONE;
}

/** Updates the variant slot of the held token from the D-pad press of the hand holding it. */
void mnRemixVariantHandleInput(s32 held_player, u16 button_tap)
{
    s32 slot;

    if ((held_player < 0) || (held_player >= MNREMIX_VARIANT_PLAYERS_MAX))
    {
        return;
    }
    slot = mnRemixVariantGetTapSlot(button_tap);

    if (slot == MNREMIX_VARIANT_NONE)
    {
        return;
    }
    sMNRemixVariantOffsets[held_player] = (sMNRemixVariantOffsets[held_player] == slot) ? MNREMIX_VARIANT_NONE : slot;
}

s32 mnRemixVariantGetOffset(s32 held_player)
{
    if ((held_player < 0) || (held_player >= MNREMIX_VARIANT_PLAYERS_MAX))
    {
        return MNREMIX_VARIANT_NONE;
    }
    return sMNRemixVariantOffsets[held_player];
}

/** Variant of `fkind` in D-pad slot `slot` (1 up, 2 down, 3 left, 4 right), or `fkind` when it has none. */
s32 mnRemixVariantLookup(s32 fkind, s32 slot)
{
    u8 variant;

    if ((slot < 1) || (slot > MNREMIX_VARIANT_SLOTS) || (fkind < 0))
    {
        return fkind;
    }
    if (!mnRemixVariantReadByte(REMIX_SYM_Character__variants__table + (fkind * MNREMIX_VARIANT_SLOTS) + (slot - 1), &variant))
    {
        return fkind;
    }
    if ((variant == nFTKindNull) || (variant == nFTKindBoss))
    {
        return fkind;
    }
    return variant;
}

/** The fighter the token picks: the variant of `fkind` for the held slot, or `fkind` when it has none. */
s32 mnRemixVariantGetFighterKind(s32 held_player, s32 fkind)
{
    return mnRemixVariantLookup(fkind, mnRemixVariantGetOffset(held_player));
}

/**
 * Applies the Random Select setting (Toggles.entry_variant_random) to a random pick of a grid portrait:
 * 2 (all variants) turns it into one of its four variants one time in five, 3 (vanilla) rerolls non-vanilla fighters.
 * Returns nFTKindNull when the pick must be rolled again.
 */
s32 mnRemixRandomResolveFighterKind(s32 fkind)
{
    switch (gmRemixToggle(variant_random))
    {
    case MNREMIX_RANDOM_ALL_VARIANTS:
        return mnRemixVariantLookup(fkind, syUtilsRandTimeUCharRange(MNREMIX_VARIANT_SLOTS + 1));

    case MNREMIX_RANDOM_VANILLA:
        return (fkind <= nFTKindNess) ? fkind : nFTKindNull;

    default:
        return fkind;
    }
}

#endif
