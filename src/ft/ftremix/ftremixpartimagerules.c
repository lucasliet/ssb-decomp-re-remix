/**
 * ftremixpartimagerules.c - pure rules of the Remix model part image swaps, see ftremixpartimagerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixpartimagerules.h"

/** First costume id that selects an extra costume (Costumes.asm check_for_custom_costume_, t5). */
int ftRemixPartImageRulesFirstExtraCostume(int num_costumes, int skipped)
{
    return num_costumes + skipped + 1;
}

/** Extra costume index of a costume id, or FTREMIX_PARTIMAGE_NO_EXTRA when the id does not select an extra costume. */
int ftRemixPartImageRulesExtraIndex(int costume, int num_costumes, int skipped, int extra_count)
{
    int first_extra = ftRemixPartImageRulesFirstExtraCostume(num_costumes, skipped);

    if ((costume < first_extra) || (costume >= (first_extra + extra_count)))
    {
        return FTREMIX_PARTIMAGE_NO_EXTRA;
    }
    return costume - first_extra;
}

/** Highest costume id a player can reach (Costumes.asm select_, t4). */
int ftRemixPartImageRulesMaxCostume(int num_costumes, int skipped, int extra_count)
{
    return num_costumes + skipped + extra_count;
}

/** Costume after a C right press (Costumes.asm select_ _right). */
int ftRemixPartImageRulesNextCostume(int costume, int num_costumes, int skipped, int extra_count)
{
    int max_costume = ftRemixPartImageRulesMaxCostume(num_costumes, skipped, extra_count);

    if (costume == num_costumes)
    {
        costume += skipped;
    }
    return (costume < max_costume) ? (costume + 1) : 0;
}

/** Costume after a C left press (Costumes.asm select_ _left). */
int ftRemixPartImageRulesPrevCostume(int costume, int num_costumes, int skipped, int extra_count)
{
    int max_costume = ftRemixPartImageRulesMaxCostume(num_costumes, skipped, extra_count);

    if (costume == ftRemixPartImageRulesFirstExtraCostume(num_costumes, skipped))
    {
        costume -= skipped;
    }
    return (costume > 0) ? (costume - 1) : max_costume;
}

/** TRUE when a Random pick may land on `costume` (Costumes.asm get_random_legal_costume_). */
int ftRemixPartImageRulesIsLegalRandomCostume(int costume, int num_costumes, int skipped)
{
    return (costume <= num_costumes) || (costume > (num_costumes + skipped));
}

/** TRUE when the images array of the part is the special part one (check_for_custom_costume_ _get_images_array). */
int ftRemixPartImageRulesUsesSpecialArray(int special_part_index)
{
    return special_part_index > 0;
}

/** Prim color word of the extra costume (check_for_custom_costume_ _prim_color). */
unsigned int ftRemixPartImageRulesPickPrimColor(unsigned int high_poly, unsigned int low_poly, int is_low_poly)
{
    if (is_low_poly && (low_poly != 0))
    {
        return low_poly;
    }
    return high_poly;
}

/** Image index of the MObj of an accessory part (check_for_custom_costume_ _get_part_id, special part with object id 0x3E9). */
int ftRemixPartImageRulesAccessoryImageIndex(int start_index, int is_first_mobj)
{
    return is_first_mobj ? start_index : (start_index + 1);
}

/** Palette index of a Yoshi egg (Shield.asm alter_yoshi_egg_color_). */
int ftRemixPartImageRulesEggPaletteIndex(int shield_option, int costume_color_index)
{
    if (shield_option != FTREMIX_PARTIMAGE_SHIELD_COSTUME)
    {
        return 0;
    }
    if ((costume_color_index < 0) || (costume_color_index >= FTREMIX_PARTIMAGE_EGG_PALETTE_COUNT))
    {
        return 0;
    }
    return costume_color_index;
}

#endif
