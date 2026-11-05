/**
 * mnremixstagevariant.c - Smash Remix stage variants of the stage select (src/Stages.asm variant_table, variant,
 * C-down cycle).
 *
 * `Stages.variant_table` has 8 bytes per stage id, one stage id per variant type (DL, Omega, Remix 1..6), 0xFF when the
 * stage has no variant of that type. C-down cycles the selected type over the types the hovered stage has; the choice
 * stays while the cursor moves and only applies to stages that have a variant of that type. The tournament layout turns the
 * variants off (get_stage_id_ skips the variant lookup there, right_check_ skips the C-down cycle).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "mn/mnmaps/mnremixstagetournament.h"
#include "mn/mnmaps/mnremixstagevariant.h"

#define MNREMIX_STAGE_VARIANT_TYPE_COUNT 8
#define MNREMIX_STAGE_VARIANT_NONE 0xFF
#define MNREMIX_STAGE_VARIANT_DEFAULT 0

static s32 sMNRemixMapsVariantType = MNREMIX_STAGE_VARIANT_DEFAULT;

static s32 mnRemixMapsReadVariant(s32 gkind, s32 type)
{
    u32 ram_addr;
    const u32 *word;

    if ((gkind < 0) || (type < 1) || (type > MNREMIX_STAGE_VARIANT_TYPE_COUNT))
    {
        return MNREMIX_STAGE_VARIANT_NONE;
    }
    ram_addr = REMIX_SYM_Stages__variant_table + (gkind * MNREMIX_STAGE_VARIANT_TYPE_COUNT) + (type - 1);
    word = (const u32*)remix_ram_image_resolve(ram_addr & ~3U);

    if (word == NULL)
    {
        return MNREMIX_STAGE_VARIANT_NONE;
    }
    return (*word >> ((3 - (ram_addr & 3)) * 8)) & 0xFF;
}

s32 mnRemixMapsGetVariantType(void)
{
    return sMNRemixMapsVariantType;
}

/** The stage the selection resolves to: the variant of the selected type, or `gkind` itself. */
s32 mnRemixMapsGetVariantGroundKind(s32 gkind)
{
    s32 variant;

    if (mnRemixTournamentIsActive() != FALSE)
    {
        return gkind;
    }
    variant = mnRemixMapsReadVariant(gkind, sMNRemixMapsVariantType);

    return (variant != MNREMIX_STAGE_VARIANT_NONE) ? variant : gkind;
}

/** TRUE when `gkind` has a variant of any type (a variant_table row that is not all 0xFF). */
sb32 mnRemixMapsHasVariants(s32 gkind)
{
    s32 type;

    for (type = 1; type <= MNREMIX_STAGE_VARIANT_TYPE_COUNT; type++)
    {
        if (mnRemixMapsReadVariant(gkind, type) != MNREMIX_STAGE_VARIANT_NONE)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** The selected variant type when `gkind` has a variant of it, else the default type (update_text_ _check_layout_table). */
s32 mnRemixMapsGetShownVariantType(s32 gkind)
{
    if ((sMNRemixMapsVariantType != MNREMIX_STAGE_VARIANT_DEFAULT) && (mnRemixMapsReadVariant(gkind, sMNRemixMapsVariantType) == MNREMIX_STAGE_VARIANT_NONE))
    {
        return MNREMIX_STAGE_VARIANT_DEFAULT;
    }
    return sMNRemixMapsVariantType;
}

/** C-down: moves to the next variant type `gkind` has, back to the default after the last. TRUE when the stage has any. */
sb32 mnRemixMapsCycleVariant(s32 gkind)
{
    if ((mnRemixTournamentIsActive() != FALSE) || (mnRemixMapsHasVariants(gkind) == FALSE))
    {
        return FALSE;
    }
    if ((sMNRemixMapsVariantType != MNREMIX_STAGE_VARIANT_DEFAULT) && (mnRemixMapsReadVariant(gkind, sMNRemixMapsVariantType) == MNREMIX_STAGE_VARIANT_NONE))
    {
        sMNRemixMapsVariantType = MNREMIX_STAGE_VARIANT_DEFAULT;
    }
    do
    {
        sMNRemixMapsVariantType = (sMNRemixMapsVariantType < MNREMIX_STAGE_VARIANT_TYPE_COUNT) ? (sMNRemixMapsVariantType + 1) : MNREMIX_STAGE_VARIANT_DEFAULT;
    }
    while ((sMNRemixMapsVariantType != MNREMIX_STAGE_VARIANT_DEFAULT) && (mnRemixMapsReadVariant(gkind, sMNRemixMapsVariantType) == MNREMIX_STAGE_VARIANT_NONE));

    return TRUE;
}

#endif
